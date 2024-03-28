#include "asr.h"

#include <string>
#include <utility>

static const std::string Vertex_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    attribute vec4 position;
    attribute vec4 color;
    attribute vec4 texture_coordinates;

    uniform bool texture_enabled;
    uniform mat4 texture_transformation_matrix;

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
        gl_PointSize = 10.0;
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

static asr::GeometryPair generate_box_geometry_data(
                             asr::GeometryType geometry_type,
                             float width, float height, float depth,
                             unsigned int width_segments_count,
                             unsigned int height_segments_count,
                             unsigned int depth_segments_count,
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

    float half_depth{depth * 0.5f};
    float segment_depth{depth / static_cast<float>(depth_segments_count)};

    // Front Face of the Box

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, half_depth,
                color.r, color.g, color.b, color.a
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

    unsigned int offset = vertices.size();

    // Right Face of the Box

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (auto j = 0U; j <= depth_segments_count; ++j) {
            float z{static_cast<float>(j) * segment_depth - half_depth};
            vertices.push_back(asr::Vertex{
                half_width, y, z,
                color.r, color.g, color.b, color.a
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
                index_a += offset; index_b += offset;
                index_c += offset; index_d += offset;
                if (geometry_type == asr::GeometryType::Lines) {
                    indices.insert(indices.end(), {
                        index_a, index_b, index_b, index_c, index_c, index_a
                    });
                    indices.insert(indices.end(), {
                        index_b, index_d, index_d, index_c, index_c, index_b
                    });
                } else {
                    indices.insert(indices.end(), {index_a, index_c, index_b});
                    indices.insert(indices.end(), {index_b, index_c, index_d});
                }
            }
        }
    }

    offset = vertices.size();

    // Back Face of the Box

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, -half_depth,
                color.r, color.g, color.b, color.a
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
                index_a += offset; index_b += offset;
                index_c += offset; index_d += offset;
                if (geometry_type == asr::GeometryType::Lines) {
                    indices.insert(indices.end(), {
                        index_a, index_b, index_b, index_c, index_c, index_a
                    });
                    indices.insert(indices.end(), {
                        index_b, index_d, index_d, index_c, index_c, index_b
                    });
                } else {
                    indices.insert(indices.end(), {index_a, index_c, index_b});
                    indices.insert(indices.end(), {index_b, index_c, index_d});
                }
            }
        }
    }

    offset = vertices.size();

    // Left Face of the Box

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (auto j = 0U; j <= depth_segments_count; ++j) {
            float z{static_cast<float>(j) * segment_depth - half_depth};
            vertices.push_back(asr::Vertex{
                -half_width, y, z,
                color.r, color.g, color.b, color.a
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
                index_a += offset; index_b += offset;
                index_c += offset; index_d += offset;
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

    offset = vertices.size();

    // Bottom Face of the Box

    for (auto i = 0U; i <= depth_segments_count; ++i) {
        float z{static_cast<float>(i) * segment_depth - half_depth};
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, -half_height, z,
                color.r, color.g, color.b, color.a
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
                index_a += offset; index_b += offset;
                index_c += offset; index_d += offset;
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

    offset = vertices.size();

    // Top Face of the Box

    for (auto i = 0U; i <= depth_segments_count; ++i) {
        float z{static_cast<float>(i) * segment_depth - half_depth};
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, half_height, z,
                color.r, color.g, color.b, color.a
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
                index_a += offset; index_b += offset;
                index_c += offset; index_d += offset;
                if (geometry_type == asr::GeometryType::Lines) {
                    indices.insert(indices.end(), {
                        index_a, index_b, index_b, index_c, index_c, index_a
                    });
                    indices.insert(indices.end(), {
                        index_b, index_d, index_d, index_c, index_c, index_b
                    });
                } else {
                    indices.insert(indices.end(), {index_a, index_c, index_b});
                    indices.insert(indices.end(), {index_b, index_c, index_d});
                }
            }
        }
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500U, 500U, "Box Test on ASR Version 1.2");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);

    float width{1.0f}, height{1.0f}, depth{1.0f};
    unsigned int width_segments{5U}, height_segments{5U}, depth_segments{5U};

    auto [triangle_vertices, triangle_indices] = generate_box_geometry_data(Triangles, width, height, depth, width_segments, height_segments, depth_segments);
    auto triangles = create_geometry(Triangles, triangle_vertices, triangle_indices);

    glm::vec4 edge_color{1.0f, 0.7f, 0.7f, 1.0f};
    auto [edge_vertices, edge_indices] = generate_box_geometry_data(Lines, width * 1.005f, height * 1.005f, depth * 1.005f, width_segments, height_segments, depth_segments, edge_color);
    auto lines = create_geometry(Lines, edge_vertices, edge_indices);

    glm::vec4 vertex_color{1.0f, 0.0f, 0.0f, 1.0f};
    auto [vertices, vertex_indices] = generate_box_geometry_data(Points, width * 1.01f, height * 1.01f, depth * 1.01f, width_segments, height_segments, depth_segments, vertex_color);
    auto points = create_geometry(Points, vertices, vertex_indices);
    auto image = read_image_file("data/images/cubemap_test.png");
    auto texture = create_texture(image);

    prepare_for_rendering();
    enable_face_culling();
    enable_depth_test();
    set_line_width(3.0f);

    static const float CAMERA_SPEED{6.0f};
    static const float CAMERA_ROT_SPEED{1.5f};
    static const float CAMERA_FOV{1.13f};
    static const float CAMERA_NEAR_PLANE{0.1f};
    static const float CAMERA_FAR_PLANE{100.0f};

    glm::vec3 camera_position{1.40f, 1.0f, 1.5f};
    glm::vec3 camera_rotation{-0.5f, 0.75f, 0.0f};
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

        set_texture_current(&texture);
        set_geometry_current(&triangles);
        render_current_geometry();

        set_texture_current(nullptr);
        set_geometry_current(&lines);
        render_current_geometry();
        set_geometry_current(&points);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_texture(texture);
    destroy_geometry(triangles);
    destroy_geometry(lines);
    destroy_geometry(points);
    destroy_shader();

    destroy_window();

    return 0;
}
