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
  template<typename T> class UniformBuffer
  {
public:
    static auto create(const ghi::Device &device) -> Result<UniformBuffer>
    {
      UniformBuffer result{};
      result.m_data = {};
      result.m_device = device;
      ghi::BufferDesc buf_desc{
          .size_bytes = sizeof(T),
          .usage = ghi::EBufferUsage::Uniform,
          .cpu_visible = true,
      };
      AU_TRY_DISCARD(ghi::create_buffers(device, 1, &buf_desc, &result.m_buffer));
      return result;
    }

    auto destroy() -> void
    {
      ghi::destroy_buffers(m_device, 1, &m_buffer);
    }

    auto flush() -> void
    {
      const auto ptr = ghi::map_buffer(m_device, m_buffer);
      memcpy(ptr, &m_data, sizeof(m_data));
      ghi::unmap_buffer(m_device, m_buffer);
    }

    auto data() -> T &
    {
      return m_data;
    }

    [[nodiscard]] auto get_handle() const -> ghi::Buffer
    {
      return m_buffer;
    }

private:
    T m_data{};
    ghi::Buffer m_buffer{};
    ghi::Device m_device{};
  };

  struct InternalCamera : Camera
  {
    auto operator=(const Camera &camera) noexcept -> void;

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

  struct UBO_Unlit_Per_Scene
  {
    glm::mat4 projection_matrix{};
  };

  struct UBO_Unlit_Per_Frame
  {
    glm::mat4 view_matrix{};
  };

  struct UBO_Unlit_Per_Draw
  {
    glm::mat4 model_matrix{};
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

    UniformBuffer<UBO_Unlit_Per_Scene> ubo_unlit_per_scene;
    UniformBuffer<UBO_Unlit_Per_Frame> ubo_unlit_per_frame;
    UniformBuffer<UBO_Unlit_Per_Draw> ubo_unlit_per_draw;

    ghi::BindingLayout unlit_pipeline_binding_layout{};
    ghi::DescriptorTable unlit_pipeline_descriptor_table{};
  };
} // namespace iavis