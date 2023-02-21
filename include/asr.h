#ifndef ASR_H
#define ASR_H

#include <GL/glew.h>
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

namespace asr
{
    /*
     * Common Constants
     */

    static constexpr float pi{static_cast<float>(M_PI)};
    static constexpr float two_pi{2.0f * static_cast<float>(M_PI)};
    static constexpr float half_pi{0.5f * static_cast<float>(M_PI)};
    static constexpr float quarter_pi{0.25f * static_cast<float>(M_PI)};

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

    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    typedef std::vector<asr::Vertex> Vertices;
    typedef std::vector<unsigned int> Indices;
    typedef std::pair<asr::Vertices, asr::Indices> GeometryPair;

    struct Geometry
    {
        GeometryType type{Triangles};
        unsigned int vertex_count{0};

        GLuint vertex_array_object{0};
        GLuint vertex_buffer_object{0};
        GLuint index_buffer_object{0};
    };

    /*
     * Transformation Types
     */

    enum MatrixMode
    {
        Model,
        View,
        Projection
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

        bool window_should_close{false};
        std::function<void(int)> key_down_event_handler;
        std::function<void(const uint8_t *)> keys_down_event_handler;

        /*
         * Shader Data
         */

        static GLuint shader_program{0};

        static GLint position_attribute_location{-1};
        static GLint color_attribute_location{-1};

        static GLint mvp_matrix_uniform_location{-1};
        static GLint time_uniform_location{-1};

        /*
         * Geometry Data
         */

        static Geometry *current_geometry{nullptr};

        /*
         * Transformation Data
         */

        static std::stack<glm::mat4> model_matrix_stack;
        static std::stack<glm::mat4> view_matrix_stack;
        static std::stack<glm::mat4> projection_matrix_stack;

        static std::stack<glm::mat4> *current_matrix_stack = &model_matrix_stack;

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

    static void create_window(unsigned int width, unsigned int height, const std::string &name = "ASR: Version 1.1")
    {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

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

        if (SDL_GL_SetSwapInterval(-1) < 0) {
            SDL_GL_SetSwapInterval(1);
        }

        data::key_down_event_handler = [&](int key) { };
        data::keys_down_event_handler = [&](const uint8_t *keys) { };
    }

    static void create_window(const std::string &name = "ASR: Version 1.1")
    {
        create_window(data::fullscreen, data::fullscreen, name);
    }

    static void set_key_down_event_handler(std::function<void(int)> event_handler)
    {
        data::key_down_event_handler = std::move(event_handler);
    }

    static void set_keys_down_event_handler(std::function<void(const uint8_t *)> event_handler)
    {
        data::keys_down_event_handler = std::move(event_handler);
    }

    static void process_window_events(bool *should_stop, bool ignore_esc_key = false)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || data::window_should_close) {
                *should_stop = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (!ignore_esc_key && event.key.keysym.sym == SDLK_ESCAPE) {
                    *should_stop = true;
                }
                data::key_down_event_handler(event.key.keysym.sym);
            }
            data::keys_down_event_handler(SDL_GetKeyboardState(nullptr));
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
        data::mvp_matrix_uniform_location = glGetUniformLocation(shader_program, "model_view_projection_matrix");

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
        data::mvp_matrix_uniform_location = -1;
    }

    /*
     * Geometry Handling
     */

    static Geometry create_geometry(
        GeometryType type,
        Vertices vertices,
        Indices indices
    )
    {
        Geometry geometry{
            type, static_cast<unsigned int>(indices.size())
        };

#ifdef __APPLE__
        glGenVertexArraysAPPLE(1, &geometry.vertex_array_object);
        glBindVertexArrayAPPLE(geometry.vertex_array_object);
#else
        glGenVertexArrays(1, &geometry.vertex_array_object);
        glBindVertexArray(geometry.vertex_array_object);
#endif

        glGenBuffers(1, &geometry.vertex_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, geometry.vertex_buffer_object);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
            reinterpret_cast<const float *>(vertices.data()),
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &geometry.index_buffer_object);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.index_buffer_object);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(decltype(indices)::value_type)),
            reinterpret_cast<const unsigned int *>(indices.data()),
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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return geometry;
    }

    static void set_geometry_current(Geometry *geometry)
    {
        data::current_geometry = geometry;
        if (geometry != nullptr) {
#ifdef __APPLE__
            glBindVertexArrayAPPLE(geometry->vertex_array_object);
#else
            glBindVertexArray(geometry->vertex_array_object);
#endif
        } else {
#ifdef __APPLE__
            glBindVertexArrayAPPLE(0);
#else
            glBindVertexArray(0);
#endif
        }
    }

    static void destroy_geometry(Geometry &geometry)
    {
#ifdef __APPLE__
        GLint current_vertex_array_object;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING_APPLE, &current_vertex_array_object);
        if (geometry.vertex_array_object == static_cast<GLint>(current_vertex_array_object)) {
            glBindVertexArrayAPPLE(0);
        }

        glDeleteVertexArraysAPPLE(1, &geometry.vertex_array_object);
