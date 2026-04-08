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

#include <iavis/iavis.hpp>

#include <base.hpp>
#include <iaghi/utils.hpp>

namespace iavis
{
  Context g_context{};

  auto InternalCamera::operator=(const Camera &camera) const noexcept -> void
  {
  }

  auto intialize_pipelines() -> Result<void>;

  auto initialize(const InitInfo &init_info) -> Result<void>
  {
    g_context.device = AU_TRY(ghi::create_device(init_info));

    AU_TRY_DISCARD(ghi::utils::initialize(g_context.device));

    ghi::get_swapchain_extent(g_context.device, g_context.surface_width, g_context.surface_height);

    AU_TRY_DISCARD(intialize_pipelines());

    return {};
  }

  auto shutdown() -> void
  {
    ghi::wait_idle(g_context.device);

    for (usize i = 0; i < g_context.geometries.size(); i++)
      destroy_geometry(i);

    ghi::destroy_pipeline(g_context.device, g_context.unlit_2d_pipeline);

    ghi::utils::shutdown(g_context.device);

    ghi::destroy_device(g_context.device);
  }

  auto resize(u32 width, u32 height) -> Result<void>
  {
    AU_TRY_DISCARD(ghi::resize_swapchain(g_context.device, width, height));
    g_context.surface_width = width;
    g_context.surface_height = height;
    return {};
  }

  auto get_surface_width() -> u32
  {
    return g_context.surface_width;
  }

  auto get_surface_height() -> u32
  {
    return g_context.surface_height;
  }

  auto set_camera(const Camera &camera) -> void
  {
    g_context.camera = camera;
  }

  auto set_clear_color(f32 r, f32 g, f32 b) -> void
  {
    ghi::set_clear_color(r, g, b, 1.0f);
  }

  auto add_drawable(GeomId geometry, MatId material, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
      -> DrawableId
  {
    if (geometry >= g_context.geometries.size())
      return INVALID_ID;
    //if (material >= g_context.materials.size())
    //  return INVALID_ID;
    const DrawableId id = g_context.drawables.size();
    g_context.drawables.push_back(Drawable{
        .active = true,
        .geometry = g_context.geometries[geometry],
        .material = {},//g_context.materials[material],
        .transform = glm::translate(glm::mat4(1.0f), position) *
                     glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
                     glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                     glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
                     glm::scale(glm::mat4(1.0f), scale),
    });
    return id;
  }

  auto render() -> void
  {
    const auto cmd = ghi::begin_frame(g_context.device);

    ghi::cmd_set_viewport(cmd, 0, 0, g_context.surface_width, g_context.surface_height);
    ghi::cmd_set_scissor(cmd, 0, 0, g_context.surface_width, g_context.surface_height);

    ghi::cmd_bind_pipeline(cmd, g_context.unlit_2d_pipeline);

    for (const auto &drawable : g_context.drawables)
    {
      const u64 offset{0};
      ghi::cmd_bind_vertex_buffers(cmd, 0, 1, &drawable.geometry.vertex_buffer, &offset);
      ghi::cmd_bind_index_buffer(cmd, drawable.geometry.index_buffer, 0, true);
      ghi::cmd_draw_indexed(cmd, drawable.geometry.index_count, 1, 0, 0, 0);
    }

    ghi::end_frame(g_context.device);
  }

  /*
  *  auto draw_geometry(GeomId id, const glm::mat4& model_matrix) -> void
  {

  }

   */

  auto create_geometry_unlit_2d(Span<const VertexUnlit2DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>
  {
    GeomId id = g_context.geometries.size();
    g_context.geometries.push_back({
        .vertex_buffer = AU_TRY(ghi::utils::create_device_local_buffer(g_context.device, ghi::EBufferUsage::Vertex,
                                                                       vertices.size_bytes(), vertices.data(),
                                                                       vertices.size_bytes())),
        .index_buffer = AU_TRY(ghi::utils::create_device_local_buffer(
            g_context.device, ghi::EBufferUsage::Index, indices.size_bytes(), indices.data(), indices.size_bytes())),
        .index_count = static_cast<u32>(indices.size()),
    });
    return id;
  }

  auto create_geometry_unlit_3d(Span<const VertexUnlit3DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>
  {
    GeomId id = g_context.geometries.size();
    g_context.geometries.push_back({
        .vertex_buffer = AU_TRY(ghi::utils::create_device_local_buffer(g_context.device, ghi::EBufferUsage::Vertex,
                                                                       vertices.size_bytes(), vertices.data(),
                                                                       vertices.size_bytes())),
        .index_buffer = AU_TRY(ghi::utils::create_device_local_buffer(
            g_context.device, ghi::EBufferUsage::Index, indices.size_bytes(), indices.data(), indices.size_bytes())),
        .index_count = static_cast<u32>(indices.size()),
    });
    return id;
  }

  auto destroy_geometry(GeomId id) -> void
  {
    if (id >= g_context.geometries.size())
      return;
    auto &geometry = g_context.geometries[id];
    if (!geometry.index_count)
      return;
    ghi::destroy_buffers(g_context.device, 1, &geometry.vertex_buffer);
    ghi::destroy_buffers(g_context.device, 1, &geometry.index_buffer);
    geometry.index_count = 0;
  }

  auto create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps) -> Result<TexId>
  {
  }

  auto destroy_texture(TexId id) -> void
  {
  }

  auto create_material(TexId albedo_tex, TexId normal_tex, TexId height_tex, TexId roughness_tex, TexId ao_tex)
      -> Result<MatId>
  {
  }

  auto destroy_material(MatId id) -> void
  {
  }
} // namespace iavis

namespace iavis
{
  const auto VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 vUV;

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    vUV = inTexCoord;
}
)";

  const auto FRAGMENT_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(vUV, 0.0, 1.0);
}
)";

  auto intialize_pipelines() -> Result<void>
  {
    const auto vertex_shader =
        AU_TRY(ghi::utils::create_shader_from_glsl(g_context.device, VERTEX_SHADER_SRC, ghi::EShaderStage::Vertex));
    const auto fragment_shader =
        AU_TRY(ghi::utils::create_shader_from_glsl(g_context.device, FRAGMENT_SHADER_SRC, ghi::EShaderStage::Fragment));

    auto color_format = ghi::get_swapchain_format(g_context.device);

    ghi::VertexInputBinding vertex_input_binding{
        .binding = 0,
        .stride = sizeof(glm::vec4),
        .input_rate = ghi::EInputRate::Vertex,
    };

    ghi::VertexInputAttribute vertex_input_attributes[2] = {
        {.location = 0, .binding = 0, .format = ghi::EFormat::R32G32Float, .offset = 0},
        {.location = 1, .binding = 0, .format = ghi::EFormat::R32G32Float, .offset = sizeof(glm::vec2)},
    };

    const ghi::GraphicsPipelineDesc unlit_2d_pipeline_desc{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,

        .color_formats = &color_format,
        .color_attachment_count = 1,
        .depth_format = ghi::EFormat::D32Sfloat,
        .cull_mode = ghi::ECullMode::None,

        .binding_layouts = {},
        .vertex_bindings = {vertex_input_binding},
        .vertex_attributes = {vertex_input_attributes},
    };
    g_context.unlit_2d_pipeline = AU_TRY(ghi::create_graphics_pipeline(g_context.device, &unlit_2d_pipeline_desc));

    ghi::destroy_shader(g_context.device, vertex_shader);
    ghi::destroy_shader(g_context.device, fragment_shader);

    return {};
  }
} // namespace iavis