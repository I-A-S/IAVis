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

#include <iavis/iavis.hpp>
#include <auxid/containers/vec.hpp>
#include <auxid/containers/hash_map.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace iavis
{
  struct InternalCamera: Camera
  {
    auto operator=(const Camera &camera) const noexcept -> void;

    glm::mat4 view_matrix{};
    glm::mat4 projection_matrix{};
  };

  struct Geometry
  {
    ghi::Buffer vertex_buffer{};
    ghi::Buffer index_buffer{};
    u32 index_count{};
  };

  struct Material
  {

  };

  struct Drawable
  {
    bool active{true};
    Geometry geometry{};
    Material material{};
    glm::mat4 transform{};
  };

  struct Context
  {
    ghi::Device device{};
    u32 surface_width{};
    u32 surface_height{};

    InternalCamera camera{};

    Vec<Drawable> drawables{};
    Vec<Geometry> geometries{};
    Vec<Material> materials{};
    Vec<ghi::Image> textures{};

    ghi::Pipeline unlit_2d_pipeline{};
    ghi::Pipeline unlit_3d_pipeline{};
  };
}