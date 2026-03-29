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

#include <iavis/definitions.hpp>

namespace iavis
{
  auto initialize(const InitInfo &init_info) -> Result<void>;
  auto terminate() -> void;

  auto get_surface_width() -> i32;
  auto get_surface_height() -> i32;
  auto get_surface_handle() -> void *;

  auto create_geometry_unlit_2d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices) -> Result<GeomId>;
  auto create_geometry_unlit_3d(Span<const f32> positions, Span<const f32> tex_coords, Span<const u32> indices) -> Result<GeomId>;
  // [IATODO] auto create_geometry_lit_2d(Span<const f32> positions, Span<const f32> tex_coords, Span<const f32> normals, Span<const f32> tangents, Span<const u32> indices) -> Result<GeomId>;
  // [IATODO] auto create_geometry_lit_3d(Span<const f32> positions, Span<const f32> tex_coords, Span<const f32> normals, Span<const f32> tangents, Span<const u32> indices) -> Result<GeomId>;
  auto destroy_geometry(GeomId id) -> void;

  auto create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps = true) -> Result<TexId>;
  auto destroy_texture(TexId id) -> void;

  auto create_material(TexId albedo_tex, TexId normal_tex = INVALID_ID, TexId height_tex = INVALID_ID,
                       TexId roughness_tex = INVALID_ID, TexId ao_tex = INVALID_ID) -> Result<MatId>;
  auto destroy_material(MatId id) -> void;

  auto cmd_set_camera_matrix(CmdBufferId cmd, const Mat4 &camera_matrix) -> void;
  auto cmd_set_projection_matrix(CmdBufferId cmd, const Mat4 &projection_matrix) -> void;
  auto cmd_set_scissor(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void;
  auto cmd_set_viewport(CmdBufferId cmd, u32 x, u32 y, u32 width, u32 height) -> void;
  auto cmd_set_material(CmdBufferId cmd, MatId id) -> void;
  auto cmd_draw_geometry(CmdBufferId cmd, GeomId id, const Mat4 &model_matrix) -> void;
  // [IATODO] auto cmd_set_light_data() -> void;

  auto begin_frame() -> void;
  auto end_frame() -> void;

  auto begin_command_buffer() -> CmdBufferId;
  auto end_command_buffer(CmdBufferId id) -> void;
  auto submit_command_buffer(CmdBufferId id) -> void;
  auto submit_command_buffer_sync(CmdBufferId id) -> void;

  auto set_clear_color(const Vec4 &color) -> void;
} // namespace iavis

#if !defined(IAVIS_DONT_ALIAS_TO_VIS)
namespace vis = iavis;
#endif
