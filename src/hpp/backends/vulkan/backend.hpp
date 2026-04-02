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

#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <volk.h>
#include <vk_mem_alloc.h>

#include <backends/backend.hpp>

#define VK_CALL(call, description)                                                                                     \
  {                                                                                                                    \
    const auto r = call;                                                                                               \
    if (r != VK_SUCCESS)                                                                                               \
      return fail("'%s' failed with code %llu", description, (u64) r);                                                 \
  }

#define VK_ENUM_CALL(call, result, ...)                                                                                \
  {                                                                                                                    \
    UINT32 __count;                                                                                                    \
    call(__VA_ARGS__, &__count, nullptr);                                                                              \
    result.resize(__count);                                                                                            \
    call(__VA_ARGS__, &__count, result.data());                                                                        \
  }

namespace iavis
{
  class IAVis_Backend_Vulkan
  {
    struct Frame
    {
      VkFence in_use_fence;
      VkSemaphore image_available_semaphore;
      VkSemaphore render_finished_semaphore;
      VkCommandBuffer command_buffer;
      VkCommandPool command_pool;
      VkImage swapchain_image;
      VkImageView swapchain_image_view;
    };

    static constexpr u32 VULKAN_API_VERSION = VK_MAKE_VERSION(1, 3, 0);

public:
    IAVis_Backend_Vulkan() = default;
    ~IAVis_Backend_Vulkan() = default;

public:
    auto initialize(const InitInfo& init_info) -> Result<void>;
    auto shutdown() -> void;

    auto begin_frame() -> void;
    auto end_frame() -> void;

    auto create_geometry_unlit_2d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices)
        -> Result<GeomId>;
    auto create_geometry_unlit_3d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices)
        -> Result<GeomId>;
    auto destroy_geometry(GeomId id) -> void;

    auto create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps = true) -> Result<TexId>;
    auto destroy_texture(TexId id) -> void;

    auto create_material(TexId albedo_tex, TexId normal_tex = INVALID_ID, TexId height_tex = INVALID_ID,
                         TexId roughness_tex = INVALID_ID, TexId ao_tex = INVALID_ID) -> Result<MatId>;
    auto destroy_material(MatId id) -> void;

    auto cmd_set_camera_matrix(CmdBufferId cmd, const f32 *camera_matrix) -> void;
    auto cmd_set_projection_matrix(CmdBufferId cmd, const f32 *projection_matrix) -> void;
    auto cmd_set_scissor(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void;
    auto cmd_set_viewport(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void;
    auto cmd_set_material(CmdBufferId cmd, MatId id) -> void;
    auto cmd_draw_geometry(CmdBufferId cmd, GeomId id, const f32 *model_matrix) -> void;

public:
    auto get_surface_width() const -> i32
    {
      return m_swapchain_extent.width;
    }

    auto get_surface_height() const -> i32
    {
      return m_swapchain_extent.height;
    }

    auto get_surface_handle() const -> void *
    {
      return m_surface;
    }

    auto set_clear_color(f32 r, f32 g, f32 b) -> void
    {
      m_clear_color[0] = r;
      m_clear_color[1] = g;
      m_clear_color[2] = b;
    }

private:
    auto populate_extensions() -> void;

    auto initialize_instance(const char *app_name, bool is_debug) -> Result<void>;
    auto initialize_device() -> Result<void>;
    auto initialize_swapchain(i32 width, i32 height) -> Result<void>;
    auto select_physical_device() -> Result<VkPhysicalDevice>;

    auto recreate_swapchain() -> Result<void>;

    auto acquire_next_swapchain_image() -> VkImageView;
    auto submit_command_buffer(VkCommandBuffer cmd_buffer, VkFence fence) -> bool;
    auto submit_command_buffer_and_present(VkCommandBuffer cmd_buffer, VkFence fence) -> bool;

    auto queue_image_transition(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) -> void;
    auto queue_buffer_transition(VkBuffer buffer, VkBufferUsageFlags usage, VkAccessFlags src_access, VkAccessFlags dst_access) -> void;
    auto flush_transitions() -> void;

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkCommandPool m_command_pool;
    VkSurfaceKHR m_surface;

    VkSwapchainKHR m_swapchain;
    VkExtent2D m_swapchain_extent;
    VkFormat m_swapchain_format;
    VkColorSpaceKHR m_swapchain_colorspace;
    VkExtent2D m_swapchain_min_extent;
    VkExtent2D m_swapchain_max_extent;
    u32 m_swapchain_buffer_count;

    VkDebugUtilsMessengerEXT m_debug_messenger;

    Vec<const char *> m_device_extensions{};
    Vec<const char *> m_instance_extensions{};

    u32 m_graphics_queue_family_index;
    u32 m_compute_queue_family_index;
    u32 m_transfer_queue_family_index;

    VkQueue m_graphics_queue;
    VkQueue m_compute_queue;
    VkQueue m_transfer_queue;

    VmaAllocator m_allocator;

    Frame m_frames[NUM_FRAMES_BUFFERED];

    u32 m_current_frame_index;
    u32 m_current_sync_frame_index;

    f32 m_clear_color[3];

    Vec<VkImageMemoryBarrier2> m_image_memory_barriers;
    Vec<VkBufferMemoryBarrier2> m_buffer_memory_barriers;
  };

  static_assert(IAVIS_BACKEND<IAVis_Backend_Vulkan>, "All backends must implement IAVis_Backend");
} // namespace iavis