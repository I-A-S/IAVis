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

#include <backends/vulkan/backend.hpp>

namespace iavis
{
  auto IAVis_Backend_Vulkan::initialize_swapchain(i32 width, i32 height) -> Result<void>
  {
    VkSurfaceFormatKHR selected_surface_format{};
    Vec<VkSurfaceFormatKHR> surface_formats;
    VK_ENUM_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, surface_formats, m_physical_device, m_surface);
    for (const auto &format : surface_formats)
    {
      selected_surface_format = format;
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
        break;
    }
    m_swapchain_format = selected_surface_format.format;
    m_swapchain_colorspace = selected_surface_format.colorSpace;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &surface_capabilities),
            "Fetching surface capabilities");
    m_swapchain_buffer_count = std::max(NUM_FRAMES_BUFFERED, surface_capabilities.minImageCount);
    if (surface_capabilities.maxImageCount > 0)
      m_swapchain_buffer_count = std::min(m_swapchain_buffer_count, surface_capabilities.maxImageCount);
    m_swapchain_min_extent = surface_capabilities.minImageExtent;
    m_swapchain_max_extent = surface_capabilities.maxImageExtent;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_graphics_queue_family_index,
    };

    VkCommandBufferAllocateInfo command_alloc_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

    for (INT32 i = 0; i < m_swapchain_buffer_count; i++)
    {
      VK_CALL(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_frames[i].render_finished_semaphore),
              "Creating frame render finished semaphore");
      VK_CALL(vkCreateFence(m_device, &fence_create_info, nullptr, &m_frames[i].in_use_fence),
              "Creating frame inflight fence");
      VK_CALL(vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &m_frames[i].command_pool),
              "Creating frame command pool");

      command_alloc_info.commandPool = m_frames[i].command_pool;
      VK_CALL(vkAllocateCommandBuffers(m_device, &command_alloc_info, &m_frames[i].command_buffer),
              "Allocating frame command buffer");
    }

    m_swapchain = VK_NULL_HANDLE;

    AU_TRY_DISCARD(recreate_swapchain());

    return {};
  }

  auto IAVis_Backend_Vulkan::recreate_swapchain() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    VK_CALL(vkDeviceWaitIdle(m_device), "Waiting device idle");

    if (m_swapchain != VK_NULL_HANDLE)
    {
      for (INT32 i = 0; i < m_swapchain_buffer_count; i++)
        vkDestroyImageView(m_device, m_frames[i].swapchain_image_view, nullptr);
    }

    m_swapchain_extent.width =
        std::min(std::max(m_swapchain_extent.width, m_swapchain_min_extent.width), m_swapchain_max_extent.width);
    m_swapchain_extent.height =
        std::min(std::max(m_swapchain_extent.height, m_swapchain_min_extent.height), m_swapchain_max_extent.height);

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.oldSwapchain = m_swapchain;
    create_info.surface = m_surface;
    create_info.imageFormat = m_swapchain_format;
    create_info.imageColorSpace = m_swapchain_colorspace;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &m_graphics_queue_family_index;
    create_info.minImageCount = m_swapchain_buffer_count;
    create_info.imageExtent = m_swapchain_extent;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CALL(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain), "Creating swapchain");
    vkDestroySwapchainKHR(m_device, create_info.oldSwapchain, nullptr);

    Vec<VkImage> swapchain_images;
    VK_ENUM_CALL(vkGetSwapchainImagesKHR, swapchain_images, m_device, m_swapchain);
    INT32 frame_index{0};
    for (const auto &img : swapchain_images)
    {
      VkImageView view{};

      VkImageViewCreateInfo view_create_info{};
      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.image = img;
      view_create_info.format = m_swapchain_format;
      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      view_create_info.subresourceRange.baseArrayLayer = 0;
      view_create_info.subresourceRange.baseMipLevel = 0;
      view_create_info.subresourceRange.layerCount = 1;
      view_create_info.subresourceRange.levelCount = 1;
      VK_CALL(vkCreateImageView(m_device, &view_create_info, nullptr, &view), "Creating swapchain image view");

      m_frames[frame_index].swapchain_image = img;
      m_frames[frame_index++].swapchain_image_view = view;
    }

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (INT32 i = 0; i < m_swapchain_buffer_count; i++)
    {
      vkDestroySemaphore(m_device, m_frames[i].image_available_semaphore, nullptr);
      VK_CALL(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_frames[i].image_available_semaphore),
              "Creating swapchain semaphore");
    }

    logger.info("recreated swapchain (%ux%ux%u)", m_swapchain_extent.width, m_swapchain_extent.height,
                create_info.minImageCount);

    return {};
  }
} // namespace iavis