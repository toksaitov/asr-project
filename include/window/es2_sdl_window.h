#ifndef ES2_SDL_WINDOW_H
#define ES2_SDL_WINDOW_H

#include "window/window.h"

#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <imgui.h>
#include <vendor/imgui_impl_sdl.h>
#include <vendor/imgui_impl_opengl.h>

#include <iostream>
#include <cstdlib>

namespace asr
{
    class ES2SDLWindow final : public Window
    {
    public:
        ES2SDLWindow(
            const std::string &name,
            unsigned int width = FULLSCREEN,
            unsigned int height = FULLSCREEN,
            bool msaa_enabled = false,
            int msaa_samples = 2
        ) : Window{name, width, height}
        {
            if (_instance != nullptr) {
                std::cerr << "Only one SDL window is supported at this moment." << std::endl;
                std::exit(-1);
            }
            _instance = this;

            SDL_SetMainReady();
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
                std::cerr << "Failed to create an SDL window." << std::endl;
                std::exit(-1);
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            if (msaa_enabled) {
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_samples);
            }

            unsigned int flags{SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI};
            if (width == FULLSCREEN || height == FULLSCREEN) {
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            _window = SDL_CreateWindow(
                name.c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                static_cast<int>(width),
                static_cast<int>(height),
                flags
            );
            SDL_GL_GetDrawableSize(_window, reinterpret_cast<int *>(&_width), reinterpret_cast<int *>(&_height));
            if (_relative_mouse_mode_enabled) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            if (_capture_mouse_enabled) {
                SDL_CaptureMouse(SDL_TRUE);
            } else {
                SDL_CaptureMouse(SDL_FALSE);
            }

            _gl_context = SDL_GL_CreateContext(_window);
            SDL_GL_MakeCurrent(_window, _gl_context);
            if (glewInit() != GLEW_OK) {
                std::cerr << "Failed to initialize the OpenGL loader." << std::endl;
                exit(-1);
            }

            if (SDL_GL_SetSwapInterval(-1) < 0) {
                SDL_GL_SetSwapInterval(1);
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            _io = &ImGui::GetIO();
            _io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            _io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            _io->IniFilename = nullptr;
            ImGui_ImplSDL2_InitForOpenGL(_window, _gl_context);
            ImGui_ImplOpenGL3_Init("#version 120");

            _on_exit = [&]() { exit(0); };
            _on_key_down = [&](int key) { if (key == SDLK_ESCAPE) { exit(0); }};
            _on_late_keys_down = [&](const uint8_t *keys) { };
            _on_mouse_move = [&](int x, int y, int x_rel, int y_rel) { };
            _on_mouse_down = [&](int button, int x, int y) { };
        }

        ES2SDLWindow(const ES2SDLWindow &other) = delete;
        ES2SDLWindow& operator=(const ES2SDLWindow &other) = delete;

        ~ES2SDLWindow() final
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();

            SDL_GL_DeleteContext(_gl_context);
            SDL_DestroyWindow(_window);
            SDL_Quit();
        }

        void poll() final
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    _on_exit();
                } else if (event.type == SDL_KEYDOWN) {
                    _on_key_down(event.key.keysym.sym);
                } else if (event.type == SDL_MOUSEMOTION) {
                    _on_mouse_move(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    _on_mouse_down(event.button.button, event.button.x, event.button.y);
                }
                _on_late_keys_down(SDL_GetKeyboardState(nullptr));

                ImGui_ImplSDL2_ProcessEvent(&event);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
        }

        void swap() final
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(_window);
        }

        [[nodiscard]] bool is_relative_mouse_mode_enabled() const
        {
            return _relative_mouse_mode_enabled;
        }

        void set_relative_mouse_mode_enabled(bool relative_mouse_mode_enabled)
        {
            _relative_mouse_mode_enabled = relative_mouse_mode_enabled;
            if (_relative_mouse_mode_enabled) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }

        [[nodiscard]] bool is_capture_mouse_enabled() const
        {
            return _capture_mouse_enabled;
        }

        void set_capture_mouse_enabled(bool capture_mouse_enabled)
        {
            _capture_mouse_enabled = capture_mouse_enabled;
            if (_capture_mouse_enabled) {
                SDL_CaptureMouse(SDL_TRUE);
            } else {
                SDL_CaptureMouse(SDL_FALSE);
            }
        }

    private:
        inline static ES2SDLWindow *_instance{nullptr};

        ImGuiIO *_io{nullptr};
        SDL_GLContext _gl_context;
        SDL_Window *_window{nullptr};

        bool _relative_mouse_mode_enabled{false};
        bool _capture_mouse_enabled{false};
    };
}

#endif
