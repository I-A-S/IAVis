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

#include <auxid/auxid.hpp>

#include <glm/glm.hpp>

namespace iavis
{
  using namespace au;

  enum class EGeometryType : u32
  {
    UNLIT_2D,
    UNLIT_3D,
    LIT_2D,
    LIT_3D,
  };

  struct InitInfo
  {
    i32 surface_width{};
    i32 surface_height{};
    const char *app_name{nullptr};
    void* (*surface_creation_callback)(void* instance_handle, void* user_data){nullptr};
    void* surface_creation_callback_user_data{nullptr};
  };

  using Vec2 = glm::vec2;
  using Vec3 = glm::vec3;
  using Vec4 = glm::vec4;
  using Mat4 = glm::mat4;

  using TexId = u32;
  using MatId = u32;
  using GeomId = u32;
  using CmdBufferId = u32;

  static constexpr u32 INVALID_ID = 0;
}