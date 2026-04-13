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
#include <glm/gtc/matrix_transform.hpp>

namespace iavis
{
  struct InternalCamera : Camera
  {
    auto operator=(const Camera &camera) noexcept -> void;

    glm::mat4 view_matrix{};
    glm::mat4 projection_matrix{};
  };

  struct Geometry
  {
    bool is_3d{false};
    bool is_lit{false};
    ghi::Buffer vertex_buffer{};
    ghi::Buffer index_buffer{};
    u32 index_count{};
  };

  struct Material {
    u32 albedo_index;
    u32 normal_index;
    u32 height_index;
    u32 roughness_index;
    u32 ao_index;

    u32 padding[3];
  };

  struct Drawable
  {
    bool active{true};
    Geometry geometry{};

    glm::vec2 tex_coords{};

    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};
    glm::mat4 transform{};
    bool is_transform_dirty{};

    auto update_transform() -> void
    {
      if (!is_transform_dirty)
        return;
      transform =
          glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
      is_transform_dirty = false;
    }
  };

  struct UBO_Unlit_Global
  {
    glm::mat4 projection_matrix{};
  };

  struct UBO_Unlit_Per_Frame
  {
    glm::mat4 view_matrix{};
  };

  struct PC_Unlit_Per_Draw
  {
    glm::mat4 model_matrix{};
    glm::vec2 tex_coords{};
    u32 material_index{};
  };

  struct Context
  {
    ghi::Device device{};
    u32 surface_width{};
    u32 surface_height{};

    InternalCamera camera{};

    Vec<Geometry> geometries{};
    Vec<Material> materials{};
    Vec<ghi::Image> textures{};

    HashMap<MatId, Vec<Drawable *>> unlit_2d_drawables{};
    HashMap<MatId, Vec<Drawable *>> unlit_3d_drawables{};

    ghi::Pipeline unlit_2d_pipeline{};
    ghi::Pipeline unlit_3d_pipeline{};

    ghi::Sampler sampler_clamp{};
    ghi::Sampler sampler_repeat{};

    ghi::Buffer material_storage_buffer{};

    ghi::Buffer ubo_unlit_global;
    ghi::Buffer ubo_unlit_per_frame;

    UBO_Unlit_Global ub_unlit_global;
    UBO_Unlit_Per_Frame ub_unlit_per_frame;

    ghi::BindingLayout unlit_pipeline_global_data_binding_layout{};
    ghi::BindingLayout unlit_pipeline_per_frame_data_binding_layout{};
    ghi::DescriptorTable unlit_pipeline_global_data_descriptor_table{};
    ghi::DescriptorTable unlit_pipeline_per_frame_binding_descriptor_table{};

    ghi::BindingLayout texture_data_binding_layout{};
    ghi::DescriptorTable texture_data_descriptor_table{};
  };
} // namespace iavis