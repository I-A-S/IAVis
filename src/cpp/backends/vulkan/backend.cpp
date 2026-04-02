// IAVis: IA Real-Time Rendering Library
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define VMA_IMPLEMENTATION

#include <backends/vulkan/backend.hpp>

namespace iavis
{
  auto IAVis_Backend_Vulkan::initialize(const InitInfo &init_info) -> Result<void>
  {
    populate_extensions();

    AU_TRY_DISCARD(initialize_instance(init_info.app_name, init_info.is_debug));

    m_surface = static_cast<VkSurfaceKHR>(
        init_info.surface_creation_callback(m_instance, init_info.surface_creation_callback_user_data));
    if (!m_surface)
      return fail("failed to create vulkan surface");

    AU_TRY_DISCARD(initialize_device());

    AU_TRY_DISCARD(initialize_swapchain(init_info.surface_width, init_info.surface_height));

    m_clear_color[0] = 0.0f;
    m_clear_color[1] = 0.0f;
    m_clear_color[2] = 0.0f;

    return {};
  }

  auto IAVis_Backend_Vulkan::shutdown() -> void
  {
    vkDeviceWaitIdle(m_device);

    for (INT32 i = 0; i < m_swapchain_buffer_count; i++)
    {
      vkDestroyImageView(m_device, m_frames[i].swapchain_image_view, nullptr);
      vkDestroySemaphore(m_device, m_frames[i].image_available_semaphore, nullptr);
      vkDestroySemaphore(m_device, m_frames[i].render_finished_semaphore, nullptr);
      vkDestroyFence(m_device, m_frames[i].in_use_fence, nullptr);
      vkDestroyCommandPool(m_device, m_frames[i].command_pool, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);

    vmaDestroyAllocator(m_allocator);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
  }

  auto IAVis_Backend_Vulkan::begin_frame() -> void
  {
    if (acquire_next_swapchain_image() == VK_NULL_HANDLE)
      return;

    auto &cmd = m_frames[m_current_sync_frame_index].command_buffer;

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &begin_info);

    queue_image_transition(m_frames[m_current_frame_index].swapchain_image, m_swapchain_format,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    flush_transitions();

    VkRenderingAttachmentInfo color_attachment_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_frames[m_current_frame_index].swapchain_image_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {m_clear_color[0], m_clear_color[1], m_clear_color[2], 1.0f}},
    };

    VkRenderingInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                .extent = {.width = m_swapchain_extent.width, .height = m_swapchain_extent.height},
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = nullptr,
    };

    vkCmdBeginRendering(m_frames[m_current_sync_frame_index].command_buffer, &rendering_info);
  }

  auto IAVis_Backend_Vulkan::end_frame() -> void
  {
    vkCmdEndRendering(m_frames[m_current_sync_frame_index].command_buffer);

    queue_image_transition(m_frames[m_current_frame_index].swapchain_image, m_swapchain_format,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    flush_transitions();

    submit_command_buffer_and_present(m_frames[m_current_sync_frame_index].command_buffer,
                                      m_frames[m_current_sync_frame_index].in_use_fence);
  }

  auto IAVis_Backend_Vulkan::create_geometry_unlit_2d(Span<const f32> positions, Span<const f32> tex_coords,
                                                      Span<const u32> indices) -> Result<GeomId>
  {
  }

  auto IAVis_Backend_Vulkan::create_geometry_unlit_3d(Span<const f32> positions, Span<const f32> tex_coords,
                                                      Span<const u32> indices) -> Result<GeomId>
  {
  }

  auto IAVis_Backend_Vulkan::destroy_geometry(GeomId id) -> void
  {
  }

  auto IAVis_Backend_Vulkan::create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps)
      -> Result<TexId>
  {
  }

  auto IAVis_Backend_Vulkan::destroy_texture(TexId id) -> void
  {
  }

  auto IAVis_Backend_Vulkan::create_material(TexId albedo_tex, TexId normal_tex, TexId height_tex, TexId roughness_tex,
                                             TexId ao_tex) -> Result<MatId>
  {
  }

  auto IAVis_Backend_Vulkan::destroy_material(MatId id) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_set_camera_matrix(CmdBufferId cmd, const f32 *camera_matrix) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_set_projection_matrix(CmdBufferId cmd, const f32 *projection_matrix) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_set_scissor(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_set_viewport(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_set_material(CmdBufferId cmd, MatId id) -> void
  {
  }

  auto IAVis_Backend_Vulkan::cmd_draw_geometry(CmdBufferId cmd, GeomId id, const f32 *model_matrix) -> void
  {
  }
} // namespace iavis

namespace iavis
{
  auto IAVis_Backend_Vulkan::populate_extensions() -> void
  {
    m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if AU_PLATFORM_WINDOWS
    m_instance_extensions.push_back("VK_KHR_win32_surface");
#elif AU_PLATFORM_ANDROID
    m_instance_extensions.push_back("VK_KHR_android_surface");
#elif AU_PLATFORM_LINUX
    m_instance_extensions.push_back("VK_KHR_xcb_surface");
    m_instance_extensions.push_back("VK_KHR_xlib_surface");
#endif

    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  auto IAVis_Backend_Vulkan::acquire_next_swapchain_image() -> VkImageView
  {
    m_current_sync_frame_index = (m_current_sync_frame_index + 1) % m_swapchain_buffer_count;

    const auto &frame = m_frames[m_current_sync_frame_index];

    vkWaitForFences(m_device, 1, &frame.in_use_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &frame.in_use_fence);

    u32 image_index{};
    const auto result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, frame.image_available_semaphore,
                                              VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      (void)recreate_swapchain();
      return VK_NULL_HANDLE;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      return VK_NULL_HANDLE;

    m_current_frame_index = image_index;

    return m_frames[m_current_frame_index].swapchain_image_view;
  }

  auto IAVis_Backend_Vulkan::submit_command_buffer(VkCommandBuffer cmd_buffer, VkFence fence) -> bool
  {
    vkEndCommandBuffer(cmd_buffer);

    VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT};
    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    if (vkResetFences(m_device, 1, &fence) != VK_SUCCESS)
      return false;
    const auto success = vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence) == VK_SUCCESS;
    return success;
  }

  auto IAVis_Backend_Vulkan::submit_command_buffer_and_present(VkCommandBuffer cmd_buffer, VkFence fence) -> bool
  {
    vkEndCommandBuffer(cmd_buffer);

    const auto &frame = m_frames[m_current_sync_frame_index];

    VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT};
    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.image_available_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frame.render_finished_semaphore,
    };

    if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence) != VK_SUCCESS)
      return false;

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pImageIndices = &m_current_frame_index;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame.render_finished_semaphore;
    vkQueuePresentKHR(m_graphics_queue, &present_info);

    return true;
  }

  auto IAVis_Backend_Vulkan::queue_image_transition(VkImage image, VkFormat format, VkImageLayout old_layout,
                                                    VkImageLayout new_layout) -> void
  {
    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
    m_image_memory_barriers.push_back(barrier);
  }

  auto IAVis_Backend_Vulkan::queue_buffer_transition(VkBuffer buffer, VkBufferUsageFlags usage,
                                                     VkAccessFlags src_access, VkAccessFlags dst_access) -> void
  {
  }

  auto IAVis_Backend_Vulkan::flush_transitions() -> void
  {
    VkDependencyInfo dep_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = static_cast<u32>(m_buffer_memory_barriers.size()),
        .pBufferMemoryBarriers = m_buffer_memory_barriers.data(),
        .imageMemoryBarrierCount = static_cast<u32>(m_image_memory_barriers.size()),
        .pImageMemoryBarriers = m_image_memory_barriers.data(),
    };

    vkCmdPipelineBarrier2(m_frames[m_current_sync_frame_index].command_buffer, &dep_info);

    m_image_memory_barriers.clear();
    m_buffer_memory_barriers.clear();
  }
} // namespace iavis