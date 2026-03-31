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

#include <vulkan_impl.hpp>

namespace iavis::vulkan
{
  auto initialize_swapchain(i32 width, i32 height) -> Result<void>
  {
    VkSurfaceFormatKHR selectedSurfaceFormat;
    Vec<VkSurfaceFormatKHR> surfaceFormats;
    VK_ENUM_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, surfaceFormats, g_context.physical_device, g_context.surface);
    for (const auto &format : surfaceFormats)
    {
      selectedSurfaceFormat = format;
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
        break;
    }
    g_context.swapchain_format = selectedSurfaceFormat.format;
    g_context.swapchain_colorspace = selectedSurfaceFormat.colorSpace;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CALL(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_context.physical_device, g_context.surface, &surfaceCapabilities),
        "Fetching surface capabilities");
    g_context.swapchain_buffer_count = std::max(NUM_FRAMES_BUFFERED, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0)
      g_context.swapchain_buffer_count = std::min(g_context.swapchain_buffer_count, surfaceCapabilities.maxImageCount);
    g_context.swapchain_min_extent = surfaceCapabilities.minImageExtent;
    g_context.swapchain_max_extent = surfaceCapabilities.maxImageExtent;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = g_context.graphics_queue_family_index;

    for (INT32 i = 0; i < g_context.swapchain_buffer_count; i++)
    {
      VK_CALL(vkCreateSemaphore(g_context.device, &semaphoreCreateInfo, nullptr,
                                &g_context.frames[i].render_finished_semaphore),
              "Creating swapchain render finished semaphore");
      VK_CALL(vkCreateFence(g_context.device, &fenceCreateInfo, nullptr, &g_context.frames[i].in_use_fence),
              "Creating swapchain inflight fence");
      VK_CALL(vkCreateCommandPool(g_context.device, &commandPoolCreateInfo, nullptr, &g_context.frames[i].command_pool),
              "Creating swapchain command pool");
    }

    g_context.swapchain = VK_NULL_HANDLE;

    AU_TRY_DISCARD(recreate_swapchain());

    return {};
  }

  auto recreate_swapchain() -> Result<void>
  {
    auto& logger = auxid::get_thread_logger();

    VK_CALL(vkDeviceWaitIdle(g_context.device), "Waiting device idle");

    if (g_context.swapchain != VK_NULL_HANDLE)
    {
      for (INT32 i = 0; i < g_context.swapchain_buffer_count; i++)
        vkDestroyImageView(g_context.device, g_context.frames[i].swapchain_image_view, nullptr);
    }

    g_context.swapchain_extent.width = std::min(std::max(g_context.swapchain_extent.width, g_context.swapchain_min_extent.width),
                                       g_context.swapchain_max_extent.width);
    g_context.swapchain_extent.height = std::min(std::max(g_context.swapchain_extent.height, g_context.swapchain_min_extent.height),
                                        g_context.swapchain_max_extent.height);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.oldSwapchain = g_context.swapchain;
    createInfo.surface = g_context.surface;
    createInfo.imageFormat = g_context.swapchain_format;
    createInfo.imageColorSpace = g_context.swapchain_colorspace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &g_context.graphics_queue_family_index;
    createInfo.minImageCount = g_context.swapchain_buffer_count;
    createInfo.imageExtent = g_context.swapchain_extent;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CALL(vkCreateSwapchainKHR(g_context.device, &createInfo, nullptr, &g_context.swapchain), "Creating swapchain");
    vkDestroySwapchainKHR(g_context.device, createInfo.oldSwapchain, nullptr);

    Vec<VkImage> swapchainImages;
    VK_ENUM_CALL(vkGetSwapchainImagesKHR, swapchainImages, g_context.device, g_context.swapchain);
    INT32 frameIndex{0};
    for (const auto &img : swapchainImages)
    {
      VkImageView view{};

      VkImageViewCreateInfo viewCreateInfo{};
      viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewCreateInfo.image = img;
      viewCreateInfo.format = g_context.swapchain_format;
      viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewCreateInfo.subresourceRange.baseArrayLayer = 0;
      viewCreateInfo.subresourceRange.baseMipLevel = 0;
      viewCreateInfo.subresourceRange.layerCount = 1;
      viewCreateInfo.subresourceRange.levelCount = 1;
      VK_CALL(vkCreateImageView(g_context.device, &viewCreateInfo, nullptr, &view), "Creating swapchain image view");

      g_context.frames[frameIndex].swapchain_image = img;
      g_context.frames[frameIndex++].swapchain_image_view = view;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (INT32 i = 0; i < g_context.swapchain_buffer_count; i++)
    {
      vkDestroySemaphore(g_context.device, g_context.frames[i].image_available_semaphore, nullptr);
      VK_CALL(vkCreateSemaphore(g_context.device, &semaphoreCreateInfo, nullptr, &g_context.frames[i].image_available_semaphore),
              "Creating swapchain semaphore");
    }

    logger.info("Recreated swapchain (%ux%ux%u)", g_context.swapchain_extent.width, g_context.swapchain_extent.height,
                 createInfo.minImageCount);

    return {};
  }
} // namespace iavis