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

  auto initialize_instance(const char *app_name, bool is_debug) -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    VK_CALL(volkInitialize(), "Initializing Vulkan loader");

    u32 instanceVersion{};
    VK_CALL(vkEnumerateInstanceVersion(&instanceVersion), "Enumerating Vulkan version");

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = 1;
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VULKAN_API_VERSION;
    applicationInfo.pApplicationName = applicationInfo.pEngineName = app_name ? app_name : "IAVis";

    static auto validationLayer = "VK_LAYER_KHRONOS_validation";

    if (is_debug)
    {
      bool validation_found = false;

      u32 layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
      Vec<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

      for (const auto &layer : availableLayers)
      {
        if (strcmp(validationLayer, layer.layerName) == 0)
        {
          validation_found = true;
          break;
        }
      }

      if (!validation_found)
      {
        logger.warn("validation layer '%s' not found. Debugging will be disabled.", validationLayer);
        is_debug = false;
      }
    }

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (is_debug)
    {
      instanceCreateInfo.enabledLayerCount = 1;
      instanceCreateInfo.ppEnabledLayerNames = &validationLayer;

      debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugCreateInfo.pfnUserCallback = debug_callback;

      instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;

      g_context.instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = g_context.instance_extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = g_context.instance_extensions.data();
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    if (instanceVersion < applicationInfo.apiVersion)
      return fail("IAVis requires graphics hardware that supports at least Vulkan API version '%u'",
                  VULKAN_API_VERSION);

    VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &g_context.instance), "Creating Vulkan instance");
    volkLoadInstance(g_context.instance);

    if (is_debug)
    {
      if (vkCreateDebugUtilsMessengerEXT(g_context.instance, &debugCreateInfo, nullptr, &g_context.debug_messenger) != VK_SUCCESS)
        logger.warn("failed to set up debug messenger");
    }

    return {};
  }
} // namespace iavis
