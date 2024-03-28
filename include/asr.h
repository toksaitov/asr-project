#ifndef ASR_H
#define ASR_H

#include <GL/glew.h>
#include <SDL.h>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
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

    struct Vertex
    {
        float x, y, z;
        float r{1.0f}, g{1.0f}, b{1.0f}, a{1.0f};
        float u, v;
    };

    typedef std::vector<asr::Vertex> Vertices;
    typedef std::vector<unsigned int> Indices;
    typedef std::pair<asr::Vertices, asr::Indices> GeometryPair;

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

    struct Geometry
    {
        GeometryType type{Triangles};
        unsigned int vertex_count{0U};

        GLuint vertex_array_object{0U};
        GLuint vertex_buffer_object{0U};
        GLuint index_buffer_object{0U};
    };

    /*
     * Texture Types
     */

    struct Image
    {
        std::vector<uint8_t> pixel_data;
        unsigned int width;
        unsigned int height;
        unsigned int channels;
    };

    enum TexturingMode
    {
        Addition,
        Subtraction,
        ReverseSubtraction,
        Modulation,
        Decaling
    };

    enum TextureWrapMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge
    };

    enum TextureFilterType
    {
        Nearest,
        Linear,
        NearestMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapNearest,
        LinearMipmapLinear
    };

    struct Texture
    {
        unsigned int width{};
        unsigned int height{};
        unsigned int channels{};

        TexturingMode mode{Modulation};
        TextureWrapMode wrap_mode_u{ClampToEdge};
        TextureWrapMode wrap_mode_v{ClampToEdge};
        TextureFilterType minification_filter{Linear};
        TextureFilterType magnification_filter{Linear};
        float anisotropy{0.0f};

        GLuint texture_object{0U};
    };

    /*
     * Transformation Types
     */

    enum MatrixMode
    {
        Model,
        View,
        Projection,
        Texturing
    };

    namespace data
    {
        /*
         * Window Data
         */

        static unsigned int window_width{500U};
        static unsigned int window_height{500U};
        static const unsigned int fullscreen{std::numeric_limits<unsigned int>::max()};

        static SDL_Window *window{nullptr};
        static SDL_GLContext gl_context;

        static uint32_t mouse_state{0U};
        static int mouse_x{0}, mouse_y{0};

        static bool window_should_close{false};
        static std::function<void(int)> key_down_event_handler;
        static std::function<void(const uint8_t *)> keys_down_event_handler;

        /*
         * Shader Data
         */

        static GLuint shader_program{0U};

        static GLint position_attribute_location{-1};
        static GLint color_attribute_location{-1};
        static GLint texture_coordinates_attribute_location{-1};

        static GLint resolution_uniform_location{-1};
        static GLint mouse_uniform_location{-1};

        static GLint time_uniform_location{-1};
        static GLint dt_uniform_location{-1};

        static GLint texture_sampler_uniform_location{-1};
        static GLint texture_enabled_uniform_location{-1};
        static GLint texturing_mode_uniform_location{-1};
        static GLint texture_transformation_matrix_uniform_location{-1};

        static GLint model_matrix_uniform_location{-1};
        static GLint view_matrix_uniform_location{-1};
        static GLint model_view_matrix_uniform_location{-1};
        static GLint projection_matrix_uniform_location{-1};
        static GLint view_projection_matrix_uniform_location{-1};
        static GLint mvp_matrix_uniform_location{-1};

        /*
         * Geometry Data
         */

        static Geometry *current_geometry{nullptr};

        /*
         * Texture Data
         */

        static Texture *current_texture{nullptr};

        /*
         * Transformation Data
         */

        static std::stack<glm::mat4> model_matrix_stack;
        static std::stack<glm::mat4> view_matrix_stack;
        static std::stack<glm::mat4> projection_matrix_stack;
        static std::stack<glm::mat4> texture_matrix_stack;

        static std::stack<glm::mat4> *current_matrix_stack = &model_matrix_stack;

        /*
         * Utility Data
         */

        static std::chrono::system_clock::time_point rendering_start_time; // NOLINT(cert-err58-cpp)
        static std::chrono::system_clock::time_point frame_rendering_start_time; // NOLINT(cert-err58-cpp)
        static float frame_rendering_delta_time{0.016f};
        static float time_scale{1.0f};
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

        /*
        * Texture Handling
        */

        static GLint convert_wrap_mode_to_es2_texture_wrap_mode(TextureWrapMode wrap_mode)
        {
            switch (wrap_mode) {
                case Repeat:
                    return GL_REPEAT;
                case MirroredRepeat:
                    return GL_MIRRORED_REPEAT;
                case ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
            }

            return GL_REPEAT;
        }

        static GLint convert_filter_type_to_es2_texture_filter_type(TextureFilterType filter)
        {
            switch (filter) {
                case Nearest:
                    return GL_NEAREST;
                case Linear:
                    return GL_LINEAR;
                case NearestMipmapNearest:
                    return GL_NEAREST_MIPMAP_NEAREST;
                case NearestMipmapLinear:
                    return GL_NEAREST_MIPMAP_LINEAR;
                case LinearMipmapNearest:
                    return GL_LINEAR_MIPMAP_NEAREST;
                case LinearMipmapLinear:
                    return GL_LINEAR_MIPMAP_LINEAR;
            }

            return GL_NEAREST;
        }
    }

    /*
     * Window Handling
     */

    static void create_window(unsigned int width, unsigned int height, const std::string &name = "ASR: Version 1.2")
    {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

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

    static void create_window(const std::string &name = "ASR: Version 1.2")
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
     * Shader Handling
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

        data::position_attribute_location =
            glGetAttribLocation(shader_program, "position");
        data::color_attribute_location =
            glGetAttribLocation(shader_program, "color");
        data::texture_coordinates_attribute_location =
            glGetAttribLocation(shader_program, "texture_coordinates");

        data::resolution_uniform_location =
            glGetUniformLocation(shader_program, "resolution");
        data::mouse_uniform_location =
            glGetUniformLocation(shader_program, "mouse");

        data::time_uniform_location =
            glGetUniformLocation(shader_program, "time");
        data::dt_uniform_location =
            glGetUniformLocation(shader_program, "get_dt");

        data::texture_enabled_uniform_location =
            glGetUniformLocation(shader_program, "texture_enabled");
        data::texture_transformation_matrix_uniform_location =
            glGetUniformLocation(shader_program, "texture_transformation_matrix");
        data::texturing_mode_uniform_location =
            glGetUniformLocation(shader_program, "texturing_mode");
        data::texture_sampler_uniform_location =
            glGetUniformLocation(shader_program, "texture_sampler");

        data::model_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_matrix");
        data::view_matrix_uniform_location =
            glGetUniformLocation(shader_program, "view_matrix");
        data::model_view_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_view_matrix");
        data::projection_matrix_uniform_location =
            glGetUniformLocation(shader_program, "projection_matrix");
        data::view_projection_matrix_uniform_location =
            glGetUniformLocation(shader_program, "view_projection_matrix");
        data::mvp_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_view_projection_matrix");

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
        data::texture_coordinates_attribute_location = -1;

        data::resolution_uniform_location = -1;
        data::mouse_uniform_location = -1;

        data::time_uniform_location = -1;
        data::dt_uniform_location = -1;

        data::texture_sampler_uniform_location = -1;
        data::texture_enabled_uniform_location = -1;
        data::texturing_mode_uniform_location = -1;
        data::texture_transformation_matrix_uniform_location = -1;

        data::model_matrix_uniform_location = -1;
        data::view_matrix_uniform_location = -1;
        data::model_view_matrix_uniform_location = -1;
        data::projection_matrix_uniform_location = -1;
        data::view_projection_matrix_uniform_location = -1;
        data::mvp_matrix_uniform_location = -1;
    }

    /*
     * Geometry Handling
     */

    static Geometry create_geometry(GeometryType type, Vertices vertices, Indices indices)
    {
        Geometry geometry{type, static_cast<unsigned int>(indices.size())};

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
            static_cast<GLsizeiptr>(vertices.size() * 9 * sizeof(float)),
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

        GLsizei stride = sizeof(GLfloat) * 9;
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
        glEnableVertexAttribArray(data::texture_coordinates_attribute_location);
        glVertexAttribPointer(
            data::texture_coordinates_attribute_location,
            2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 7)
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
     * Texture Handling
     */

    static Texture create_texture(Image &image, bool generate_mipmaps = false)
    {
        Texture texture{image.width, image.height, image.channels};

        glGenTextures(1, &texture.texture_object);
        glBindTexture(GL_TEXTURE_2D, texture.texture_object);

        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            utilities::convert_wrap_mode_to_es2_texture_wrap_mode(texture.wrap_mode_u)
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
            utilities::convert_wrap_mode_to_es2_texture_wrap_mode(texture.wrap_mode_v)
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            utilities::convert_filter_type_to_es2_texture_filter_type(texture.magnification_filter)
        );
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            utilities::convert_filter_type_to_es2_texture_filter_type(texture.minification_filter)
        );
        glTexParameterf(
            GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
            static_cast<GLfloat>(texture.anisotropy)
        );

        GLint format = texture.channels == 3 ? GL_RGB : GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
            GL_TEXTURE_2D, 0, format,
            static_cast<GLsizei>(texture.width),
            static_cast<GLsizei>(texture.height),
            0, static_cast<GLenum>(format), GL_UNSIGNED_BYTE,
            reinterpret_cast<GLvoid *>(image.pixel_data.data())
        );

        if (generate_mipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        return texture;
    }

    static void set_texture_mode(TexturingMode mode)
    {
        assert(data::current_texture);

        data::current_texture->mode = mode;
    }

    static void set_texture_wrap_mode_u(TextureWrapMode wrap_mode_u)
    {
        assert(data::current_texture);

        data::current_texture->wrap_mode_u = wrap_mode_u;
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            utilities::convert_wrap_mode_to_es2_texture_wrap_mode(wrap_mode_u)
        );
    }

    static void set_texture_wrap_mode_v(TextureWrapMode wrap_mode_v)
    {
        assert(data::current_texture);

        data::current_texture->wrap_mode_v = wrap_mode_v;
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
            utilities::convert_wrap_mode_to_es2_texture_wrap_mode(wrap_mode_v)
        );
    }

    static void set_texture_magnification_filter(TextureFilterType magnification_filter)
    {
        assert(data::current_texture);

        data::current_texture->magnification_filter = magnification_filter;
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            utilities::convert_filter_type_to_es2_texture_filter_type(magnification_filter)
        );
    }

    static void set_texture_minification_filter(TextureFilterType minification_filter)
    {
        assert(data::current_texture);

        data::current_texture->minification_filter = minification_filter;
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            utilities::convert_filter_type_to_es2_texture_filter_type(minification_filter)
        );
    }

    static void set_texture_anisotropy(float anisotropy)
    {
        assert(data::current_texture);

        data::current_texture->anisotropy = anisotropy;
        glTexParameterf(
            GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
            static_cast<GLfloat>(anisotropy)
        );
    }

    static void set_texture_current(Texture *texture, unsigned int sampler = 0)
    {
        data::current_texture = texture;
        if (texture != nullptr) {
            glActiveTexture(GL_TEXTURE0 + sampler);
            glBindTexture(GL_TEXTURE_2D, texture->texture_object);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    static void destroy_texture(Texture &texture)
    {
        GLint current_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
        if (texture.texture_object == static_cast<GLuint>(current_texture)) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glDeleteTextures(1, &texture.texture_object);
        texture.texture_object = 0;
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
            case Texturing:
                data::current_matrix_stack = &data::texture_matrix_stack;
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

    static inline glm::mat4 get_texture_matrix()
    {
        return data::texture_matrix_stack.top();
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
     * Utility Functions
     */

    static std::string read_text_file(const std::string &path)
    {
        std::ifstream file_stream{path};
        if (!file_stream.is_open()) {
            std::cerr << "Failed to open the file: '" << path << "'" << std::endl;
            std::exit(-1);
        }

        std::stringstream string_stream;
        string_stream << file_stream.rdbuf();

        return string_stream.str();
    }

    static Image read_image_file(const std::string &path)
    {
        int image_width, image_height;
        int bytes_per_pixel;

        auto image_data = static_cast<uint8_t *>(stbi_load(path.c_str(), &image_width, &image_height, &bytes_per_pixel, 0));
        if (!image_data) {
            std::cerr << "Failed to open the file: '" << path << "'" << std::endl;
            std::exit(-1);
        }
        if (!(bytes_per_pixel == 3 || bytes_per_pixel == 4)) {
            std::cerr << "Invalid image file format (only RGB and RGBA files are supported): '" << path << "'"
                      << std::endl;
            std::exit(-1);
        }

        std::vector<uint8_t> result{image_data, image_data + image_height * image_width * bytes_per_pixel};
        stbi_image_free(image_data);

        return Image{
            result,
            static_cast<unsigned int>(image_width),
            static_cast<unsigned int>(image_height),
            static_cast<unsigned int>(bytes_per_pixel)
        };
    }

    static float get_time_scale()
    {
        return data::time_scale;
    }

    static void set_time_scale(float time_scale)
    {
        data::time_scale = time_scale;
    }

    static inline float get_dt()
    {
        return data::frame_rendering_delta_time * data::time_scale;
    }

    /*
     * Rendering
     */

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

        while (!data::texture_matrix_stack.empty()) data::texture_matrix_stack.pop();
        data::texture_matrix_stack.push(glm::mat4{1.0f});

        data::rendering_start_time = std::chrono::system_clock::now();
    }

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

    static void prepare_to_render_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        data::frame_rendering_start_time = std::chrono::system_clock::now();
    }

    static void render_current_geometry()
    {
        assert(data::current_geometry);

        glUseProgram(data::shader_program);

        if (data::resolution_uniform_location != -1) {
            glUniform2f(
                data::resolution_uniform_location,
                static_cast<GLfloat>(data::window_width),
                static_cast<GLfloat>(data::window_height)
            );
        }

        if (data::mouse_uniform_location != -1) {
            glUniform2f(
                data::mouse_uniform_location,
                static_cast<GLfloat>(data::mouse_x),
                static_cast<GLfloat>(data::mouse_y)
            );
        }

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

        if (data::dt_uniform_location != -1) {
            glUniform1f(
                data::dt_uniform_location,
                static_cast<GLfloat>(data::frame_rendering_delta_time)
            );
        }

        bool texture_enabled = data::current_texture != nullptr;
        if (data::texture_enabled_uniform_location != -1) {
            glUniform1i(
                data::texture_enabled_uniform_location,
                static_cast<GLint>(texture_enabled)
            );
        }

        if (data::texture_sampler_uniform_location != -1) {
            glUniform1i(data::texture_sampler_uniform_location, 0);
        }

        if (data::texture_transformation_matrix_uniform_location != -1) {
            glm::mat4 texture_matrix = data::texture_matrix_stack.top();
            glUniformMatrix4fv(
                data::texture_transformation_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(texture_matrix)
            );
        }

        if (data::texturing_mode_uniform_location != -1 && data::current_texture != nullptr) {
            glUniform1i(
                data::texturing_mode_uniform_location,
                static_cast<GLint>(data::current_texture->mode)
            );
        }

        if (data::model_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glUniformMatrix4fv(
                data::model_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_matrix)
            );
        }

        if (data::view_matrix_uniform_location != -1) {
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glUniformMatrix4fv(
                data::view_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(view_matrix)
            );
        }

        if (data::model_view_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 model_view_matrix = view_matrix * model_matrix;
            glUniformMatrix4fv(
                data::model_view_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_view_matrix)
            );
        }

        if (data::projection_matrix_uniform_location != -1) {
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glUniformMatrix4fv(
                data::projection_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(projection_matrix)
            );
        }

        if (data::view_projection_matrix_uniform_location != -1) {
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glm::mat4 view_projection_matrix = projection_matrix * view_matrix;
            glUniformMatrix4fv(
                data::view_projection_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(view_projection_matrix)
            );
        }

        if (data::mvp_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glm::mat4 model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;
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

        data::frame_rendering_delta_time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - data::frame_rendering_start_time
        ).count()) / 1000.0f;

        // If frame time is too large (e.g., the process is being debugged, set an artificial frame time for 60 fps.
        if (data::frame_rendering_delta_time > 1.0f) data::frame_rendering_delta_time = 0.016;
    }
}

#endif
