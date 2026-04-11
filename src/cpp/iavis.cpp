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

  auto InternalCamera::operator=(const Camera &camera) noexcept -> void
  {
    projection_matrix =
        camera.projection == EProjection::PERSPECTIVE
            ? glm::perspective(glm::radians(camera.fov),
                               static_cast<f32>(g_context.surface_width) / static_cast<f32>(g_context.surface_height),
                               camera.near_plane, camera.far_plane)
            : glm::ortho(0.0f, static_cast<f32>(g_context.surface_width), 0.0f,
                         -static_cast<f32>(g_context.surface_height), camera.near_plane, camera.far_plane);
    projection_matrix[1][1] *= -1.0f;
    view_matrix = glm::lookAtLH(camera.position, camera.position + camera.forward, camera.up);
  }

  auto initialize_pipelines() -> Result<void>;

  auto initialize(const InitInfo &init_info) -> Result<void>
  {
    g_context.device = AU_TRY(ghi::create_device(init_info));

    AU_TRY_DISCARD(ghi::utils::initialize(g_context.device));

    ghi::get_swapchain_extent(g_context.device, g_context.surface_width, g_context.surface_height);

    AU_TRY_DISCARD(initialize_pipelines());

    g_context.ubo_unlit_per_scene.data().projection_matrix = g_context.camera.projection_matrix;
    AU_TRY_DISCARD(g_context.ubo_unlit_per_scene.flush());

    ghi::SamplerDesc sampler_desc{
        .linear_filter = true,
        .repeat_uv = true,
    };
    AU_TRY_DISCARD(ghi::create_samplers(g_context.device, 1, &sampler_desc, &g_context.sampler_repeat));
    sampler_desc.repeat_uv = false;
    AU_TRY_DISCARD(ghi::create_samplers(g_context.device, 1, &sampler_desc, &g_context.sampler_clamp));

    ghi::DescriptorUpdate unlit_material_descriptor_updates = {
        .table = g_context.unlit_pipeline_material_descriptor_table,
        .binding = 0,
        .array_element = 0,

        .image = ghi::utils::get_default_image(),
        .sampler = g_context.sampler_repeat,
        .image_update_all_frames = true,
    };
    ghi::update_descriptor_tables(g_context.device, 1, &unlit_material_descriptor_updates);

    return {};
  }

  auto shutdown() -> void
  {
    ghi::wait_idle(g_context.device);

    ghi::destroy_images(g_context.device, g_context.textures.size(), g_context.textures.data());

    for (usize i = 0; i < g_context.geometries.size(); i++)
      destroy_geometry(i);

    g_context.ubo_unlit_per_scene.destroy();
    g_context.ubo_unlit_per_frame.destroy();
    g_context.ubo_unlit_per_draw_vs.destroy();
    g_context.ubo_unlit_per_draw_fs.destroy();

    ghi::destroy_samplers(g_context.device, 1, &g_context.sampler_clamp);
    ghi::destroy_samplers(g_context.device, 1, &g_context.sampler_repeat);

    ghi::destroy_pipeline(g_context.device, g_context.unlit_2d_pipeline);
    ghi::destroy_pipeline(g_context.device, g_context.unlit_3d_pipeline);

    ghi::destroy_binding_layout(g_context.device, g_context.unlit_pipeline_binding_layout);
    ghi::destroy_binding_layout(g_context.device, g_context.unlit_pipeline_material_binding_layout);

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
    g_context.ubo_unlit_per_scene.data().projection_matrix = g_context.camera.projection_matrix;
    AU_UNUSED(g_context.ubo_unlit_per_scene.flush(true));
  }

  auto set_clear_color(f32 r, f32 g, f32 b) -> void
  {
    ghi::set_clear_color(r, g, b, 1.0f);
  }

  auto add_drawable(GeomId geometry, MatId material, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
                    glm::vec2 tex_coords) -> DrawableId
  {
    if (geometry >= g_context.geometries.size())
      return INVALID_ID;
    if (material >= g_context.materials.size())
      return INVALID_ID;
    const auto geom = g_context.geometries[geometry];
    auto &drawables = geom.is_3d ? g_context.unlit_3d_drawables[material] : g_context.unlit_2d_drawables[material];
    const auto drawable = new Drawable{
        .active = true,
        .geometry = geom,
        .tex_coords = tex_coords,
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .is_transform_dirty = true,
    };
    drawables.push_back(drawable);
    return reinterpret_cast<DrawableId>(drawable);
  }

  auto set_drawable_position(DrawableId id, glm::vec3 position) -> void
  {
    const auto drawable = reinterpret_cast<Drawable *>(id);
    drawable->position = position;
    drawable->is_transform_dirty = true;
  }

  auto set_drawable_rotation(DrawableId id, glm::quat rotation) -> void
  {
    const auto drawable = reinterpret_cast<Drawable *>(id);
    drawable->rotation = rotation;
    drawable->is_transform_dirty = true;
  }

  auto set_drawable_scale(DrawableId id, glm::vec3 scale) -> void
  {
    const auto drawable = reinterpret_cast<Drawable *>(id);
    drawable->scale = scale;
    drawable->is_transform_dirty = true;
  }

  auto set_drawable_tex_coords(DrawableId id, glm::vec2 tex_coords) -> void
  {
    const auto drawable = reinterpret_cast<Drawable *>(id);
    drawable->tex_coords = tex_coords;
  }

  auto render() -> void
  {
    g_context.ubo_unlit_per_frame.data().view_matrix = g_context.camera.view_matrix;
    AU_UNUSED(g_context.ubo_unlit_per_frame.flush());

    const auto cmd = ghi::begin_frame(g_context.device);

    ghi::cmd_set_viewport(cmd, 0, 0, g_context.surface_width, g_context.surface_height);
    ghi::cmd_set_scissor(cmd, 0, 0, g_context.surface_width, g_context.surface_height);

    ghi::cmd_bind_pipeline(cmd, g_context.unlit_2d_pipeline);
    ghi::cmd_bind_descriptor_table(cmd, 0, g_context.unlit_2d_pipeline, g_context.unlit_pipeline_descriptor_table);
    ghi::cmd_bind_descriptor_table(cmd, 1, g_context.unlit_2d_pipeline,
                                   g_context.unlit_pipeline_material_descriptor_table);
    for (const auto [matId, drawables] : g_context.unlit_2d_drawables)
    {
      ghi::DescriptorUpdate unlit_material_descriptor_updates = {
          .table = g_context.unlit_pipeline_material_descriptor_table,
          .binding = 0,
          .array_element = 0,

          .image = g_context.materials[matId].albedo_texture,
          .sampler = g_context.sampler_repeat,
      };
      ghi::update_descriptor_tables(g_context.device, 1, &unlit_material_descriptor_updates);

      for (auto &drawable : drawables)
      {
        drawable->update_transform();
        g_context.ubo_unlit_per_draw_vs.data().model_matrix = drawable->transform;
        AU_UNUSED(g_context.ubo_unlit_per_draw_vs.flush());

        g_context.ubo_unlit_per_draw_fs.data().tex_coords = drawable->tex_coords;
        AU_UNUSED(g_context.ubo_unlit_per_draw_fs.flush());

        const u64 offset{0};
        ghi::cmd_bind_vertex_buffers(cmd, 0, 1, &drawable->geometry.vertex_buffer, &offset);
        ghi::cmd_bind_index_buffer(cmd, drawable->geometry.index_buffer, 0, true);
        ghi::cmd_draw_indexed(cmd, drawable->geometry.index_count, 1, 0, 0, 0);
      }
    }

    ghi::cmd_bind_pipeline(cmd, g_context.unlit_3d_pipeline);
    ghi::cmd_bind_descriptor_table(cmd, 0, g_context.unlit_3d_pipeline, g_context.unlit_pipeline_descriptor_table);
    ghi::cmd_bind_descriptor_table(cmd, 1, g_context.unlit_3d_pipeline,
                                   g_context.unlit_pipeline_material_descriptor_table);
    for (const auto [matId, drawables] : g_context.unlit_3d_drawables)
    {
      ghi::DescriptorUpdate unlit_material_descriptor_updates = {
          .table = g_context.unlit_pipeline_material_descriptor_table,
          .binding = 0,
          .array_element = 0,

          .image = g_context.materials[matId].albedo_texture,
          .sampler = g_context.sampler_repeat,
      };
      ghi::update_descriptor_tables(g_context.device, 1, &unlit_material_descriptor_updates);

      for (auto &drawable : drawables)
      {
        drawable->update_transform();
        g_context.ubo_unlit_per_draw_vs.data().model_matrix = drawable->transform;
        AU_UNUSED(g_context.ubo_unlit_per_draw_vs.flush());

        g_context.ubo_unlit_per_draw_fs.data().tex_coords = drawable->tex_coords;
        AU_UNUSED(g_context.ubo_unlit_per_draw_fs.flush());

        const u64 offset{0};
        ghi::cmd_bind_vertex_buffers(cmd, 0, 1, &drawable->geometry.vertex_buffer, &offset);
        ghi::cmd_bind_index_buffer(cmd, drawable->geometry.index_buffer, 0, true);
        ghi::cmd_draw_indexed(cmd, drawable->geometry.index_count, 1, 0, 0, 0);
      }
    }

    ghi::end_frame(g_context.device);
  }

  auto create_geometry_unlit_2d(Span<const VertexUnlit2DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>
  {
    GeomId id = g_context.geometries.size();
    g_context.geometries.push_back({
        .is_3d = false,
        .is_lit = false,
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
        .is_3d = true,
        .is_lit = false,
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
    const auto img = AU_TRY(ghi::utils::create_image_from_rgba(g_context.device, width, height, rgba_data));
    g_context.textures.push_back(img);
    return g_context.textures.size() - 1;
  }

  auto create_texture_from_file(const char *filename, bool generate_mipmaps) -> Result<TexId>
  {
    const auto img = AU_TRY(ghi::utils::create_image_from_file(g_context.device, filename));
    g_context.textures.push_back(img);
    return g_context.textures.size() - 1;
  }

  auto destroy_texture(TexId id) -> void
  {
    if (id >= g_context.textures.size())
      return;
    ghi::destroy_images(g_context.device, 1, &g_context.textures[id]);
    g_context.textures[id] = {};
  }

  auto create_material(TexId albedo_tex, TexId normal_tex, TexId height_tex, TexId roughness_tex, TexId ao_tex)
      -> Result<MatId>
  {
    g_context.materials.push_back(Material{
        .albedo_texture = (albedo_tex <= g_context.textures.size() ? g_context.textures[albedo_tex]
                                                                   : ghi::utils::get_default_image()),
        .normal_texture = (normal_tex <= g_context.textures.size() ? g_context.textures[normal_tex]
                                                                   : ghi::utils::get_default_image()),
        .height_texture = (height_tex <= g_context.textures.size() ? g_context.textures[height_tex]
                                                                   : ghi::utils::get_default_image()),
        .roughness_texture = (roughness_tex <= g_context.textures.size() ? g_context.textures[roughness_tex]
                                                                         : ghi::utils::get_default_image()),
        .ao_texture =
            (ao_tex <= g_context.textures.size() ? g_context.textures[ao_tex] : ghi::utils::get_default_image()),
    });
    return g_context.materials.size() - 1;
  }

  auto destroy_material(MatId id) -> void
  {
    if (id >= g_context.materials.size())
      return;
    g_context.materials[id] = {};
  }
} // namespace iavis

namespace iavis
{
  // [IATODO]: Precompile these
  const auto UNLIT_2D_VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 vUV;

layout(set = 0, binding = 0) uniform UBOPerScene_T {
    mat4 projection;
} PerSceneUBO;

layout(set = 0, binding = 1) uniform UBOPerFrame_T {
    mat4 view;
} PerFrameUBO;

layout(set = 0, binding = 2) uniform UBOPerDraw_T {
    mat4 model;
} PerDrawUBO;

void main()
{
    gl_Position = PerSceneUBO.projection * PerFrameUBO.view * PerDrawUBO.model * vec4(inPosition, 0.0, 1.0);
    vUV = inTexCoord;
}
)";

  const auto UNLIT_3D_VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 vUV;

layout(set = 0, binding = 0) uniform UBOPerScene_T {
    mat4 projection;
} PerSceneUBO;

layout(set = 0, binding = 1) uniform UBOPerFrame_T {
    mat4 view;
} PerFrameUBO;

layout(set = 0, binding = 2) uniform UBOPerDraw_T {
    mat4 model;
} PerDrawUBO;

void main()
{
    gl_Position = PerSceneUBO.projection * PerFrameUBO.view * PerDrawUBO.model * vec4(inPosition, 1.0);
    vUV = inTexCoord;
}
)";

  const auto UNLIT_FRAGMENT_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 3) uniform UBOPerDrawFS_T {
    vec2 tex_coords;
} PerDrawFSUBO;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
    outColor = texture(tex, vUV + PerDrawFSUBO.tex_coords);
}
)";

  auto initialize_pipelines() -> Result<void>
  {
    g_context.ubo_unlit_per_scene = AU_TRY(UniformBuffer<UBO_Unlit_Per_Scene>::create(g_context.device));
    g_context.ubo_unlit_per_frame = AU_TRY(UniformBuffer<UBO_Unlit_Per_Frame>::create(g_context.device));
    g_context.ubo_unlit_per_draw_vs = AU_TRY(UniformBuffer<UBO_Unlit_Per_Draw_VS>::create(g_context.device));
    g_context.ubo_unlit_per_draw_fs = AU_TRY(UniformBuffer<UBO_Unlit_Per_Draw_FS>::create(g_context.device));

    const auto unlit_2d_vertex_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_2D_VERTEX_SHADER_SRC, ghi::EShaderStage::Vertex));
    const auto unlit_3d_vertex_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_3D_VERTEX_SHADER_SRC, ghi::EShaderStage::Vertex));
    const auto unlit_fragment_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_FRAGMENT_SHADER_SRC, ghi::EShaderStage::Fragment));

    auto color_format = ghi::get_swapchain_format(g_context.device);

    g_context.unlit_pipeline_binding_layout =
        AU_TRY(ghi::create_binding_layout(g_context.device, {
                                                                {
                                                                    .binding = 0,
                                                                    .count = 1,
                                                                    .visibility = ghi::EShaderStage::Vertex,
                                                                    .type = ghi::EDescriptorType::UniformBuffer,
                                                                },
                                                                {
                                                                    .binding = 1,
                                                                    .count = 1,
                                                                    .visibility = ghi::EShaderStage::Vertex,
                                                                    .type = ghi::EDescriptorType::UniformBuffer,
                                                                },
                                                                {
                                                                    .binding = 2,
                                                                    .count = 1,
                                                                    .visibility = ghi::EShaderStage::Vertex,
                                                                    .type = ghi::EDescriptorType::UniformBuffer,
                                                                },
                                                                {
                                                                    .binding = 3,
                                                                    .count = 1,
                                                                    .visibility = ghi::EShaderStage::Fragment,
                                                                    .type = ghi::EDescriptorType::UniformBuffer,
                                                                },
                                                            }));
    g_context.unlit_pipeline_material_binding_layout =
        AU_TRY(ghi::create_binding_layout(g_context.device, {
                                                                {
                                                                    .binding = 0,
                                                                    .count = 1,
                                                                    .visibility = ghi::EShaderStage::Fragment,
                                                                    .type = ghi::EDescriptorType::CombinedImageSampler,
                                                                },
                                                            }));

    ghi::GraphicsPipelineDesc unlit_pipeline_desc{
        .vertex_shader = unlit_2d_vertex_shader,
        .fragment_shader = unlit_fragment_shader,

        .color_formats = &color_format,
        .color_attachment_count = 1,
        .depth_format = ghi::EFormat::D32Sfloat,
        .cull_mode = ghi::ECullMode::None,

        .binding_layouts =
            {
                g_context.unlit_pipeline_binding_layout,
                g_context.unlit_pipeline_material_binding_layout,
            },
        .vertex_bindings =
            {
                ghi::VertexInputBinding{
                    .binding = 0,
                    .stride = sizeof(glm::vec4),
                    .input_rate = ghi::EInputRate::Vertex,
                },
            },
        .vertex_attributes =
            {
                {.location = 0, .binding = 0, .format = ghi::EFormat::R32G32Float, .offset = 0},
                {.location = 1, .binding = 0, .format = ghi::EFormat::R32G32Float, .offset = sizeof(glm::vec2)},
            },
    };
    g_context.unlit_2d_pipeline = AU_TRY(ghi::create_graphics_pipeline(g_context.device, &unlit_pipeline_desc));
    unlit_pipeline_desc.vertex_shader = unlit_3d_vertex_shader;
    unlit_pipeline_desc.vertex_bindings = {
        ghi::VertexInputBinding{
            .binding = 0,
            .stride = sizeof(glm::vec4) + sizeof(glm::vec2),
            .input_rate = ghi::EInputRate::Vertex,
        },
    };
    unlit_pipeline_desc.vertex_attributes = {
        {.location = 0, .binding = 0, .format = ghi::EFormat::R32G32B32A32Float, .offset = 0},
        {.location = 1, .binding = 0, .format = ghi::EFormat::R32G32Float, .offset = sizeof(glm::vec4)},
    };
    g_context.unlit_3d_pipeline = AU_TRY(ghi::create_graphics_pipeline(g_context.device, &unlit_pipeline_desc));

    ghi::destroy_shader(g_context.device, unlit_2d_vertex_shader);
    ghi::destroy_shader(g_context.device, unlit_3d_vertex_shader);
    ghi::destroy_shader(g_context.device, unlit_fragment_shader);

    AU_TRY_DISCARD(ghi::create_descriptor_tables(g_context.device, g_context.unlit_pipeline_binding_layout, 1,
                                                 &g_context.unlit_pipeline_descriptor_table));

    AU_TRY_DISCARD(ghi::create_descriptor_tables(g_context.device, g_context.unlit_pipeline_material_binding_layout, 1,
                                                 &g_context.unlit_pipeline_material_descriptor_table));

    ghi::DescriptorUpdate unlit_pipeline_descriptor_updates[4] = {
        {
            .table = g_context.unlit_pipeline_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_per_scene.get_handle(),
            .buffer_offset = 0,
            .buffer_range = 0,
        },
        {
            .table = g_context.unlit_pipeline_descriptor_table,
            .binding = 1,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_per_frame.get_handle(),
            .buffer_offset = 0,
            .buffer_range = 0,
        },
        {
            .table = g_context.unlit_pipeline_descriptor_table,
            .binding = 2,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_per_draw_vs.get_handle(),
            .buffer_offset = 0,
            .buffer_range = 0,
        },
        {
            .table = g_context.unlit_pipeline_descriptor_table,
            .binding = 3,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_per_draw_fs.get_handle(),
            .buffer_offset = 0,
            .buffer_range = 0,
        },
    };
    ghi::update_descriptor_tables(g_context.device, 4, unlit_pipeline_descriptor_updates);

    return {};
  }
} // namespace iavis