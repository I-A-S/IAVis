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
  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData)
  {
    AU_UNUSED(pUserData);
    AU_UNUSED(messageType);

    auto &logger = auxid::get_thread_logger();

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
      logger.error("[Validation]: %s", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      logger.warn("[Validation]: %s", pCallbackData->pMessage);

    return VK_FALSE;
  }

  auto IAVis_Backend_Vulkan::initialize_instance(const char *app_name, bool is_debug) -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    VK_CALL(volkInitialize(), "Initializing Vulkan loader");

    u32 instance_version{};
    VK_CALL(vkEnumerateInstanceVersion(&instance_version), "Enumerating Vulkan version");

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.applicationVersion = 1;
    application_info.engineVersion = 1;
    application_info.apiVersion = VULKAN_API_VERSION;
    application_info.pApplicationName = application_info.pEngineName = app_name ? app_name : "IAVis";

    static auto validation_layer_name = "VK_LAYER_KHRONOS_validation";

    if (is_debug)
    {
      bool validation_found = false;

      u32 layer_count{};
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      Vec<VkLayerProperties> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      for (const auto &layer : available_layers)
      {
        if (strcmp(validation_layer_name, layer.layerName) == 0)
        {
          validation_found = true;
          break;
        }
      }

      if (!validation_found)
      {
        logger.warn("validation layer '%s' not found. Debugging will be disabled.", validation_layer_name);
        is_debug = false;
      }
    }

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

    if (is_debug)
    {
      instance_create_info.enabledLayerCount = 1;
      instance_create_info.ppEnabledLayerNames = &validation_layer_name;

      debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info.pfnUserCallback = debug_callback;

      instance_create_info.pNext = &debug_create_info;

      m_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instance_create_info.enabledExtensionCount = m_instance_extensions.size();
    instance_create_info.ppEnabledExtensionNames = m_instance_extensions.data();
    instance_create_info.pApplicationInfo = &application_info;

    if (instance_version < application_info.apiVersion)
      return fail("IAVis requires graphics hardware that supports at least Vulkan API version '%u'",
                  VULKAN_API_VERSION);

    VK_CALL(vkCreateInstance(&instance_create_info, nullptr, &m_instance), "Creating Vulkan instance");
    volkLoadInstance(m_instance);

    if (is_debug)
    {
      if (vkCreateDebugUtilsMessengerEXT(m_instance, &debug_create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
        logger.warn("failed to set up debug messenger");
    }

    return {};
  }
} // namespace iavis
