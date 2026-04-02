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

#include <backends/vulkan/backend.hpp>

namespace iavis
{
  IAVis_Backend_Vulkan vulkan_backend;

  auto initialize(const InitInfo &init_info) -> Result<void>
  {
    return vulkan_backend.initialize(init_info);
  }

  auto shutdown() -> void
  {
    vulkan_backend.shutdown();
  }

  auto get_surface_width() -> i32
  {
    return vulkan_backend.get_surface_width();
  }

  auto get_surface_height() -> i32
  {
    return vulkan_backend.get_surface_height();
  }

  auto get_surface_handle() -> void *
  {
    return vulkan_backend.get_surface_handle();
  }

  auto create_geometry_unlit_2d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices)
      -> Result<GeomId>
  {
    return vulkan_backend.create_geometry_unlit_2d(positions, tex_coords, indices);
  }

  auto create_geometry_unlit_3d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices)
      -> Result<GeomId>
  {
    return vulkan_backend.create_geometry_unlit_3d(positions, tex_coords, indices);
  }

  auto destroy_geometry(GeomId id) -> void
  {
    vulkan_backend.destroy_geometry(id);
  }

  auto create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps) -> Result<TexId>
  {
    return vulkan_backend.create_texture(rgba_data, width, height, generate_mipmaps);
  }

  auto destroy_texture(TexId id) -> void
  {
    vulkan_backend.destroy_texture(id);
  }

  auto create_material(TexId albedo_tex, TexId normal_tex, TexId height_tex, TexId roughness_tex, TexId ao_tex)
      -> Result<MatId>
  {
    return vulkan_backend.create_material(albedo_tex, normal_tex, height_tex, roughness_tex, ao_tex);
  }

  auto destroy_material(MatId id) -> void
  {
    vulkan_backend.destroy_material(id);
  }

  auto cmd_set_camera_matrix(CmdBufferId cmd, const f32 *camera_matrix) -> void
  {
    vulkan_backend.cmd_set_camera_matrix(cmd, camera_matrix);
  }

  auto cmd_set_projection_matrix(CmdBufferId cmd, const f32 *projection_matrix) -> void
  {
    vulkan_backend.cmd_set_projection_matrix(cmd, projection_matrix);
  }

  auto cmd_set_scissor(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void
  {
    vulkan_backend.cmd_set_scissor(cmd, x, y, width, height);
  }

  auto cmd_set_viewport(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void
  {
    vulkan_backend.cmd_set_viewport(cmd, x, y, width, height);
  }

  auto cmd_set_material(CmdBufferId cmd, MatId id) -> void
  {
    vulkan_backend.cmd_set_material(cmd, id);
  }

  auto cmd_draw_geometry(CmdBufferId cmd, GeomId id, const f32 *model_matrix) -> void
  {
    vulkan_backend.cmd_draw_geometry(cmd, id, model_matrix);
  }

  auto begin_frame() -> void
  {
    vulkan_backend.begin_frame();
  }

  auto end_frame() -> void
  {
    vulkan_backend.end_frame();
  }

  auto set_clear_color(f32 r, f32 g, f32 b) -> void
  {
    vulkan_backend.set_clear_color(r, g, b);
  }
} // namespace iavis

