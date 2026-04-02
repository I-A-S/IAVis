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

#include <base.hpp>

namespace iavis
{
  template<typename BackendT>
  concept IAVIS_BACKEND = requires(BackendT backend) {
    { backend.initialize(std::declval<const InitInfo&>()) } -> std::same_as<Result<void>>;
    { backend.shutdown() } -> std::same_as<void>;

    { backend.begin_frame() } -> std::same_as<void>;
    { backend.end_frame() } -> std::same_as<void>;

    { backend.get_surface_width() } -> std::same_as<i32>;
    { backend.get_surface_height() } -> std::same_as<i32>;
    { backend.get_surface_handle() } -> std::same_as<void *>;

    {
      backend.create_geometry_unlit_2d(std::declval<Span<const f32>>(), std::declval<Span<const f32>>(),
                                       std::declval<Span<const u32>>())
    } -> std::same_as<Result<GeomId>>;
    {
      backend.create_geometry_unlit_3d(std::declval<Span<const f32>>(), std::declval<Span<const f32>>(),
                                       std::declval<Span<const u32>>())
    } -> std::same_as<Result<GeomId>>;
    { backend.destroy_geometry(std::declval<GeomId>()) } -> std::same_as<void>;

    {
      backend.create_texture(std::declval<const u8 *>(), std::declval<u32>(), std::declval<u32>(), std::declval<bool>())
    } -> std::same_as<Result<TexId>>;
    { backend.destroy_texture(std::declval<TexId>()) } -> std::same_as<void>;

    {
      backend.create_material(std::declval<TexId>(), std::declval<TexId>(), std::declval<TexId>(),
                              std::declval<TexId>(), std::declval<TexId>())
    } -> std::same_as<Result<MatId>>;
    { backend.destroy_material(std::declval<MatId>()) } -> std::same_as<void>;

    { backend.cmd_set_camera_matrix(std::declval<CmdBufferId>(), std::declval<const f32 *>()) } -> std::same_as<void>;
    {
      backend.cmd_set_projection_matrix(std::declval<CmdBufferId>(), std::declval<const f32 *>())
    } -> std::same_as<void>;
    {
      backend.cmd_set_scissor(std::declval<CmdBufferId>(), std::declval<u32>(), std::declval<u32>(),
                              std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;
    {
      backend.cmd_set_viewport(std::declval<CmdBufferId>(), std::declval<u32>(), std::declval<u32>(),
                               std::declval<u32>(), std::declval<u32>())
    } -> std::same_as<void>;
    { backend.cmd_set_material(std::declval<CmdBufferId>(), std::declval<MatId>()) } -> std::same_as<void>;
    {
      backend.cmd_draw_geometry(std::declval<CmdBufferId>(), std::declval<GeomId>(), std::declval<const f32 *>())
    } -> std::same_as<void>;

    //{ backend.begin_command_buffer() } -> std::same_as<CmdBufferId>;
    //{ backend.end_command_buffer(std::declval<CmdBufferId>()) } -> std::same_as<void>;
    //{ backend.submit_command_buffer(std::declval<CmdBufferId>()) } -> std::same_as<void>;
    //{ backend.submit_command_buffer_sync(std::declval<CmdBufferId>()) } -> std::same_as<void>;

    { backend.set_clear_color(std::declval<f32>(), std::declval<f32>(), std::declval<f32>()) } -> std::same_as<void>;
  };
} // namespace iavis