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
  const Vec<VertexUnlit2DGeometry> QUAD_VERTICES = {
      {{-1.0f, -1.0f}, {0.0f, 0.0f}},
      {{1.0f, -1.0f}, {1.0f, 0.0f}},
      {{1.0f, 1.0f}, {1.0f, 1.0f}},
      {{-1.0f, 1.0f}, {0.0f, 1.0f}},
  };

  const Vec<u32> QUAD_INDICES = {2, 1, 0, 3, 2, 0};

  const Vec<VertexUnlit3DGeometry> CUBE_VERTICES = {
      {{-0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, 0.0f}},  {{0.5f, -0.5f, 0.5f}, 0.0f, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, 0.0f, {1.0f, 1.0f}},    {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, 0.0f, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, 0.0f, {1.0f, 1.0f}},  {{0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, 0.0f}}, {{-0.5f, -0.5f, 0.5f}, 0.0f, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, 0.0f, {1.0f, 1.0f}},   {{-0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, 0.0f}},   {{0.5f, -0.5f, -0.5f}, 0.0f, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, 0.0f, {1.0f, 1.0f}},   {{0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 0.0f}},   {{0.5f, 0.5f, 0.5f}, 0.0f, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, 0.0f, {1.0f, 1.0f}},   {{-0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, 0.0f, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, 0.0f, {1.0f, 1.0f}},   {{-0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, 1.0f}}};

  const Vec<u32> CUBE_INDICES = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
                                 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

  void generate_cylinder(f32 radius, f32 height, u32 sectorCount, Vec<VertexUnlit3DGeometry> &outVertices,
                         Vec<u32> &outIndices)
  {
    outVertices.clear();
    outIndices.clear();

    f32 halfHeight = height / 2.0f;
    f32 sectorStep = 2.0f * glm::pi<f32>() / sectorCount;

    for (u32 i = 0; i <= sectorCount; ++i)
    {
      f32 sectorAngle = i * sectorStep;
      f32 x = radius * cos(sectorAngle);
      f32 z = radius * sin(sectorAngle);
      f32 u = (f32) i / sectorCount;

      outVertices.push_back({{x, -halfHeight, z}, 0.0f, {u, 1.0f}});
      outVertices.push_back({{x, halfHeight, z}, 0.0f, {u, 0.0f}});
    }

    for (u32 i = 0; i < sectorCount; ++i)
    {
      u32 k1 = i * 2;
      u32 k2 = k1 + 1;
      u32 k3 = k1 + 2;
      u32 k4 = k1 + 3;

      outIndices.push_back(k1);
      outIndices.push_back(k2);
      outIndices.push_back(k3);
      outIndices.push_back(k3);
      outIndices.push_back(k2);
      outIndices.push_back(k4);
    }
  }

  void generate_sphere(f32 radius, u32 sectorCount, u32 stackCount, Vec<VertexUnlit3DGeometry> &outVertices,
                       Vec<u32> &outIndices)
  {
    outVertices.clear();
    outIndices.clear();

    f32 x, y, z, xy;
    f32 sectorStep = 2.0f * glm::pi<f32>() / sectorCount;
    f32 stackStep = glm::pi<f32>() / stackCount;
    f32 sectorAngle, stackAngle;

    for (u32 i = 0; i <= stackCount; ++i)
    {
      stackAngle = glm::pi<f32>() / 2.0f - i * stackStep;
      xy = radius * cos(stackAngle);
      y = radius * sin(stackAngle);

      for (u32 j = 0; j <= sectorCount; ++j)
      {
        sectorAngle = j * sectorStep;

        x = xy * cos(sectorAngle);
        z = xy * sin(sectorAngle);

        f32 u = (f32) j / sectorCount;
        f32 v = (f32) i / stackCount;

        outVertices.push_back({{x, y, z}, 0.0f, {u, 1.0 - v}});
      }
    }

    u32 k1, k2;
    for (u32 i = 0; i < stackCount; ++i)
    {
      k1 = i * (sectorCount + 1);
      k2 = k1 + sectorCount + 1;

      for (u32 j = 0; j < sectorCount; ++j, ++k1, ++k2)
      {
        if (i != 0)
        {
          outIndices.push_back(k1);
          outIndices.push_back(k2);
          outIndices.push_back(k1 + 1);
        }
        if (i != (stackCount - 1))
        {
          outIndices.push_back(k1 + 1);
          outIndices.push_back(k2);
          outIndices.push_back(k2 + 1);
        }
      }
    }
  }
} // namespace iavis

