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

#include <vulkan_impl.hpp>

namespace iavis::vulkan
{
  GlobalContext g_context{};
} // namespace iavis::vulkan

namespace iavis
{
  auto initialize(const InitInfo &init_info) -> Result<void>
  {
    vulkan::populate_extensions();

    AU_TRY_DISCARD(vulkan::initialize_instance(init_info.app_name, init_info.is_debug));

    vulkan::g_context.surface = static_cast<VkSurfaceKHR>(
        init_info.surface_creation_callback(vulkan::g_context.instance, init_info.surface_creation_callback_user_data));
    if (!vulkan::g_context.surface)
      return fail("failed to create vulkan surface");

    AU_TRY_DISCARD(vulkan::initialize_device());

    AU_TRY_DISCARD(vulkan::initialize_swapchain(init_info.surface_width, init_info.surface_height));

    return {};
  }

  auto terminate() -> void
  {
    vkDeviceWaitIdle(vulkan::g_context.device);

    for (INT32 i = 0; i < vulkan::g_context.swapchain_buffer_count; i++)
    {
      vkDestroyImageView(vulkan::g_context.device, vulkan::g_context.frames[i].swapchain_image_view, nullptr);
      vkDestroySemaphore(vulkan::g_context.device, vulkan::g_context.frames[i].image_available_semaphore, nullptr);
      vkDestroySemaphore(vulkan::g_context.device, vulkan::g_context.frames[i].render_finished_semaphore, nullptr);
      vkDestroyFence(vulkan::g_context.device, vulkan::g_context.frames[i].in_use_fence, nullptr);
      vkDestroyCommandPool(vulkan::g_context.device, vulkan::g_context.frames[i].command_pool, nullptr);
    }

    vkDestroySwapchainKHR(vulkan::g_context.device, vulkan::g_context.swapchain, nullptr);
    vkDestroyFence(vulkan::g_context.device, vulkan::g_context.command_submit_fence, nullptr);

    vkDestroyDebugUtilsMessengerEXT(vulkan::g_context.instance, vulkan::g_context.debug_messenger, nullptr);

    vmaDestroyAllocator(vulkan::g_context.allocator);

    vkDestroySurfaceKHR(vulkan::g_context.instance, vulkan::g_context.surface, nullptr);
    vkDestroyDevice(vulkan::g_context.device, nullptr);
    vkDestroyInstance(vulkan::g_context.instance, nullptr);
  }

  auto get_surface_width() -> i32
  {
    return vulkan::g_context.swapchain_extent.width;
  }

  auto get_surface_height() -> i32
  {
    return vulkan::g_context.swapchain_extent.height;
  }

  auto get_surface_handle() -> void *
  {
    return vulkan::g_context.surface;
  }

  auto begin_frame() -> TexId
  {
    const auto img_view = vulkan::acquire_next_swapchain_image();
    if (img_view == VK_NULL_HANDLE)
      return INVALID_ID;



    return reinterpret_cast<TexId>(img_view);
  }

  auto end_frame() -> bool
  {
  }

  auto begin_command_buffer() -> CmdBufferId
  {
  }

  auto end_command_buffer(CmdBufferId id) -> void
  {
  }

  auto submit_command_buffer(CmdBufferId id) -> void
  {
  }

  auto submit_command_buffer_sync(CmdBufferId id) -> void
  {
  }

  auto set_clear_color(f32 r, f32 g, f32 b) -> void
  {
    vulkan::g_context.clear_color[0] = r;
    vulkan::g_context.clear_color[1] = g;
    vulkan::g_context.clear_color[2] = b;
  }
} // namespace iavis

namespace iavis::vulkan
{
  auto populate_extensions() -> void
  {
    vulkan::g_context.instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if AU_PLATFORM_WINDOWS
    vulkan::g_context.instance_extensions.push_back("VK_KHR_win32_surface");
#elif AU_PLATFORM_ANDROID
    vulkan::g_context.instance_extensions.push_back("VK_KHR_android_surface");
#elif AU_PLATFORM_LINUX
    vulkan::g_context.instance_extensions.push_back("VK_KHR_xcb_surface");
    vulkan::g_context.instance_extensions.push_back("VK_KHR_xlib_surface");
#endif

    vulkan::g_context.device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  auto acquire_next_swapchain_image() -> VkImageView
  {
    g_context.current_sync_frame_index = (g_context.current_sync_frame_index + 1) % g_context.swapchain_buffer_count;

    const auto &frame = g_context.frames[g_context.current_sync_frame_index];

    vkWaitForFences(g_context.device, 1, &frame.in_use_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(g_context.device, 1, &frame.in_use_fence);

    u32 image_index{};
    const auto result = vkAcquireNextImageKHR(g_context.device, g_context.swapchain, UINT64_MAX,
                                              frame.image_available_semaphore, VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      if (!recreate_swapchain())
        return VK_NULL_HANDLE;
      return acquire_next_swapchain_image();
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      return VK_NULL_HANDLE;

    g_context.current_frame_index = image_index;

    return g_context.frames[g_context.current_frame_index].swapchain_image_view;
  }

  auto submit_command_buffer(VkCommandBuffer cmd_buffer) -> bool
  {
    vkEndCommandBuffer(cmd_buffer);

    VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    if (vkResetFences(g_context.device, 1, &g_context.command_submit_fence) != VK_SUCCESS)
      return false;
    const auto success =
        vkQueueSubmit(g_context.graphics_queue, 1, &submitInfo, g_context.command_submit_fence) == VK_SUCCESS;
    return success;
  }

  auto submit_command_buffer_and_present(VkCommandBuffer cmd_buffer) -> bool
  {
    vkEndCommandBuffer(cmd_buffer);

    VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    if (vkResetFences(g_context.device, 1, &g_context.command_submit_fence) != VK_SUCCESS)
      return false;

    if (vkQueueSubmit(g_context.graphics_queue, 1, &submitInfo, g_context.command_submit_fence) != VK_SUCCESS)
      return false;

    const auto &frame = g_context.frames[g_context.current_sync_frame_index];

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pImageIndices = &g_context.current_frame_index;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &g_context.swapchain;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &frame.render_finished_semaphore;
    vkQueuePresentKHR(g_context.graphics_queue, &presentInfo);

    g_context.current_frame_index = (g_context.current_frame_index + 1) % g_context.swapchain_buffer_count;

    return true;
  }
} // namespace iavis::vulkan
