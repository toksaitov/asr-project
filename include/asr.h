#ifndef ASR_H
#define ASR_H

#include <GL/glew.h>
#include <SDL.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace asr
{
    /*
     * Geometry Types
     */

    enum GeometryType
    {
        Points,
        Lines,
        LineLoop,
        LineStrip,
        Triangles,
        TriangleFan,
        TriangleStrip
    };

    namespace data
    {
        /*
         * Window Data
         */

        static unsigned int window_width{500};
        static unsigned int window_height{500};
        static const unsigned int fullscreen{std::numeric_limits<unsigned int>::max()};

        static SDL_Window *window{nullptr};
        static SDL_GLContext gl_context;

        /*
         * Shader Data
         */

        static GLuint shader_program{0};
        static GLint position_attribute_location{-1};
        static GLint color_attribute_location{-1};
        static GLint time_uniform_location{-1};

        /*
         * Geometry Data
         */

        static GLuint vertex_array_object{0};
        static GLuint vertex_buffer_object{0};

        static GLenum geometry_type;
        static size_t geometry_vertex_count{0};

        /*
         * Utility Data
         */

        static std::chrono::system_clock::time_point rendering_start_time; // NOLINT(cert-err58-cpp)
    }

    namespace utilities
    {
        /*
         * Geometry Handling
         */

        static GLenum convert_geometry_type_to_es2_geometry_type(GeometryType type)
        {
            switch (type) {
                case GeometryType::Points:
                    return GL_POINTS;
                case GeometryType::Lines:
                    return GL_LINES;
                case GeometryType::LineLoop:
                    return GL_LINE_LOOP;
                case GeometryType::LineStrip:
                    return GL_LINE_STRIP;
                case GeometryType::Triangles:
                    return GL_TRIANGLES;
                case GeometryType::TriangleFan:
                    return GL_TRIANGLE_FAN;
                case GeometryType::TriangleStrip:
                    return GL_TRIANGLE_STRIP;
            }

            return GL_TRIANGLES;
        }
    }

    /*
     * Window Handling
     */

    static void create_window(unsigned int width, unsigned int height, const std::string &name = "ASR: Version 1.0")
    {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        data::window_width = width;
        data::window_height = height;

        unsigned int flags{SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI};
        if (data::window_width == data::fullscreen || data::window_height == data::fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        data::window =
            SDL_CreateWindow(
                name.c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                static_cast<int>(data::window_width),
                static_cast<int>(data::window_height),
                flags
            );
        SDL_GL_GetDrawableSize(
            data::window,
            reinterpret_cast<int *>(&data::window_width),
            reinterpret_cast<int *>(&data::window_height)
        );

        data::gl_context = SDL_GL_CreateContext(data::window);
        SDL_GL_MakeCurrent(data::window, data::gl_context);

        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize the OpenGL loader." << std::endl;
            std::exit(-1);
        }

        SDL_GL_SetSwapInterval(1);
    }

    static void process_window_events(bool *should_stop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                *should_stop = true;
            }
        }
    }

    static void destroy_window()
    {
        SDL_GL_DeleteContext(data::gl_context);

        SDL_DestroyWindow(data::window);
        data::window = nullptr;

        SDL_Quit();
    }

    /*
     * Shader Program Handling
     */

    static void create_shader(const std::string &vertex_shader, const std::string &fragment_shader)
    {
        GLint status;

        const char *vertex_shader_source = vertex_shader.c_str();
        const char *fragment_shader_source = fragment_shader.c_str();

        GLuint vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader_object, 1, static_cast<const GLchar **>(&vertex_shader_source), nullptr);
        glCompileShader(vertex_shader_object);
        glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetShaderiv(vertex_shader_object, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetShaderInfoLog(vertex_shader_object, info_log_length, nullptr, info_log);
                std::cerr << "Failed to compile a vertex shader" << std::endl
                          << "Compilation log:\n" << info_log << std::endl << std::endl;

                delete[] info_log;
            }

            std::exit(-1);
        }

        GLuint fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader_object, 1, static_cast<const GLchar **>(&fragment_shader_source), nullptr);
        glCompileShader(fragment_shader_object);
        glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetShaderInfoLog(fragment_shader_object, info_log_length, nullptr, info_log);
                std::cerr << "Failed to compile a fragment shader" << std::endl
                          << "Compilation log:\n" << info_log << std::endl;

                delete[] info_log;
            }

            std::exit(-1);
        }

        GLuint shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader_object);
        glAttachShader(shader_program, fragment_shader_object);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetProgramInfoLog(shader_program, info_log_length, nullptr, info_log);
                std::cerr << "Failed to link a shader program" << std::endl
                          << "Linker log:\n" << info_log << std::endl;

                delete[] info_log;
            }

            std::exit(-1);
        }

        glDetachShader(shader_program, vertex_shader_object);
        glDetachShader(shader_program, fragment_shader_object);
        glDeleteShader(vertex_shader_object);
        glDeleteShader(fragment_shader_object);

        data::position_attribute_location = glGetAttribLocation(shader_program, "position");
        data::color_attribute_location = glGetAttribLocation(shader_program, "color");

        data::time_uniform_location = glGetUniformLocation(shader_program, "time");

        data::shader_program = shader_program;
    }

    static void destroy_shader()
    {
        GLint current_shader_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current_shader_program);
        if (data::shader_program == current_shader_program) {
            glUseProgram(0);
        }

        glDeleteProgram(data::shader_program);
        data::shader_program = 0;

        data::position_attribute_location = -1;
        data::color_attribute_location = -1;

        data::time_uniform_location = -1;
    }

    /*
     * Geometry Handling
     */

    static void create_geometry(GeometryType type, const float *data, const size_t vertex_count)
    {
        data::geometry_vertex_count = vertex_count;
        data::geometry_type = utilities::convert_geometry_type_to_es2_geometry_type(type);

#ifdef __APPLE__
        glGenVertexArraysAPPLE(1, &data::vertex_array_object);
        glBindVertexArrayAPPLE(data::vertex_array_object);
#else
        glGenVertexArrays(1, &data::vertex_array_object);
        glBindVertexArray(data::vertex_array_object);
#endif

        glGenBuffers(1, &data::vertex_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, data::vertex_buffer_object);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(data::geometry_vertex_count * 7 * sizeof(float)), data,
            GL_STATIC_DRAW
        );

        GLsizei stride = sizeof(GLfloat) * 7;
        glEnableVertexAttribArray(data::position_attribute_location);
        glVertexAttribPointer(
            data::position_attribute_location,
            3, GL_FLOAT, GL_FALSE, stride, static_cast<const GLvoid *>(nullptr)
        );
        glEnableVertexAttribArray(data::color_attribute_location);
        glVertexAttribPointer(
            data::color_attribute_location,
            4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 3)
        );

