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

#include <context.hpp>

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

namespace iavis::vulkan
{
  static constexpr u32 VULKAN_API_VERSION = VK_MAKE_VERSION(1, 3, 0);

  extern GlobalContext g_context;

  auto populate_extensions() -> void;

  auto initialize_instance(const char *app_name, bool is_debug) -> Result<void>;
  auto initialize_device() -> Result<void>;
  auto initialize_swapchain(i32 width, i32 height) -> Result<void>;

  auto recreate_swapchain() -> Result<void>;

  auto acquire_next_swapchain_image() -> VkImageView;
  auto submit_command_buffer(VkCommandBuffer cmd_buffer) -> bool;
  auto submit_command_buffer_and_present(VkCommandBuffer cmd_buffer) -> bool;
}