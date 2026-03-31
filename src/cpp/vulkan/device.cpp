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
  static auto select_physical_device() -> Result<VkPhysicalDevice>
  {
    bool found = false;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    auto &logger = auxid::get_thread_logger();

    VkPhysicalDeviceProperties props{};
    VkPhysicalDeviceFeatures features{};
    Vec<VkPhysicalDevice> physicalDevices;
    VK_ENUM_CALL(vkEnumeratePhysicalDevices, physicalDevices, g_context.instance);

    for (const auto &pd : physicalDevices)
    {
      vkGetPhysicalDeviceProperties(pd, &props);
      vkGetPhysicalDeviceFeatures(pd, &features);

      if (props.deviceType != VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        continue;

      u32 queueCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, nullptr);
      Vec<VkQueueFamilyProperties> queues(queueCount);
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, queues.data());

      for (u32 i = 0; i < queueCount; i++)
      {
        VkBool32 supportsPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, g_context.surface, &supportsPresent);

        if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent)
        {
          physical_device = pd;
          g_context.graphics_queue_family_index = i;
          found = true;
        }
      }
    }

    if (!found)
      return fail("failed to find suitable graphics hardware.");

    logger.info("using the hardware device '%s'", props.deviceName);

    return physical_device;
  }

  auto initialize_device() -> Result<void>
  {
    g_context.physical_device = AU_TRY(select_physical_device());

    Vec<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

    Vec<VkQueueFamilyProperties> queueFamilyProps;
    VK_ENUM_CALL(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilyProps, g_context.physical_device);

    for (u32 i = 0; i < queueFamilyProps.size(); i++)
    {
      const auto &props = queueFamilyProps[i];
      if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
      {
        if (g_context.compute_queue_family_index == UINT32_MAX)
          g_context.compute_queue_family_index = i;
        else if (g_context.graphics_queue_family_index != UINT32_MAX && i != g_context.graphics_queue_family_index)
        {
          g_context.compute_queue_family_index = i;
          break;
        }
      }
    }

    for (u32 i = 0; i < queueFamilyProps.size(); i++)
    {
      const auto &props = queueFamilyProps[i];
      if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
      {
        if (g_context.transfer_queue_family_index == UINT32_MAX)
          g_context.transfer_queue_family_index = i;
        else if (i != g_context.graphics_queue_family_index && i != g_context.compute_queue_family_index)
        {
          g_context.transfer_queue_family_index = i;
          break;
        }
      }
    }

    HashMap<u32, u32> queueFamilyIndexMap;

    if (g_context.graphics_queue_family_index != UINT32_MAX)
      queueFamilyIndexMap[g_context.graphics_queue_family_index]++;
    if (g_context.compute_queue_family_index != UINT32_MAX)
      queueFamilyIndexMap[g_context.compute_queue_family_index]++;
    if (g_context.transfer_queue_family_index != UINT32_MAX)
      queueFamilyIndexMap[g_context.transfer_queue_family_index]++;

    Vec<Vec<f32>> priorityStorage;
    priorityStorage.reserve(queueFamilyIndexMap.size());
    for (auto &[family_index, count] : queueFamilyIndexMap)
    {
      VkDeviceQueueCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.queueFamilyIndex = family_index;

      info.queueCount = std::min(count, queueFamilyProps[family_index].queueCount);

      Vec<f32> priorities;
      priorities.resize(info.queueCount);
      for (u32 i = 0; i < info.queueCount; i++) priorities[i] = 1.0f;
      priorityStorage.push_back(priorities);

      info.pQueuePriorities = priorityStorage.back().data();
      deviceQueueCreateInfos.push_back(info);
    }

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT dynamicVertexInputFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
        .vertexInputDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = &dynamicVertexInputFeatures,
        .extendedDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features enable_vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enable_extended_dynamic_state_features,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 enable_device_features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enable_vulkan13_features,
    };

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledExtensionCount = g_context.device_extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = g_context.device_extensions.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.queueCreateInfoCount = deviceQueueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    deviceCreateInfo.pNext = &enable_device_features2;

    VK_CALL(vkCreateDevice(g_context.physical_device, &deviceCreateInfo, nullptr, &g_context.device), "Creating logical device");

    volkLoadDevice(g_context.device);

    HashMap<u32, u32> tmpQueueFamilyIndexMap;
    if (g_context.graphics_queue_family_index != UINT32_MAX)
    {
      u32 qIndex = tmpQueueFamilyIndexMap[g_context.graphics_queue_family_index]++;
      if (qIndex < queueFamilyProps[g_context.graphics_queue_family_index].queueCount)
        vkGetDeviceQueue(g_context.device, g_context.graphics_queue_family_index, qIndex, &g_context.graphics_queue);
    }
    if (g_context.compute_queue_family_index != UINT32_MAX)
    {
      u32 qIndex = tmpQueueFamilyIndexMap[g_context.compute_queue_family_index]++;
      if (qIndex >= queueFamilyProps[g_context.compute_queue_family_index].queueCount)
        qIndex = 0;

      vkGetDeviceQueue(g_context.device, g_context.compute_queue_family_index, qIndex, &g_context.compute_queue);
    }
    if (g_context.transfer_queue_family_index != UINT32_MAX)
    {
      u32 qIndex = tmpQueueFamilyIndexMap[g_context.transfer_queue_family_index]++;
      if (qIndex >= queueFamilyProps[g_context.transfer_queue_family_index].queueCount)
        qIndex = 0;

      vkGetDeviceQueue(g_context.device, g_context.transfer_queue_family_index, qIndex, &g_context.transfer_queue);
    }

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CALL(vkCreateFence(g_context.device, &fenceCreateInfo, nullptr, &g_context.command_submit_fence), "Creating command submit fence");

    VmaAllocatorCreateInfo allocatorCreateInfo{
        .physicalDevice = g_context.physical_device,
        .device = g_context.device,
        .instance = g_context.instance,
        .vulkanApiVersion = VULKAN_API_VERSION,
    };
    VmaVulkanFunctions vmaVulkanFunctions{};
    VK_CALL(vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vmaVulkanFunctions), "Importing VMA functions");
    allocatorCreateInfo.pVulkanFunctions = &vmaVulkanFunctions;
    VK_CALL(vmaCreateAllocator(&allocatorCreateInfo, &g_context.allocator), "Creating VMA allocator");

    return {};
  }
} // namespace iavis