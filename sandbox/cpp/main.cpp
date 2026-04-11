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

#include "glm/ext/matrix_clip_space.hpp"

#include <auxid/containers/vec.hpp>

#include <iavis/iavis.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace iavis
{
  auto main() -> Result<void>
  {
    auto &logger = auxid::get_thread_logger();

    iavis::InitInfo init_info{
        .app_name = "IAVis Sandbox",
        .validation_enabled = true,
        .surface_width = 800,
        .surface_height = 600,
        .surface_creation_callback = [](void *instance_handle, void *user_data) -> void * {
          VkSurfaceKHR surface;
          const auto window = static_cast<SDL_Window *>(user_data);
          SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance_handle), nullptr, &surface);
          return surface;
        },
        .surface_creation_callback_user_data = nullptr,
    };

    SDL_Window *window{};

    if (!SDL_Init(SDL_INIT_VIDEO))
      return fail("failed to initialize SDL '%s'", SDL_GetError());

    window = SDL_CreateWindow(init_info.app_name, init_info.surface_width, init_info.surface_height,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (!window)
      return fail("failed to create SDL window '%s'", SDL_GetError());

    init_info.surface_creation_callback_user_data = window;

    AU_TRY_DISCARD(iavis::initialize(init_info));

    iavis::set_clear_color(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f);

    iavis::set_camera(Camera{
      .forward = {0.0f, 0.0f, 1.0f},
      .position = {0.0f, 0.0f, 10.0f},
        .projection = EProjection::ORTHOGRAPHIC,
    });

    Vec<VertexUnlit2DGeometry> vertices = {
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f}, {0.0f, 1.0f}},
    };

    const auto quad_geom = AU_TRY(iavis::create_geometry_unlit_2d(vertices, {2, 1, 0, 3, 2, 0}));

    const auto texture = AU_TRY(iavis::create_texture_from_file("sandbox/res/negx.jpg"));
    const auto material = AU_TRY(iavis::create_material(texture));

    const auto quad =
        iavis::add_drawable(quad_geom, material, glm::vec3(100.0f, 100.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f));

    SDL_ShowWindow(window);

    logger.info("successfully initialized the engine");

    bool running = true;
    f32 delta_time = 0.0f;
    f32 last_frame = 0.0f;
    while (running)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
        if ((event.type == SDL_EVENT_QUIT) ||
            ((event.type == SDL_EVENT_KEY_DOWN) && (event.key.scancode == SDL_SCANCODE_ESCAPE)))
          running = false;
      }

      const auto current_frame = static_cast<f32>(SDL_GetTicks()) / 1000.0f;
      delta_time = current_frame - last_frame;
      last_frame = current_frame;

      iavis::render();
    }

    iavis::shutdown();

    logger.info("cleanly exited the engine");

    SDL_DestroyWindow(window);
    SDL_Quit();

    return {};
  }
} // namespace iavis

int main(int argc, char *argv[])
{
  au::auxid::MainThreadGuard _thread_guard;

  if (const auto res = iavis::main(); !res)
  {
    au::auxid::get_thread_logger().error("%s", res.error().c_str());
    return -1;
  }

  return 0;
}