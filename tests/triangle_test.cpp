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

static asr::GeometryPair generate_triangle_geometry_data(
                             asr::GeometryType geometry_type,
                             float width,float height,
                             unsigned int segments_count,
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
    float segment_height{height / static_cast<float>(segments_count)};

    float half_width{width * 0.5f};
    float segment_width{width / static_cast<float>(segments_count)};

    for (auto i = 0U; i <= segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        float v{1.0f - static_cast<float>(i) / static_cast<float>(segments_count)};
        for (auto j = 0U; j <= segments_count - i; ++j) {
            float x{(static_cast<float>(j) + 0.5f * i) * segment_width - half_width};
            float u{static_cast<float>(j) / static_cast<float>(segments_count) + 0.5f * i / static_cast<float>(segments_count)};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
                color.r, color.g, color.b, color.a,
                u, v
            });
            if (geometry_type == asr::GeometryType::Points) {
                indices.push_back(vertices.size() - 1);
            }
        }
    }

    if (geometry_type == asr::GeometryType::Lines || geometry_type == asr::GeometryType::Triangles) {
        for (auto i = 0U, cursor = 0U; i < segments_count; ++i) {
            for (auto j = 0U; j < segments_count - i; ++j, ++cursor) {
                unsigned int index_a{cursor};
                unsigned int index_b{index_a + 1};
                unsigned int index_c{index_a + (segments_count - i + 1)};
                unsigned int index_d{index_c + 1};
                if (geometry_type == asr::GeometryType::Lines) {
                    indices.insert(indices.end(), {
                        index_a, index_b, index_b, index_c, index_c, index_a
                    });
                    if (j < segments_count - i - 1) {
                        indices.insert(indices.end(), {
                            index_b, index_d, index_d, index_c, index_c, index_b
                        });
                    } else {
                        ++cursor;
                    }
                } else {
                    indices.insert(indices.end(), {index_a, index_b, index_c});
                    if (j < segments_count - i - 1) {
                        indices.insert(indices.end(), {index_b, index_d, index_c});
                    } else {
                        ++cursor;
                    }
                }
            }
        }
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500U, 500U, "Triangle Test on ASR Version 1.2");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);

    float width{1.0f}, height{1.0f};
    unsigned int segments{5U};

    auto [triangle_vertices, triangle_indices] = generate_triangle_geometry_data(Triangles, width, height, segments);
    auto triangles = create_geometry(Triangles, triangle_vertices, triangle_indices);

    glm::vec4 edge_color{1.0f, 0.7f, 0.7f, 1.0f};
    auto [edge_vertices, edge_indices] = generate_triangle_geometry_data(Lines, width, height, segments, edge_color);
    for (auto &vertex : edge_vertices) { vertex.z -= 0.01f; }
    auto lines = create_geometry(Lines, edge_vertices, edge_indices);

    glm::vec4 vertex_color{1.0f, 0.0f, 0.0f, 1.0f};
    auto [vertices, vertex_indices] = generate_triangle_geometry_data(Points, width, height, segments, vertex_color);
    for (auto &vertex : vertices) { vertex.z -= 0.02f; }
    auto points = create_geometry(Points, vertices, vertex_indices);

    auto image = read_image_file("data/images/uv_test.png");
    auto texture = create_texture(image);

    prepare_for_rendering();
    set_line_width(3.0f);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

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
