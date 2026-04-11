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
  auto shutdown() -> void;

  auto resize(u32 width, u32 height) -> Result<void>;
  auto get_surface_width() -> u32;
  auto get_surface_height() -> u32;

  auto set_camera(const Camera& camera) -> void;

  auto set_clear_color(f32 r, f32 g, f32 b) -> void;
  // [IATODO] auto set_light_data() -> void;

  auto add_drawable(GeomId geometry, MatId material, glm::vec3 position, glm::quat rotation = glm::quat(), glm::vec3 scale = glm::vec3(1.0f), glm::vec2 tex_coords = glm::vec2(0.0f)) -> DrawableId;

  auto set_drawable_position(DrawableId id, glm::vec3 position) -> void;
  auto set_drawable_rotation(DrawableId id, glm::quat rotation) -> void;
  auto set_drawable_scale(DrawableId id, glm::vec3 scale) -> void;
  auto set_drawable_tex_coords(DrawableId id, glm::vec2 tex_coords) -> void;

  auto render() -> void;

  auto create_geometry_unlit_2d(Span<const VertexUnlit2DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>;
  auto create_geometry_unlit_3d(Span<const VertexUnlit3DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>;
  // [IATODO] auto create_geometry_lit_2d(Span<const VertexLit2DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>;
  // [IATODO] auto create_geometry_lit_3d(Span<const VertexLit3DGeometry> vertices, Span<const u32> indices) -> Result<GeomId>;
  auto destroy_geometry(GeomId id) -> void;

  auto create_texture(const u8 *rgba_data, u32 width, u32 height, bool generate_mipmaps = true) -> Result<TexId>;
  auto create_texture_from_file(const char* filename, bool generate_mipmaps = true) -> Result<TexId>;
  auto destroy_texture(TexId id) -> void;

  auto create_material(TexId albedo_tex, TexId normal_tex = INVALID_ID, TexId height_tex = INVALID_ID,
                       TexId roughness_tex = INVALID_ID, TexId ao_tex = INVALID_ID) -> Result<MatId>;
  auto destroy_material(MatId id) -> void;
} // namespace iavis

#if !defined(IAVIS_DONT_ALIAS_TO_VIS)
namespace vis = iavis;
#endif
