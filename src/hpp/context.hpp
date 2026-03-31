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

#include <base.hpp>

namespace iavis::vulkan
{
  struct Frame
  {
    VkFence in_use_fence;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkCommandPool command_pool;
    VkImage swapchain_image;
    VkImageView swapchain_image_view;
  };

  struct GlobalContext
  {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkCommandPool command_pool;
    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkExtent2D swapchain_extent;
    VkFormat swapchain_format;
    VkColorSpaceKHR swapchain_colorspace;
    VkExtent2D swapchain_min_extent;
    VkExtent2D swapchain_max_extent;
    u32 swapchain_buffer_count;

    VkFence command_submit_fence;
    VkDebugUtilsMessengerEXT debug_messenger;

    Vec<const char *> device_extensions{};
    Vec<const char *> instance_extensions{};

    u32 graphics_queue_family_index;
    u32 compute_queue_family_index;
    u32 transfer_queue_family_index;

    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue transfer_queue;

    VmaAllocator allocator;

    Frame frames[NUM_FRAMES_BUFFERED];

    u32 current_frame_index;
    u32 current_sync_frame_index;

    f32 clear_color[3];
  };

  // [IATODO] Add multithreading (Q2 2026 - RoadMapID: MT-CMD)
  struct ThreadContext
  {
    VkCommandPool command_pool;
  };
} // namespace iavis