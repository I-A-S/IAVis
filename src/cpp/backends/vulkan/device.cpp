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
  auto IAVis_Backend_Vulkan::select_physical_device() -> Result<VkPhysicalDevice>
  {
    bool found = false;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    auto &logger = auxid::get_thread_logger();

    VkPhysicalDeviceProperties props{};
    VkPhysicalDeviceFeatures features{};
    Vec<VkPhysicalDevice> physical_devices;
    VK_ENUM_CALL(vkEnumeratePhysicalDevices, physical_devices, m_instance);

    for (const auto &pd : physical_devices)
    {
      vkGetPhysicalDeviceProperties(pd, &props);
      vkGetPhysicalDeviceFeatures(pd, &features);

      if (props.deviceType != VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        continue;

      u32 queue_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, nullptr);
      Vec<VkQueueFamilyProperties> queues(queue_count);
      vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, queues.data());

      for (u32 i = 0; i < queue_count; i++)
      {
        VkBool32 supports_present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, m_surface, &supports_present);

        if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
        {
          physical_device = pd;
          m_graphics_queue_family_index = i;
          found = true;
        }
      }
    }

    if (!found)
      return fail("failed to find suitable graphics hardware.");

    logger.info("using the hardware device '%s'", props.deviceName);

    return physical_device;
  }

  auto IAVis_Backend_Vulkan::initialize_device() -> Result<void>
  {
    m_physical_device = AU_TRY(select_physical_device());

    Vec<VkDeviceQueueCreateInfo> device_queue_create_infos;

    Vec<VkQueueFamilyProperties> queue_family_props;
    VK_ENUM_CALL(vkGetPhysicalDeviceQueueFamilyProperties, queue_family_props, m_physical_device);

    for (u32 i = 0; i < queue_family_props.size(); i++)
    {
      const auto &props = queue_family_props[i];
      if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
      {
        if (m_compute_queue_family_index == UINT32_MAX)
          m_compute_queue_family_index = i;
        else if (m_graphics_queue_family_index != UINT32_MAX && i != m_graphics_queue_family_index)
        {
          m_compute_queue_family_index = i;
          break;
        }
      }
    }

    for (u32 i = 0; i < queue_family_props.size(); i++)
    {
      if (const auto &props = queue_family_props[i]; props.queueFlags & VK_QUEUE_TRANSFER_BIT)
      {
        if (m_transfer_queue_family_index == UINT32_MAX)
          m_transfer_queue_family_index = i;
        else if (i != m_graphics_queue_family_index && i != m_compute_queue_family_index)
        {
          m_transfer_queue_family_index = i;
          break;
        }
      }
    }

    HashMap<u32, u32> queue_family_index_map;

    if (m_graphics_queue_family_index != UINT32_MAX)
      queue_family_index_map[m_graphics_queue_family_index]++;
    if (m_compute_queue_family_index != UINT32_MAX)
      queue_family_index_map[m_compute_queue_family_index]++;
    if (m_transfer_queue_family_index != UINT32_MAX)
      queue_family_index_map[m_transfer_queue_family_index]++;

    Vec<Vec<f32>> priority_storage;
    priority_storage.reserve(queue_family_index_map.size());
    for (auto &[family_index, count] : queue_family_index_map)
    {
      VkDeviceQueueCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.queueFamilyIndex = family_index;

      info.queueCount = std::min(count, queue_family_props[family_index].queueCount);

      Vec<f32> priorities;
      priorities.resize(info.queueCount);
      for (u32 i = 0; i < info.queueCount; i++) priorities[i] = 1.0f;
      priority_storage.push_back(priorities);

      info.pQueuePriorities = priority_storage.back().data();
      device_queue_create_infos.push_back(info);
    }

    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT dynamic_vertex_input_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
        .vertexInputDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = &dynamic_vertex_input_features,
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

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.enabledExtensionCount = m_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = m_device_extensions.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.queueCreateInfoCount = device_queue_create_infos.size();
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.pNext = &enable_device_features2;

    VK_CALL(vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device), "Creating logical device");

    volkLoadDevice(m_device);

    HashMap<u32, u32> tmp_queue_family_index_map;
    if (m_graphics_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_graphics_queue_family_index]++;
      if (q_index < queue_family_props[m_graphics_queue_family_index].queueCount)
        vkGetDeviceQueue(m_device, m_graphics_queue_family_index, q_index, &m_graphics_queue);
    }
    if (m_compute_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_compute_queue_family_index]++;
      if (q_index >= queue_family_props[m_compute_queue_family_index].queueCount)
        q_index = 0;

      vkGetDeviceQueue(m_device, m_compute_queue_family_index, q_index, &m_compute_queue);
    }
    if (m_transfer_queue_family_index != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_transfer_queue_family_index]++;
      if (q_index >= queue_family_props[m_transfer_queue_family_index].queueCount)
        q_index = 0;

      vkGetDeviceQueue(m_device, m_transfer_queue_family_index, q_index, &m_transfer_queue);
    }

    VmaAllocatorCreateInfo allocator_create_info{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .instance = m_instance,
        .vulkanApiVersion = VULKAN_API_VERSION,
    };
    VmaVulkanFunctions vma_vulkan_functions{};
    VK_CALL(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vulkan_functions), "Importing VMA functions");
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
    VK_CALL(vmaCreateAllocator(&allocator_create_info, &m_allocator), "Creating VMA allocator");

    return {};
  }
} // namespace iavis