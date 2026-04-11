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

#include <iaghi/iaghi.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace iavis
{
  using namespace au;

  enum class EGeometryType : u32
  {
    INVALID = 0,

    UNLIT_2D,
    UNLIT_3D,
    LIT_2D,
    LIT_3D,
  };

  enum class EProjection : u32
  {
    ORTHOGRAPHIC,
    PERSPECTIVE,
  };

  struct VertexUnlit2DGeometry
  {
    glm::vec2 position{};
    glm::vec2 texcoords{};
  };

  struct VertexUnlit3DGeometry
  {
    glm::vec3 position{};
    float _padding;
    glm::vec2 texcoords{};
  };

  struct Camera
  {
    f32 fov{90.0f};
    f32 near_plane{0.1f};
    f32 far_plane{100.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 forward{0.0f, 0.0f, -1.0f};
    glm::vec3 position{};
    EProjection projection{EProjection::PERSPECTIVE};
  };

  using InitInfo = ghi::InitInfo;

  using IdType = u64;
  using TexId = IdType;
  using MatId = IdType;
  using GeomId = IdType;
  using DrawableId = IdType;
  static constexpr IdType INVALID_ID = UINT64_MAX;
} // namespace iavis