#else
        GLint current_vertex_array_object;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vertex_array_object);
        if (geometry.vertex_array_object == static_cast<GLint>(current_vertex_array_object)) {
            glBindVertexArray(0);
        }

        glDeleteVertexArrays(1, &geometry.vertex_array_object);
#endif
        geometry.vertex_array_object = 0;

        GLint current_vertex_buffer_object;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vertex_buffer_object);
        if (geometry.vertex_buffer_object == static_cast<GLint>(current_vertex_buffer_object)) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glDeleteBuffers(1, &geometry.vertex_buffer_object);
        geometry.vertex_buffer_object = 0;

        GLint current_index_buffer_object;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &current_index_buffer_object);
        if (geometry.index_buffer_object == static_cast<GLint>(current_index_buffer_object)) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        glDeleteBuffers(1, &geometry.index_buffer_object);
        geometry.index_buffer_object = 0;
    }

    /*
     * Transformation
     */

    static void set_matrix_mode(MatrixMode mode)
    {
        switch (mode) {
            case Model:
                data::current_matrix_stack = &data::model_matrix_stack;
                break;
            case View:
                data::current_matrix_stack = &data::view_matrix_stack;
                break;
            case Projection:
                data::current_matrix_stack = &data::projection_matrix_stack;
                break;
        }
    }

    static void translate_matrix(glm::vec3 translation)
    {
        glm::mat4 current_matrix = data::current_matrix_stack->top();
        glm::mat4 translated_matrix = glm::translate(current_matrix, translation);

        data::current_matrix_stack->pop();
        data::current_matrix_stack->push(translated_matrix);
    }

    static void rotate_matrix(glm::vec3 rotation)
    {
        glm::mat4 current_matrix = data::current_matrix_stack->top();
        glm::mat4 rotated_matrix = current_matrix;
        rotated_matrix = glm::rotate(rotated_matrix, rotation.y, glm::vec3{0.0f, 1.0f, 0.0f});
        rotated_matrix = glm::rotate(rotated_matrix, rotation.x, glm::vec3{1.0f, 0.0f, 0.0f});
        rotated_matrix = glm::rotate(rotated_matrix, rotation.z, glm::vec3{0.0f, 0.0f, 1.0f});

        data::current_matrix_stack->pop();
        data::current_matrix_stack->push(rotated_matrix);
    }

    static void scale_matrix(glm::vec3 scale)
    {
        glm::mat4 current_matrix = data::current_matrix_stack->top();
        glm::mat4 scaled_matrix = glm::scale(current_matrix, scale);

        data::current_matrix_stack->pop();
        data::current_matrix_stack->push(scaled_matrix);
    }

    static inline glm::mat4 get_matrix()
    {
        return data::current_matrix_stack->top();
    }

    static inline glm::mat4 get_model_matrix()
    {
        return data::model_matrix_stack.top();
    }

    static inline glm::mat4 get_view_matrix()
    {
        return data::view_matrix_stack.top();
    }

    static inline glm::mat4 get_projection_matrix()
    {
        return data::projection_matrix_stack.top();
    }

    static void set_matrix(glm::mat4 matrix)
    {
        data::current_matrix_stack->pop();
        data::current_matrix_stack->push(matrix);
    }

    static void load_identity_matrix()
    {
        set_matrix(glm::mat4{1.0f});
    }

    static void load_look_at_matrix(glm::vec3 position, glm::vec3 target)
    {
        glm::vec3 up{0.0f, 1.0f, 0.0f};
        set_matrix(glm::lookAt(position, target, up));
    }

    static void load_orthographic_projection_matrix(float zoom, float near_plane, float far_plane)
    {
        float aspect_ratio{static_cast<float>(data::window_width) / static_cast<float>(data::window_height)};

        float left{-(zoom * aspect_ratio)};
        float right{zoom * aspect_ratio};
        float bottom{-zoom};
        float top{zoom};

        set_matrix(glm::ortho(left, right, bottom, top, near_plane, far_plane));
    }

    static void load_perspective_projection_matrix(float field_of_view, float near_plane, float far_plane)
    {
        float aspect_ratio{static_cast<float>(data::window_width) / static_cast<float>(data::window_height)};

        set_matrix(glm::perspective(field_of_view, aspect_ratio, near_plane, far_plane));
    }

    static void push_matrix()
    {
        glm::mat4 top_matrix = data::current_matrix_stack->top();
        data::current_matrix_stack->push(top_matrix);
    }

    static void pop_matrix()
    {
        data::current_matrix_stack->pop();
        if (data::current_matrix_stack->empty()) {
            data::current_matrix_stack->push(glm::mat4{1.0f});
        }
    }

    static void clear_matrices()
    {
        while (!data::current_matrix_stack->empty()) data::current_matrix_stack->pop();
        data::current_matrix_stack->push(glm::mat4{1.0f});
    }

    /*
     * Rendering
     */

    static void set_line_width(float line_width)
    {
        glLineWidth(static_cast<GLfloat>(line_width));
    }

    static void enable_face_culling()
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }

    static void disable_face_culling()
    {
        glDisable(GL_CULL_FACE);
    }

    static void enable_depth_test()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    static void disable_depth_test()
    {
        glDisable(GL_DEPTH_TEST);
    }

    static void prepare_for_rendering()
    {
        glClearColor(0, 0, 0, 0);
        glViewport(0, 0, static_cast<GLsizei>(data::window_width), static_cast<GLsizei>(data::window_height));
        glEnable(GL_PROGRAM_POINT_SIZE);

        while (!data::model_matrix_stack.empty()) data::model_matrix_stack.pop();
        data::model_matrix_stack.push(glm::mat4{1.0f});

        while (!data::view_matrix_stack.empty()) data::view_matrix_stack.pop();
        data::view_matrix_stack.push(glm::mat4{1.0f});

        while (!data::projection_matrix_stack.empty()) data::projection_matrix_stack.pop();
        data::projection_matrix_stack.push(glm::mat4{1.0f});

        data::rendering_start_time = std::chrono::system_clock::now();
    }

    static void prepare_to_render_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static void render_current_geometry()
    {
        assert(data::current_geometry);

        glUseProgram(data::shader_program);

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

        if (data::mvp_matrix_uniform_location != -1) {
            glm::mat4 model_matrix{data::model_matrix_stack.top()};
            glm::mat4 view_matrix{glm::inverse(data::view_matrix_stack.top())};
            glm::mat4 projection_matrix{data::projection_matrix_stack.top()};
            glm::mat4 model_view_projection_matrix{projection_matrix * view_matrix * model_matrix};
            glUniformMatrix4fv(
                data::mvp_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_view_projection_matrix)
            );
        }

        glDrawElements(
            utilities::convert_geometry_type_to_es2_geometry_type(data::current_geometry->type),
            static_cast<GLsizei>(data::current_geometry->vertex_count),
            GL_UNSIGNED_INT,
            nullptr
        );
    }

    static void finish_frame_rendering()
    {
        SDL_GL_SwapWindow(data::window);
    }
}

#endif
