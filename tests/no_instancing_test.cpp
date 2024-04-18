#include "asr.h"

#include <string>
#include <utility>
#include <vector>

static const std::string Vertex_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    attribute vec4 position;
    attribute vec4 color;
    attribute vec4 texture_coordinates;

    uniform bool texture_enabled;
    uniform mat4 texture_transformation_matrix;

    uniform float point_size;

    uniform mat4 model_view_projection_matrix;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        fragment_color = color;
        if (texture_enabled) {
            vec4 transformed_texture_coordinates = texture_transformation_matrix * vec4(texture_coordinates.st, 0.0, 1.0);
            fragment_texture_coordinates = vec2(transformed_texture_coordinates);
        }

        gl_Position = model_view_projection_matrix * position;
        gl_PointSize = point_size;
    }
)"};

static const std::string Fragment_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    #define TEXTURING_MODE_ADDITION            0
    #define TEXTURING_MODE_SUBTRACTION         1
    #define TEXTURING_MODE_REVERSE_SUBTRACTION 2
    #define TEXTURING_MODE_MODULATION          3
    #define TEXTURING_MODE_DECALING            4

    uniform bool texture_enabled;
    uniform int texturing_mode;
    uniform sampler2D texture_sampler;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        gl_FragColor = fragment_color;

        if (texture_enabled) {
            if (texturing_mode == TEXTURING_MODE_ADDITION) {
                gl_FragColor += texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_MODULATION) {
                gl_FragColor *= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_DECALING) {
                vec4 texel_color = texture2D(texture_sampler, fragment_texture_coordinates);
                gl_FragColor.rgb = mix(gl_FragColor.rgb, texel_color.rgb, texel_color.a);
            } else if (texturing_mode == TEXTURING_MODE_SUBTRACTION) {
                gl_FragColor -= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_REVERSE_SUBTRACTION) {
                gl_FragColor = texture2D(texture_sampler, fragment_texture_coordinates) - gl_FragColor;
            }
        }
    }
)"};

static asr::GeometryPair generate_rectangle_geometry_data(
                             asr::GeometryType geometry_type,
                             float width, float height,
                             unsigned int width_segments_count,
                             unsigned int height_segments_count,
                             glm::vec4 color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
                         )
{
    assert(
        geometry_type == asr::GeometryType::Triangles ||
        geometry_type == asr::GeometryType::Lines     ||
        geometry_type == asr::GeometryType::Points
    );

    asr::Vertices vertices;
    asr::Indices indices;

    float half_height{height * 0.5f};
    float segment_height{height / static_cast<float>(height_segments_count)};

    float half_width{width * 0.5f};
    float segment_width{width / static_cast<float>(width_segments_count)};

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        float v{1.0f - static_cast<float>(i) / static_cast<float>(height_segments_count)};
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            float u{static_cast<float>(j) / static_cast<float>(width_segments_count)};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
                0.0f, 0.0f, 1.0f,
                color.r, color.g, color.b, color.a,
                u, v
            });
            if (geometry_type == asr::GeometryType::Points) {
                indices.push_back(vertices.size() - 1);
            }
        }
    }

    if (geometry_type == asr::GeometryType::Lines || geometry_type == asr::GeometryType::Triangles) {
        for (auto i = 0U; i < height_segments_count; ++i) {
            for (auto j = 0U; j < width_segments_count; ++j) {
                unsigned int index_a{i * (width_segments_count + 1) + j};
                unsigned int index_b{index_a + 1};
                unsigned int index_c{index_a + (width_segments_count + 1)};
                unsigned int index_d{index_c + 1};
                if (geometry_type == asr::GeometryType::Lines) {
                    indices.insert(indices.end(), {
                        index_a, index_b, index_b, index_c, index_c, index_a
                    });
                    indices.insert(indices.end(), {
                        index_b, index_d, index_d, index_c, index_c, index_b
                    });
                } else {
                    indices.insert(indices.end(), {index_a, index_b, index_c});
                    indices.insert(indices.end(), {index_b, index_d, index_c});
                }
            }
        }
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500U, 500U, "No Instancing Test on ASR Version 1.3");

    auto material = create_material(Vertex_Shader_Source, Fragment_Shader_Source);

    float width{1.0f}, height{1.0f};
    unsigned int width_segments{5U}, height_segments{5U};

    auto [geometry_vertices, geometry_indices] = generate_rectangle_geometry_data(Triangles, width, height, width_segments, height_segments);
    auto geometry = create_geometry(Triangles, geometry_vertices, geometry_indices);

    auto image = read_image_file("data/images/uv_test.png");
    auto texture = create_texture(image);

    prepare_for_rendering();

    set_material_current(&material);
    set_material_depth_test_enabled(true);
    set_material_face_culling_enabled(false);
    set_material_line_width(3.0f);
    set_material_point_size(10.0f);

    static const float CAMERA_SPEED{6.0f};
    static const float CAMERA_ROT_SPEED{1.5f};
    static const float CAMERA_FOV{1.13f};
    static const float CAMERA_NEAR_PLANE{0.1f};
    static const float CAMERA_FAR_PLANE{10000.0f};

    glm::vec3 camera_position{-18.5f, 52.5f, -18.5f};
    glm::vec3 camera_rotation{-0.65, -2.36, 0.0f};
    set_keys_down_event_handler([&](const uint8_t *keys) {
        if (keys[SDL_SCANCODE_ESCAPE]) std::exit(0);
        if (keys[SDL_SCANCODE_W]) camera_rotation.x -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_A]) camera_rotation.y += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_S]) camera_rotation.x += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_D]) camera_rotation.y -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_UP]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position -= shift;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position += shift;
        }
    });
    set_matrix_mode(Projection);
    load_perspective_projection_matrix(CAMERA_FOV, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_matrix_mode(View);
        load_identity_matrix();
        translate_matrix(camera_position);
        rotate_matrix(camera_rotation);

        set_matrix_mode(Model);
        load_identity_matrix();
        set_texture_current(&texture);
        set_geometry_current(&geometry);

        static const unsigned int ROWS  = 20U;
        static const unsigned int COLS  = 20U;
        static const unsigned int DEPTH = 20U;
        static const float scale = 40.0f;

        for (unsigned int i = 0; i < ROWS; ++i) {
            for (unsigned int j = 0; j < COLS; ++j) {
                for (unsigned int k = 0; k < DEPTH; ++k) {
                    float y = static_cast<float>(i) / static_cast<float>(ROWS);
                    float x = static_cast<float>(j) / static_cast<float>(COLS);
                    float z = static_cast<float>(k) / static_cast<float>(DEPTH);

                    push_matrix();
                    translate_matrix(glm::vec3{x * scale, y * scale, z * scale});
                    render_current_geometry();
                    pop_matrix();
                }
            }
        }

        finish_frame_rendering();
    }

    destroy_texture(texture);

    destroy_geometry(geometry);
    destroy_material(material);

    destroy_window();

    return 0;
}