namespace iavis
{
  Context g_context{};

  GeomId g_quad_geometry_id{INVALID_ID};
  GeomId g_cube_geometry_id{INVALID_ID};
  GeomId g_sphere_geometry_id{INVALID_ID};
  GeomId g_cylinder_geometry_id{INVALID_ID};

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

    g_context.ub_unlit_global.projection_matrix = g_context.camera.projection_matrix;
    g_context.ub_unlit_global.projection_matrix[1][1] *= -1.0f;
    {
      const auto ptr = ghi::map_buffer(g_context.device, g_context.ubo_unlit_global);
      memcpy(ptr, &g_context.ub_unlit_global, sizeof(g_context.ub_unlit_global));
      ghi::unmap_buffer(g_context.device, g_context.ubo_unlit_global);
    }

    AU_TRY_DISCARD(ghi::create_samplers(g_context.device,
                                        {
                                            {
                                                .linear_filter = true,
                                                .repeat_uv = true,
                                            },
                                            {
                                                .linear_filter = true,
                                                .repeat_uv = false,
                                            },
                                        },
                                        {&g_context.sampler_repeat, &g_context.sampler_clamp}));

    ghi::DescriptorUpdate unlit_material_descriptor_updates = {
        .table = g_context.texture_data_descriptor_table,
        .binding = 0,
        .array_element = 0,

        .image = ghi::utils::get_default_image(),
        .sampler = g_context.sampler_repeat,
    };
    ghi::update_descriptor_tables(g_context.device, {unlit_material_descriptor_updates});

    g_quad_geometry_id = AU_TRY(create_geometry_unlit_2d(QUAD_VERTICES, QUAD_INDICES));
    g_cube_geometry_id = AU_TRY(create_geometry_unlit_3d(CUBE_VERTICES, CUBE_INDICES));
    {
      Vec<VertexUnlit3DGeometry> vertices;
      Vec<u32> indices;
      generate_sphere(1.0f, 16, 16, vertices, indices);
      g_sphere_geometry_id = AU_TRY(create_geometry_unlit_3d(vertices, indices));
    }
    {
      Vec<VertexUnlit3DGeometry> vertices;
      Vec<u32> indices;
      generate_cylinder(1.0f, 1.0f, 16, vertices, indices);
      g_cylinder_geometry_id = AU_TRY(create_geometry_unlit_3d(vertices, indices));
    }

    g_context.materials.push_back(Material{0});