#ifdef __APPLE__
        glBindVertexArrayAPPLE(0);
#else
        glBindVertexArray(0);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    static void destroy_geometry()
    {
#ifdef __APPLE__
        GLint current_vertex_array_object;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING_APPLE, &current_vertex_array_object);
        if (data::vertex_array_object == static_cast<GLint>(current_vertex_array_object)) {
            glBindVertexArrayAPPLE(0);
        }

        glDeleteVertexArraysAPPLE(1, &data::vertex_array_object);
#else
        GLint current_vertex_array_object;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vertex_array_object);
        if (data::vertex_array_object == static_cast<GLint>(current_vertex_array_object)) {
            glBindVertexArray(0);
        }

        glDeleteVertexArrays(1, &data::vertex_array_object);
#endif
        data::vertex_array_object = 0;

        GLint current_vertex_buffer_object;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vertex_buffer_object);
        if (data::vertex_buffer_object == static_cast<GLint>(current_vertex_buffer_object)) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glDeleteBuffers(1, &data::vertex_buffer_object);
        data::vertex_buffer_object = 0;
    }

    /*
     * Rendering
     */

    static void prepare_for_rendering()
    {
        glClearColor(0, 0, 0, 0);
        glViewport(0, 0, static_cast<GLsizei>(data::window_width), static_cast<GLsizei>(data::window_height));

        data::rendering_start_time = std::chrono::system_clock::now();
    }

    static void render_next_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(data::shader_program);
#ifdef __APPLE__
        glBindVertexArrayAPPLE(data::vertex_array_object);
#else
        glBindVertexArray(data::vertex_array_object);
#endif

        if (data::time_uniform_location != -1) {
            float time =
                static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - data::rendering_start_time
                ).count()) / 1000.0f;

            glUniform1f(
                data::time_uniform_location,
                static_cast<GLfloat>(time)
            );
        }

        glDrawArrays(data::geometry_type, 0, static_cast<GLsizei>(data::geometry_vertex_count));

        SDL_GL_SwapWindow(data::window);
    }
}

#endif
