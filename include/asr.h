#ifndef ASR_H
#define ASR_H

#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <any>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
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
        float nx, ny, nz;
        float r{1.0f}, g{1.0f}, b{1.0f}, a{1.0f};
        float u, v;
    };

    struct Instance
    {
        glm::mat4 transform;
        float r, g, b, a;
    };

    typedef std::vector<asr::Vertex> Vertices;
    typedef std::vector<unsigned int> Indices;
    typedef std::vector<Instance> Instances;
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
        GLuint instance_buffer_object{0U};
        GLuint index_buffer_object{0U};

        unsigned int instance_count{0U};
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
     * Material Types
     */

    enum MaterialDepthTestFunction {
        Never,
        Always,
        Less,
        LowerOrEqual,
        Equal,
        Greater,
        GreaterOrEqual,
        NotEqual
    };

    enum MaterialBlendingEquation
    {
        Add,
        Subtract,
        ReverseSubtract
    };

    enum MaterialBlendingFunction
    {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SourceAlphaSaturate
    };

    enum MaterialCullFaceMode
    {
        FrontFaces,
        BackFaces,
        FrontAndBackFaces
    };

    enum MaterialFrontFaceOrder
    {
        Clockwise,
        Counterclockwise
    };

    struct Material
    {
        std::string vertex_shader;
        std::string fragment_shader;

        float line_width{1.0f};

        bool point_sizing_enabled{true};
        float point_size{1.0f};

        bool face_culling_enabled{true};
        MaterialCullFaceMode cull_face_mode{BackFaces};
        MaterialFrontFaceOrder front_face_order{Counterclockwise};

        bool depth_mask_enabled{true};
        bool depth_test_enabled{false};
        MaterialDepthTestFunction depth_test_function{Less};

        bool blending_enabled{false};
        MaterialBlendingEquation color_blending_equation{Add};
        MaterialBlendingEquation alpha_blending_equation{Add};
        MaterialBlendingFunction source_color_blending_function{SourceAlpha};
        MaterialBlendingFunction source_alpha_blending_function{SourceAlpha};
        MaterialBlendingFunction destination_color_blending_function{OneMinusSourceAlpha};
        MaterialBlendingFunction destination_alpha_blending_function{OneMinusSourceAlpha};
        glm::vec4 blending_constant_color{0.0f};

        bool polygon_offset_enabled{false};
        float polygon_offset_factor{0.0f};
        float polygon_offset_units{0.0f};

        GLuint shader_program{0U};

        // Common Uniforms are Hard-Coded

        GLint position_attribute_location{-1};
        GLint normal_attribute_location{-1};
        GLint color_attribute_location{-1};
        GLint texture_coordinates_attribute_location{-1};

        GLint instance_transform_attribute_location{-1};
        GLint instance_color_attribute_location{-1};

        GLint resolution_uniform_location{-1};
        GLint mouse_uniform_location{-1};

        GLint time_uniform_location{-1};
        GLint dt_uniform_location{-1};

        GLint texture_sampler_uniform_location{-1};
        GLint texture_enabled_uniform_location{-1};
        GLint texturing_mode_uniform_location{-1};
        GLint texture_transformation_matrix_uniform_location{-1};

        GLint point_size_uniform_location{-1};

        GLint model_matrix_uniform_location{-1};
        GLint view_matrix_uniform_location{-1};
        GLint model_view_matrix_uniform_location{-1};
        GLint projection_matrix_uniform_location{-1};
        GLint view_projection_matrix_uniform_location{-1};
        GLint mvp_matrix_uniform_location{-1};
        GLint normal_matrix_uniform_location{-1};

        // All the other uniforms including the common

        std::map<std::string, GLint> shader_uniforms;
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
         * Geometry Data
         */

        static Geometry *current_geometry{nullptr};

        /*
         * Texture Data
         */

        static Texture *current_texture{nullptr};

        /*
         * Material Data
         */

        static Material *current_material{nullptr};

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

        /*
         * Material Handling
         */

        static GLenum convert_cull_face_mode_to_es2_cull_face_mode(MaterialCullFaceMode cull_face_mode)
        {
            switch (cull_face_mode) {
                case FrontFaces:
                    return GL_FRONT;
                case BackFaces:
                    return GL_BACK;
                case FrontAndBackFaces:
                    return GL_FRONT_AND_BACK;
            }

            return GL_BACK;
        }

        static GLenum convert_front_face_order_to_es2_front_face_order(MaterialFrontFaceOrder front_face_order)
        {
            switch (front_face_order) {
                case Clockwise:
                    return GL_CW;
                case Counterclockwise:
                    return GL_CCW;
            }

            return GL_CW;
        }

        static GLenum convert_depth_test_func_to_es2_depth_test_func(MaterialDepthTestFunction depth_test_function)
        {
            switch (depth_test_function) {
                case Never:
                    return GL_NEVER;
                case Always:
                    return GL_ALWAYS;
                case Less:
                    return GL_LESS;
                case LowerOrEqual:
                    return GL_LEQUAL;
                case Equal:
                    return GL_EQUAL;
                case Greater:
                    return GL_GREATER;
                case GreaterOrEqual:
                    return GL_GEQUAL;
                case NotEqual:
                    return GL_NOTEQUAL;
            }

            return GL_LESS;
        }

        static GLenum convert_blending_equation_to_es2_blending_equation(MaterialBlendingEquation blending_equation)
        {
            switch (blending_equation) {
                case Add:
                    return GL_FUNC_ADD;
                case Subtract:
                    return GL_FUNC_SUBTRACT;
                case ReverseSubtract:
                    return GL_FUNC_REVERSE_SUBTRACT;
            }

            return GL_FUNC_ADD;
        }

        static GLenum convert_blending_func_to_es2_blending_func(MaterialBlendingFunction blending_function)
        {
            switch (blending_function) {
                case Zero:
                    return GL_ZERO;
                case One:
                    return GL_ONE;
                case SourceColor:
                    return GL_SRC_COLOR;
                case OneMinusSourceColor:
                    return GL_ONE_MINUS_SRC_COLOR;
                case DestinationColor:
                    return GL_DST_COLOR;
                case OneMinusDestinationColor:
                    return GL_ONE_MINUS_DST_COLOR;
                case SourceAlpha:
                    return GL_SRC_ALPHA;
                case OneMinusSourceAlpha:
                    return GL_ONE_MINUS_SRC_ALPHA;
                case DestinationAlpha:
                    return GL_DST_ALPHA;
                case OneMinusDestinationAlpha:
                    return GL_ONE_MINUS_DST_ALPHA;
                case ConstantColor:
                    return GL_CONSTANT_COLOR;
                case OneMinusConstantColor:
                    return GL_ONE_MINUS_CONSTANT_COLOR;
                case ConstantAlpha:
                    return GL_CONSTANT_ALPHA;
                case OneMinusConstantAlpha:
                    return GL_ONE_MINUS_CONSTANT_ALPHA;
                case SourceAlphaSaturate:
                    return GL_SRC_ALPHA_SATURATE;
            }

            return GL_SRC_ALPHA;
        }
    }

    /*
     * Window Handling
     */

    static void create_window(unsigned int width, unsigned int height, const std::string &name = "ASR: Version 4.0")
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

    static void create_window(const std::string &name = "ASR: Version 1.3")
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
     * Material Handling
     */

    static Material create_material(const std::string &vertex_shader, const std::string &fragment_shader)
    {
        Material material{vertex_shader, fragment_shader};

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

        GLint uniform_count;
        const GLsizei uniform_name_buffer_size = 256;
        GLchar uniform_name_buffer[uniform_name_buffer_size];
        GLint uniform_size;
        GLenum uniform_type;

        glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count);
        for (GLuint i = 0; i < static_cast<GLuint>(uniform_count); i++) {
            GLsizei uniform_name_length;
            glGetActiveUniform(
                shader_program, i,
                uniform_name_buffer_size,
                &uniform_name_length,
                &uniform_size,
                &uniform_type,
                uniform_name_buffer
            );

            if (uniform_size > 1) {
                std::string uniform_name{uniform_name_buffer};
                uniform_name = uniform_name.substr(0, uniform_name.size() - 3);
                for (GLint j = 0; j < uniform_size; ++j) {
                    std::string uniform_name_at_slot = uniform_name + "[" + std::to_string(j) + "]";
                    material.shader_uniforms[uniform_name_at_slot] =
                        glGetUniformLocation(shader_program, uniform_name_at_slot.c_str());
                }
            } else {
                std::string uniform_name(uniform_name_buffer);
                material.shader_uniforms[uniform_name] =
                    glGetUniformLocation(shader_program, uniform_name_buffer);
            }
        }

        glDetachShader(shader_program, vertex_shader_object);
        glDetachShader(shader_program, fragment_shader_object);
        glDeleteShader(vertex_shader_object);
        glDeleteShader(fragment_shader_object);

        material.position_attribute_location =
            glGetAttribLocation(shader_program, "position");
        material.normal_attribute_location =
            glGetAttribLocation(shader_program, "normal");
        material.color_attribute_location =
            glGetAttribLocation(shader_program, "color");
        material.texture_coordinates_attribute_location =
            glGetAttribLocation(shader_program, "texture_coordinates");

        material.instance_transform_attribute_location =
            glGetAttribLocation(shader_program, "instance_transform");
        material.instance_color_attribute_location =
            glGetAttribLocation(shader_program, "instance_color");

        material.resolution_uniform_location =
            glGetUniformLocation(shader_program, "resolution");
        material.mouse_uniform_location =
            glGetUniformLocation(shader_program, "mouse");

        material.time_uniform_location =
            glGetUniformLocation(shader_program, "time");
        material.dt_uniform_location =
            glGetUniformLocation(shader_program, "dt");

        material.texture_enabled_uniform_location =
            glGetUniformLocation(shader_program, "texture_enabled");
        material.texture_transformation_matrix_uniform_location =
            glGetUniformLocation(shader_program, "texture_transformation_matrix");
        material.texturing_mode_uniform_location =
            glGetUniformLocation(shader_program, "texturing_mode");
        material.texture_sampler_uniform_location =
            glGetUniformLocation(shader_program, "texture_sampler");

        material.point_size_uniform_location =
            glGetUniformLocation(shader_program, "point_size");

        material.model_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_matrix");
        material.view_matrix_uniform_location =
            glGetUniformLocation(shader_program, "view_matrix");
        material.model_view_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_view_matrix");
        material.projection_matrix_uniform_location =
            glGetUniformLocation(shader_program, "projection_matrix");
        material.view_projection_matrix_uniform_location =
            glGetUniformLocation(shader_program, "view_projection_matrix");
        material.mvp_matrix_uniform_location =
            glGetUniformLocation(shader_program, "model_view_projection_matrix");
        material.normal_matrix_uniform_location =
            glGetUniformLocation(shader_program, "normal_matrix");

        material.shader_program = shader_program;

        return material;
    }

    static void set_material_line_width(float line_width)
    {
        assert(data::current_material);

        data::current_material->line_width = line_width;
        glLineWidth(static_cast<GLfloat>(line_width));
    }

    static void set_material_point_sizing_enabled(bool point_sizing_enabled)
    {
        assert(data::current_material);

        data::current_material->point_sizing_enabled = point_sizing_enabled;
        if (point_sizing_enabled) {
            glEnable(GL_PROGRAM_POINT_SIZE);
        } else {
            glDisable(GL_PROGRAM_POINT_SIZE);
        }
    }

    static void set_material_point_size(float point_size)
    {
        assert(data::current_material);

        data::current_material->point_size = point_size;
    }

    static void set_material_face_culling_enabled(bool face_culling_enabled)
    {
        assert(data::current_material);

        data::current_material->face_culling_enabled = face_culling_enabled;
        if (face_culling_enabled) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

    static void set_material_cull_face_mode(MaterialCullFaceMode cull_face_mode)
    {
        assert(data::current_material);

        data::current_material->cull_face_mode = cull_face_mode;
        glCullFace(utilities::convert_cull_face_mode_to_es2_cull_face_mode(cull_face_mode));
    }

    static void set_material_front_face_order(MaterialFrontFaceOrder front_face_order)
    {
        assert(data::current_material);

        data::current_material->front_face_order = front_face_order;
        glFrontFace(utilities::convert_front_face_order_to_es2_front_face_order(front_face_order));
    }

    static void set_material_depth_mask_enabled(bool depth_mask_enabled)
    {
        assert(data::current_material);

        data::current_material->depth_mask_enabled = depth_mask_enabled;
        if (depth_mask_enabled) {
            glDepthMask(GL_TRUE);
        } else {
            glDepthMask(GL_FALSE);
        }
    }

    static void set_material_depth_test_enabled(bool depth_test_enabled)
    {
        assert(data::current_material);

        data::current_material->depth_test_enabled = depth_test_enabled;
        if (depth_test_enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    static void set_material_depth_test_function(MaterialDepthTestFunction depth_test_function)
    {
        assert(data::current_material);

        data::current_material->depth_test_function = depth_test_function;
        glDepthFunc(utilities::convert_depth_test_func_to_es2_depth_test_func(depth_test_function));
    }

    static void set_material_blending_enabled(bool blending_enabled)
    {
        assert(data::current_material);

        data::current_material->blending_enabled = blending_enabled;
        if (blending_enabled) {
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }
    }

    static void set_material_blending_equations(
                    MaterialBlendingEquation color_blending_equation,
                    MaterialBlendingEquation alpha_blending_equation
                )
    {
        assert(data::current_material);

        data::current_material->color_blending_equation = color_blending_equation;
        data::current_material->alpha_blending_equation = alpha_blending_equation;
        glBlendEquationSeparate(utilities::convert_blending_equation_to_es2_blending_equation(color_blending_equation),
                                utilities::convert_blending_equation_to_es2_blending_equation(alpha_blending_equation));
    }

    static void set_material_blending_functions(
                    MaterialBlendingFunction source_color_blending_function,
                    MaterialBlendingFunction source_alpha_blending_function,
                    MaterialBlendingFunction destination_color_blending_function,
                    MaterialBlendingFunction destination_alpha_blending_function
                )
    {
        assert(data::current_material);

        data::current_material->source_color_blending_function = source_color_blending_function;
        data::current_material->source_alpha_blending_function = source_alpha_blending_function;
        data::current_material->destination_color_blending_function = destination_color_blending_function;
        data::current_material->destination_alpha_blending_function = destination_alpha_blending_function;
        glBlendFuncSeparate(utilities::convert_blending_func_to_es2_blending_func(source_color_blending_function),
                            utilities::convert_blending_func_to_es2_blending_func(destination_color_blending_function),
                            utilities::convert_blending_func_to_es2_blending_func(source_alpha_blending_function),
                            utilities::convert_blending_func_to_es2_blending_func(destination_alpha_blending_function));
    }

    static void set_material_blending_constant_color(glm::vec4 blending_constant_color)
    {
        assert(data::current_material);

        data::current_material->blending_constant_color = blending_constant_color;
        glBlendColor(static_cast<GLclampf>(blending_constant_color[0]),
                     static_cast<GLclampf>(blending_constant_color[1]),
                     static_cast<GLclampf>(blending_constant_color[2]),
                     static_cast<GLclampf>(blending_constant_color[3]));
    }

    static void set_material_polygon_offset_enabled(bool polygon_offset_enabled)
    {
        assert(data::current_material);

        data::current_material->polygon_offset_enabled = polygon_offset_enabled;
        if (polygon_offset_enabled) {
           glEnable(GL_POLYGON_OFFSET_FILL);
        } else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    static void set_material_polygon_offset_factor_and_units(float polygon_offset_factor, float polygon_offset_units)
    {
        assert(data::current_material);

        data::current_material->polygon_offset_factor = polygon_offset_factor;
        data::current_material->polygon_offset_units = polygon_offset_units;
        glPolygonOffset(static_cast<GLfloat>(polygon_offset_factor),
                        static_cast<GLfloat>(polygon_offset_units));
    }

    static void set_material_parameter(const std::string &parameter_name, bool value)
    {
        assert(data::current_material);

        glUniform1i(
            data::current_material->shader_uniforms[parameter_name],
            static_cast<GLint>(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int value)
    {
        assert(data::current_material);

        glUniform1i(
            data::current_material->shader_uniforms[parameter_name],
            static_cast<GLint>(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int count, int *values)
    {
        assert(data::current_material);

        glUniform1iv(
            data::current_material->shader_uniforms[parameter_name],
            count,
            static_cast<GLint *>(values)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, float value)
    {
        assert(data::current_material);

        glUniform1f(
            data::current_material->shader_uniforms[parameter_name],
            static_cast<GLfloat>(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int count, float *values)
    {
        assert(data::current_material);

        glUniform1fv(
            data::current_material->shader_uniforms[parameter_name],
            count,
            static_cast<GLfloat *>(values)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, glm::vec2 value)
    {
        assert(data::current_material);

        glUniform2fv(
            data::current_material->shader_uniforms[parameter_name],
            1, glm::value_ptr(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int count, glm::vec2 *values)
    {
        assert(data::current_material);

        glUniform2fv(
            data::current_material->shader_uniforms[parameter_name],
            count, glm::value_ptr(values[0])
        );
    }

    static void set_material_parameter(const std::string &parameter_name, glm::vec3 value)
    {
        assert(data::current_material);

        glUniform3fv(
            data::current_material->shader_uniforms[parameter_name],
            1, glm::value_ptr(value)
        );
    }

    static void set_material_parameter(const std::string& parameter_name, int count, glm::vec3 *values)
    {
        assert(data::current_material);

        glUniform3fv(
            data::current_material->shader_uniforms[parameter_name],
            count, glm::value_ptr(values[0])
        );
    }

    static void set_material_parameter(const std::string &parameter_name, glm::vec4 value)
    {
        assert(data::current_material);

        glUniform4fv(
            data::current_material->shader_uniforms[parameter_name],
            1, glm::value_ptr(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int count, glm::vec4 *values)
    {
        assert(data::current_material);

        glUniform4fv(
            data::current_material->shader_uniforms[parameter_name],
            count, glm::value_ptr(values[0])
        );
    }

    static void set_material_parameter(const std::string &parameter_name, glm::mat3 value)
    {
        assert(data::current_material);

        glUniformMatrix3fv(
            data::current_material->shader_uniforms[parameter_name],
            1, GL_FALSE,
            glm::value_ptr(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int  count, glm::mat3 *values)
    {
        assert(data::current_material);

        glUniformMatrix3fv(
            data::current_material->shader_uniforms[parameter_name],
            count, GL_FALSE,
            glm::value_ptr(values[0])
        );
    }

    static void set_material_parameter(const std::string &parameter_name, glm::mat4 value)
    {
        assert(data::current_material);

        glUniformMatrix4fv(
            data::current_material->shader_uniforms[parameter_name],
            1, GL_FALSE,
            glm::value_ptr(value)
        );
    }

    static void set_material_parameter(const std::string &parameter_name, int  count, glm::mat4 *values)
    {
        assert(data::current_material);

        glUniformMatrix4fv(
            data::current_material->shader_uniforms[parameter_name],
            count, GL_FALSE,
            glm::value_ptr(values[0])
        );
    }

    static void set_material_current(Material *material)
    {
        data::current_material = material;
        if (material != nullptr) {
            glUseProgram(material->shader_program);

            glLineWidth(static_cast<GLfloat>(material->line_width));

            if (material->point_sizing_enabled) {
                glEnable(GL_PROGRAM_POINT_SIZE);
            } else {
                glDisable(GL_PROGRAM_POINT_SIZE);
            }

            if (material->face_culling_enabled) {
                glEnable(GL_CULL_FACE);
            } else {
                glDisable(GL_CULL_FACE);
            }
            glCullFace(utilities::convert_cull_face_mode_to_es2_cull_face_mode(material->cull_face_mode));
            glFrontFace(utilities::convert_front_face_order_to_es2_front_face_order(material->front_face_order));

            if (material->depth_mask_enabled) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }
            if (material->depth_test_enabled) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            glDepthFunc(utilities::convert_depth_test_func_to_es2_depth_test_func(material->depth_test_function));

            if (material->blending_enabled) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            glBlendEquationSeparate(
                utilities::convert_blending_equation_to_es2_blending_equation(material->color_blending_equation),
                utilities::convert_blending_equation_to_es2_blending_equation(material->alpha_blending_equation)
            );
            glBlendFuncSeparate(
                utilities::convert_blending_func_to_es2_blending_func(material->source_color_blending_function),
                utilities::convert_blending_func_to_es2_blending_func(material->destination_color_blending_function),
                utilities::convert_blending_func_to_es2_blending_func(material->source_alpha_blending_function),
                utilities::convert_blending_func_to_es2_blending_func(material->destination_alpha_blending_function)
            );
            glBlendColor(
                static_cast<GLclampf>(material->blending_constant_color[0]),
                static_cast<GLclampf>(material->blending_constant_color[1]),
                static_cast<GLclampf>(material->blending_constant_color[2]),
                static_cast<GLclampf>(material->blending_constant_color[3])
            );

            if (material->polygon_offset_enabled) {
                glEnable(GL_POLYGON_OFFSET_FILL);
            } else {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            glPolygonOffset(
                static_cast<GLfloat>(material->polygon_offset_factor),
                static_cast<GLfloat>(material->polygon_offset_units)
            );
        } else {
            glUseProgram(0);
        }
    }

    static void destroy_material(Material &material)
    {
        GLint current_shader_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current_shader_program);
        if (material.shader_program == current_shader_program) {
            glUseProgram(0);
        }

        glDeleteProgram(material.shader_program);
        material.shader_program = 0;

        material.position_attribute_location = -1;
        material.normal_attribute_location = -1;
        material.color_attribute_location = -1;
        material.texture_coordinates_attribute_location = -1;

        material.instance_transform_attribute_location = -1;
        material.instance_color_attribute_location = -1;

        material.resolution_uniform_location = -1;
        material.mouse_uniform_location = -1;

        material.time_uniform_location = -1;
        material.dt_uniform_location = -1;

        material.texture_enabled_uniform_location = -1;
        material.texture_transformation_matrix_uniform_location = -1;
        material.texturing_mode_uniform_location = -1;
        material.texture_sampler_uniform_location = -1;

        material.point_size_uniform_location = -1;

        material.model_matrix_uniform_location = -1;
        material.view_matrix_uniform_location = -1;
        material.model_view_matrix_uniform_location = -1;
        material.projection_matrix_uniform_location = -1;
        material.view_projection_matrix_uniform_location = -1;
        material.mvp_matrix_uniform_location = -1;
    }

    /*
     * Geometry Handling
     */

    static Geometry create_geometry(
                        GeometryType type, Vertices vertices, Indices indices,
                        Instances instances = {}
                    )
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
            static_cast<GLsizeiptr>(vertices.size() * 12 * sizeof(float)),
            reinterpret_cast<const float *>(vertices.data()),
            GL_STATIC_DRAW
        );

        if (!instances.empty()) {
            geometry.instance_count = static_cast<unsigned int>(instances.size());

            glGenBuffers(1, &geometry.instance_buffer_object);
            glBindBuffer(GL_ARRAY_BUFFER, geometry.instance_buffer_object);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(instances.size() * 20 * sizeof(float)),
                reinterpret_cast<const float *>(instances.data()),
                GL_STATIC_DRAW
            );
        }

        glGenBuffers(1, &geometry.index_buffer_object);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.index_buffer_object);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(decltype(indices)::value_type)),
            reinterpret_cast<const unsigned int *>(indices.data()),
            GL_STATIC_DRAW
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
            assert(data::current_material);

#ifdef __APPLE__
            glBindVertexArrayAPPLE(geometry->vertex_array_object);
#else
            glBindVertexArray(geometry->vertex_array_object);
#endif
            glBindBuffer(GL_ARRAY_BUFFER, geometry->vertex_buffer_object);

            GLsizei stride = sizeof(GLfloat) * 12;
            glEnableVertexAttribArray(data::current_material->position_attribute_location);
            glVertexAttribPointer(
                data::current_material->position_attribute_location,
                3, GL_FLOAT, GL_FALSE, stride, static_cast<const GLvoid *>(nullptr)
            );
            glEnableVertexAttribArray(data::current_material->normal_attribute_location);
            glVertexAttribPointer(
                data::current_material->normal_attribute_location,
                3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 3)
            );
            glEnableVertexAttribArray(data::current_material->color_attribute_location);
            glVertexAttribPointer(
                data::current_material->color_attribute_location,
                4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 6)
            );
            glEnableVertexAttribArray(data::current_material->texture_coordinates_attribute_location);
            glVertexAttribPointer(
                data::current_material->texture_coordinates_attribute_location,
                2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 10)
            );

            if (geometry->instance_buffer_object) {
                glBindBuffer(GL_ARRAY_BUFFER, geometry->instance_buffer_object);

                stride = sizeof(GLfloat) * 20;
                glEnableVertexAttribArray(data::current_material->instance_transform_attribute_location);
                glVertexAttribPointer(
                    data::current_material->instance_transform_attribute_location,
                    4, GL_FLOAT, GL_FALSE, stride, static_cast<const GLvoid *>(nullptr)
                );
                glVertexAttribDivisorARB(data::current_material->instance_transform_attribute_location, 1);
                glEnableVertexAttribArray(data::current_material->instance_transform_attribute_location + 1);
                glVertexAttribPointer(
                    data::current_material->instance_transform_attribute_location + 1,
                    4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 4)
                );
                glVertexAttribDivisorARB(data::current_material->instance_transform_attribute_location + 1, 1);
                glEnableVertexAttribArray(data::current_material->instance_transform_attribute_location + 2);
                glVertexAttribPointer(
                    data::current_material->instance_transform_attribute_location + 2,
                    4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 8)
                );
                glVertexAttribDivisorARB(data::current_material->instance_transform_attribute_location + 2, 1);
                glEnableVertexAttribArray(data::current_material->instance_transform_attribute_location + 3);
                glVertexAttribPointer(
                    data::current_material->instance_transform_attribute_location + 3,
                    4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 12)
                );
                glVertexAttribDivisorARB(data::current_material->instance_transform_attribute_location + 3, 1);

                glEnableVertexAttribArray(data::current_material->instance_color_attribute_location);
                glVertexAttribPointer(
                    data::current_material->instance_color_attribute_location,
                    4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 16)
                );
                glVertexAttribDivisorARB(data::current_material->instance_color_attribute_location, 1);
            }

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->index_buffer_object);

#ifdef __APPLE__
            glBindVertexArrayAPPLE(0);
#else
            glBindVertexArray(0);
#endif
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

        if (geometry.instance_buffer_object) {
            GLint current_instance_buffer_object;
            glGetIntegerv(GL_VERTEX_ARRAY_BUFFER_BINDING, &current_instance_buffer_object);
            if (geometry.instance_buffer_object == static_cast<GLint>(current_instance_buffer_object)) {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            glDeleteBuffers(1, &geometry.instance_buffer_object);
            geometry.instance_buffer_object = 0;
            geometry.instance_count = 0;
        }

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

    static inline glm::mat4 get_view_matrix_inverted()
    {
        return glm::inverse(data::view_matrix_stack.top());
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

    static void prepare_to_render_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        data::frame_rendering_start_time = std::chrono::system_clock::now();
    }

    static void render_current_geometry()
    {
        assert(data::current_geometry);
        assert(data::current_material);

        if (data::current_material->resolution_uniform_location != -1) {
            glUniform2f(
                data::current_material->resolution_uniform_location,
                static_cast<GLfloat>(data::window_width),
                static_cast<GLfloat>(data::window_height)
            );
        }

        if (data::current_material->mouse_uniform_location != -1) {
            glUniform2f(
                data::current_material->mouse_uniform_location,
                static_cast<GLfloat>(data::mouse_x),
                static_cast<GLfloat>(data::mouse_y)
            );
        }

        if (data::current_material->time_uniform_location != -1) {
            float time =
                static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - data::rendering_start_time
                ).count()) / 1000.0f;

            glUniform1f(
                data::current_material->time_uniform_location,
                static_cast<GLfloat>(time)
            );
        }

        if (data::current_material->dt_uniform_location != -1) {
            glUniform1f(
                data::current_material->dt_uniform_location,
                static_cast<GLfloat>(data::frame_rendering_delta_time)
            );
        }

        bool texture_enabled = data::current_texture != nullptr;
        if (data::current_material->texture_enabled_uniform_location != -1) {
            glUniform1i(
                data::current_material->texture_enabled_uniform_location,
                static_cast<GLint>(texture_enabled)
            );
        }

        if (data::current_material->texture_sampler_uniform_location != -1) {
            glUniform1i(data::current_material->texture_sampler_uniform_location, 0);
        }

        if (data::current_material->texturing_mode_uniform_location != -1 && data::current_texture != nullptr) {
            glUniform1i(
                data::current_material->texturing_mode_uniform_location,
                static_cast<GLint>(data::current_texture->mode)
            );
        }

        if (data::current_material->texture_transformation_matrix_uniform_location != -1) {
            glm::mat4 texture_matrix = data::texture_matrix_stack.top();
            glUniformMatrix4fv(
                data::current_material->texture_transformation_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(texture_matrix)
            );
        }

        if (data::current_material->point_size_uniform_location != -1) {
            glUniform1f(
                data::current_material->point_size_uniform_location,
                static_cast<GLfloat>(data::current_material->point_size)
            );
        }

        if (data::current_material->model_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glUniformMatrix4fv(
                data::current_material->model_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_matrix)
            );
        }

        if (data::current_material->view_matrix_uniform_location != -1) {
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glUniformMatrix4fv(
                data::current_material->view_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(view_matrix)
            );
        }

        if (data::current_material->model_view_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 model_view_matrix = view_matrix * model_matrix;
            glUniformMatrix4fv(
                data::current_material->model_view_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_view_matrix)
            );
        }

        if (data::current_material->projection_matrix_uniform_location != -1) {
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glUniformMatrix4fv(
                data::current_material->projection_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(projection_matrix)
            );
        }

        if (data::current_material->view_projection_matrix_uniform_location != -1) {
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glm::mat4 view_projection_matrix = projection_matrix * view_matrix;
            glUniformMatrix4fv(
                data::current_material->view_projection_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(view_projection_matrix)
            );
        }

        if (data::current_material->mvp_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 projection_matrix = data::projection_matrix_stack.top();
            glm::mat4 model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;
            glUniformMatrix4fv(
                data::current_material->mvp_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(model_view_projection_matrix)
            );
        }

        if (data::current_material->normal_matrix_uniform_location != -1) {
            glm::mat4 model_matrix = data::model_matrix_stack.top();
            glm::mat4 view_matrix = glm::inverse(data::view_matrix_stack.top());
            glm::mat4 model_view_matrix = view_matrix * model_matrix;
            glm::mat3 normal_matrix = glm::inverseTranspose(glm::mat3(model_view_matrix));
            glUniformMatrix3fv(
                data::current_material->normal_matrix_uniform_location,
                1, GL_FALSE,
                glm::value_ptr(normal_matrix)
            );
        }

        if (data::current_geometry->instance_count > 0) {
            glDrawElementsInstancedARB(
                utilities::convert_geometry_type_to_es2_geometry_type(data::current_geometry->type),
                static_cast<GLsizei>(data::current_geometry->vertex_count),
                GL_UNSIGNED_INT,
                nullptr,
                static_cast<GLsizei>(data::current_geometry->instance_count)
            );
        } else {
            glDrawElements(
                utilities::convert_geometry_type_to_es2_geometry_type(data::current_geometry->type),
                static_cast<GLsizei>(data::current_geometry->vertex_count),
                GL_UNSIGNED_INT,
                nullptr
            );
        }
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