    return {};
  }

  auto shutdown() -> void
  {
    ghi::wait_idle(g_context.device);

    ghi::destroy_buffers(g_context.device, {g_context.material_storage_buffer});

    ghi::destroy_images(g_context.device, g_context.textures);

    for (usize i = 0; i < g_context.geometries.size(); i++)
      destroy_geometry(i);

    ghi::destroy_buffers(g_context.device, {g_context.ubo_unlit_global, g_context.ubo_unlit_per_frame});

    ghi::destroy_samplers(g_context.device, {g_context.sampler_clamp, g_context.sampler_repeat});

    ghi::destroy_pipeline(g_context.device, g_context.unlit_2d_pipeline);
    ghi::destroy_pipeline(g_context.device, g_context.unlit_3d_pipeline);

    ghi::destroy_binding_layouts(g_context.device, {g_context.unlit_pipeline_global_data_binding_layout,
                                                    g_context.unlit_pipeline_per_frame_data_binding_layout,
                                                    g_context.texture_data_binding_layout});

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
    g_context.ub_unlit_global.projection_matrix = g_context.camera.projection_matrix;
    const auto ptr = ghi::map_buffer(g_context.device, g_context.ubo_unlit_global);
    memcpy(ptr, &g_context.ub_unlit_global, sizeof(g_context.ub_unlit_global));
    ghi::unmap_buffer(g_context.device, g_context.ubo_unlit_global);
  }

  auto set_clear_color(f32 r, f32 g, f32 b) -> void
  {
    ghi::set_clear_color(g_context.device, r, g, b, 1.0f);
  }

  auto get_quad_geometry() -> GeomId
  {
    return g_quad_geometry_id;
  }

  auto get_cube_geometry() -> GeomId
  {
    return g_cube_geometry_id;
  }

  auto get_sphere_geometry() -> GeomId
  {
    return g_sphere_geometry_id;
  }

  auto get_cylinder_geometry() -> GeomId
  {
    return g_cylinder_geometry_id;
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
    {
      g_context.ub_unlit_per_frame.view_matrix = g_context.camera.view_matrix;
      const auto ptr = ghi::map_frame_bound_buffer(g_context.device, g_context.ubo_unlit_per_frame);
      memcpy(ptr, &g_context.ub_unlit_per_frame, sizeof(g_context.ub_unlit_per_frame));
      ghi::unmap_buffer(g_context.device, g_context.ubo_unlit_per_frame);
    }

    const auto cmd = ghi::begin_frame(g_context.device);

    ghi::cmd_set_viewport(cmd, 0, 0, g_context.surface_width, g_context.surface_height);
    ghi::cmd_set_scissor(cmd, 0, 0, g_context.surface_width, g_context.surface_height);

    ghi::cmd_bind_pipeline(cmd, g_context.unlit_2d_pipeline);
    ghi::cmd_bind_descriptor_table(cmd, 0, g_context.unlit_2d_pipeline,
                                   g_context.unlit_pipeline_global_data_descriptor_table, {});
    ghi::cmd_bind_frame_bound_descriptor_table(cmd, 1, g_context.unlit_2d_pipeline,
                                               g_context.unlit_pipeline_per_frame_binding_descriptor_table);
    ghi::cmd_bind_descriptor_table(cmd, 2, g_context.unlit_2d_pipeline, g_context.texture_data_descriptor_table, {});
    for (const auto [matId, drawables] : g_context.unlit_2d_drawables)
    {
      for (auto &drawable : drawables)
      {
        drawable->update_transform();

        const PC_Unlit_Per_Draw pc{
            .model_matrix = drawable->transform,
            .tex_coords = drawable->tex_coords,
            .material_index = (u32)matId,
        };
        ghi::cmd_push_constants(cmd, g_context.unlit_2d_pipeline, 0, sizeof(pc), &pc);

        ghi::cmd_bind_vertex_buffers(cmd, 0, {drawable->geometry.vertex_buffer}, {0});
        ghi::cmd_bind_index_buffer(cmd, drawable->geometry.index_buffer, 0, true);
        ghi::cmd_draw_indexed(cmd, drawable->geometry.index_count, 1, 0, 0, 0);
      }
    }

    ghi::cmd_bind_pipeline(cmd, g_context.unlit_3d_pipeline);
    ghi::cmd_bind_descriptor_table(cmd, 0, g_context.unlit_3d_pipeline,
                                   g_context.unlit_pipeline_global_data_descriptor_table, {});
    ghi::cmd_bind_frame_bound_descriptor_table(cmd, 1, g_context.unlit_3d_pipeline,
                                               g_context.unlit_pipeline_per_frame_binding_descriptor_table);
    ghi::cmd_bind_descriptor_table(cmd, 2, g_context.unlit_3d_pipeline, g_context.texture_data_descriptor_table, {});
    for (const auto [matId, drawables] : g_context.unlit_3d_drawables)
    {
      for (auto &drawable : drawables)
      {
        drawable->update_transform();

        const PC_Unlit_Per_Draw pc{
            .model_matrix = drawable->transform,
            .tex_coords = drawable->tex_coords,
            .material_index = (u32)matId,
        };
        ghi::cmd_push_constants(cmd, g_context.unlit_3d_pipeline, 0, sizeof(pc), &pc);

        ghi::cmd_bind_vertex_buffers(cmd, 0, {drawable->geometry.vertex_buffer}, {0});
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
    ghi::destroy_buffers(g_context.device, {geometry.vertex_buffer, geometry.index_buffer});
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
    ghi::destroy_images(g_context.device, {g_context.textures[id]});
    g_context.textures[id] = {};
  }

  auto create_material(TexId albedo_tex, TexId normal_tex, TexId height_tex, TexId roughness_tex, TexId ao_tex)
      -> Result<MatId>
  {
    g_context.materials.push_back(Material{
        .albedo_index = (u32)albedo_tex,
        .normal_index = (u32)normal_tex,
        .height_index = (u32)height_tex,
        .roughness_index = (u32)roughness_tex,
        .ao_index = (u32)ao_tex,
    });
    return g_context.materials.size() - 1;
  }

  auto destroy_material(MatId id) -> void
  {
    if (id >= g_context.materials.size())
      return;
    g_context.materials[id] = {};
  }

  auto flush_new_resources() -> Result<void>
  {
    Vec<ghi::DescriptorUpdate> updates;

    for (u32 i = 0; i < g_context.textures.size(); i++)
      updates.push_back({
          .table = g_context.texture_data_descriptor_table,
          .binding = 0,
          .array_element = i,

          .image = g_context.textures[i],
          .sampler = g_context.sampler_repeat,
      });

    ghi::update_descriptor_tables(g_context.device, updates);

    {
      const auto ptr = ghi::map_buffer(g_context.device, g_context.material_storage_buffer);
      memcpy(ptr, g_context.materials.data(), sizeof(g_context.materials[0]) * g_context.materials.size());
      ghi::unmap_buffer(g_context.device, g_context.material_storage_buffer);
    }

    return {};
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

layout(set = 0, binding = 0) uniform GlobalDataUBO_T {
    mat4 projection;
} global_data;

layout(set = 1, binding = 0) uniform PerFrameUBO_T {
    mat4 view;
} per_frame_data;

layout(push_constant, std430) uniform PushConstants {
    mat4 model_matrix;
    vec2 tex_coord_offset;
    uint material_index;
} pc;

void main()
{
    gl_Position = global_data.projection * per_frame_data.view * pc.model_matrix * vec4(inPosition, 0.0, 1.0);
    vUV = inTexCoord;
}
)";

  const auto UNLIT_3D_VERTEX_SHADER_SRC = R"(
#version 460
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 vUV;

layout(set = 0, binding = 0) uniform GlobalDataUBO_T {
    mat4 projection;
} global_data;

layout(set = 1, binding = 0) uniform PerFrameUBO_T {
    mat4 view;
} per_frame_data;

layout(push_constant, std430) uniform PushConstants {
    mat4 model_matrix;
    vec2 tex_coord_offset;
    uint material_index;
} pc;

void main()
{
    gl_Position = global_data.projection * per_frame_data.view * pc.model_matrix * vec4(inPosition, 1.0);
    vUV = inTexCoord;
}
)";

  const auto UNLIT_FRAGMENT_SHADER_SRC = R"(
#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D global_textures[];

struct Material {
    uint albedo_index;
    uint normal_index;
    uint height_index;
    uint roughness_index;
    uint ao_index;
    uint pad0, pad1, pad2;
};

layout(std430, set = 0, binding = 1) readonly buffer MaterialStorageBuffer {
    Material materials[];
};

layout(push_constant, std430) uniform PushConstants {
    mat4 model_matrix;
    vec2 tex_coord_offset;
    uint material_index;
} pc;

void main()
{
    Material mat = materials[pc.material_index];

    uint albedo_idx = mat.albedo_index;
    vec4 albedoColor = texture(global_textures[nonuniformEXT(albedo_idx)], vUV + pc.tex_coord_offset);

    outColor = albedoColor;
}
)";

  auto initialize_pipelines() -> Result<void>
  {
    AU_TRY_DISCARD(ghi::create_buffers(g_context.device,
                                       {
                                           {
                                               .size_bytes = sizeof(UBO_Unlit_Global),
                                               .usage = ghi::EBufferUsage::StaticUniform,
                                               .cpu_visible = true,
                                           },
                                           {
                                               .size_bytes = sizeof(UBO_Unlit_Per_Frame),
                                               .usage = ghi::EBufferUsage::FrameBoundUniform,
                                               .cpu_visible = true,
                                           },
                                       },
                                       {&g_context.ubo_unlit_global, &g_context.ubo_unlit_per_frame}));

    AU_TRY_DISCARD(ghi::create_buffers(g_context.device,
                                       {{
                                           .size_bytes = sizeof(Material) * 100,
                                           .usage = ghi::EBufferUsage::StaticStorage,
                                           .cpu_visible = true,
                                       }},
                                       {&g_context.material_storage_buffer}));

    const auto unlit_2d_vertex_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_2D_VERTEX_SHADER_SRC, ghi::EShaderStage::Vertex));
    const auto unlit_3d_vertex_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_3D_VERTEX_SHADER_SRC, ghi::EShaderStage::Vertex));
    const auto unlit_fragment_shader = AU_TRY(
        ghi::utils::create_shader_from_glsl(g_context.device, UNLIT_FRAGMENT_SHADER_SRC, ghi::EShaderStage::Fragment));

    auto color_format = ghi::get_swapchain_format(g_context.device);

    AU_TRY_DISCARD(ghi::create_binding_layouts(g_context.device,
                                               {
                                                   {
                                                       {
                                                           .binding = 0,
                                                           .count = 1,
                                                           .visibility = ghi::EShaderStage::Vertex,
                                                           .type = ghi::EDescriptorType::UniformBuffer,
                                                       },
                                                       {
                                                           .binding = 1,
                                                           .count = 1,
                                                           .visibility = ghi::EShaderStage::Fragment,
                                                           .type = ghi::EDescriptorType::StorageBuffer,
                                                       },
                                                   },
                                                   {
                                                       {
                                                           .binding = 0,
                                                           .count = 1,
                                                           .visibility = ghi::EShaderStage::Vertex,
                                                           .type = ghi::EDescriptorType::UniformBuffer,
                                                       },
                                                   },
                                                   {
                                                       {
                                                           .binding = 0,
                                                           .count = 100,
                                                           .visibility = ghi::EShaderStage::Fragment,
                                                           .type = ghi::EDescriptorType::CombinedImageSampler,
                                                       },
                                                   },
                                               },
                                               {&g_context.unlit_pipeline_global_data_binding_layout,
                                                &g_context.unlit_pipeline_per_frame_data_binding_layout,
                                                &g_context.texture_data_binding_layout}));

    ghi::GraphicsPipelineDesc unlit_pipeline_desc{
        .vertex_shader = unlit_2d_vertex_shader,
        .fragment_shader = unlit_fragment_shader,

        .color_formats = &color_format,
        .color_attachment_count = 1,
        .depth_format = ghi::EFormat::D32Sfloat,
        .cull_mode = ghi::ECullMode::None,

        .binding_layouts =
            {
                g_context.unlit_pipeline_global_data_binding_layout,
                g_context.unlit_pipeline_per_frame_data_binding_layout,
                g_context.texture_data_binding_layout,
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
        .push_constant_ranges =
            {
                {
                    .offset = 0,
                    .size = sizeof(PC_Unlit_Per_Draw),
                    .stages = (ghi::EShaderStage) ((u32) ghi::EShaderStage::Vertex | (u32) ghi::EShaderStage::Fragment),
                },
            },
    };
    g_context.unlit_2d_pipeline = AU_TRY(ghi::create_graphics_pipeline(g_context.device, unlit_pipeline_desc));
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
    g_context.unlit_3d_pipeline = AU_TRY(ghi::create_graphics_pipeline(g_context.device, unlit_pipeline_desc));

    ghi::destroy_shader(g_context.device, unlit_2d_vertex_shader);
    ghi::destroy_shader(g_context.device, unlit_3d_vertex_shader);
    ghi::destroy_shader(g_context.device, unlit_fragment_shader);

    AU_TRY_DISCARD(ghi::create_descriptor_tables(g_context.device, false,
                                                 g_context.unlit_pipeline_global_data_binding_layout,
                                                 {&g_context.unlit_pipeline_global_data_descriptor_table}));
    AU_TRY_DISCARD(ghi::create_descriptor_tables(g_context.device, false, g_context.texture_data_binding_layout,
                                                 {&g_context.texture_data_descriptor_table}));
    AU_TRY_DISCARD(ghi::create_descriptor_tables(g_context.device, true,
                                                 g_context.unlit_pipeline_per_frame_data_binding_layout,
                                                 {&g_context.unlit_pipeline_per_frame_binding_descriptor_table}));

    ghi::DescriptorUpdate unlit_pipeline_descriptor_updates[3] = {
        {
            .table = g_context.unlit_pipeline_global_data_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_global,
            .buffer_offset = 0,
            .buffer_range = 0,
        },
        {
            .table = g_context.unlit_pipeline_global_data_descriptor_table,
            .binding = 1,
            .array_element = 0,

            .buffer = g_context.material_storage_buffer,
            .buffer_offset = 0,
            .buffer_range = 0,
        },
        {
            .table = g_context.unlit_pipeline_per_frame_binding_descriptor_table,
            .binding = 0,
            .array_element = 0,

            .buffer = g_context.ubo_unlit_per_frame,
            .buffer_offset = 0,
            .buffer_range = 0,
        },
    };
    ghi::update_descriptor_tables(g_context.device, unlit_pipeline_descriptor_updates);

    return {};
  }
} // namespace iavis