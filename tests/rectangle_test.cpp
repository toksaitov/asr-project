#include "asr.h"

#include <string>
#include <utility>

static const std::string Vertex_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    attribute vec4 position;
    attribute vec4 color;

    uniform mat4 model_view_projection_matrix;

    varying vec4 fragment_color;

    void main()
    {
        fragment_color = color;

        gl_Position = model_view_projection_matrix * position;
        gl_PointSize = 10.0;
    }
)"};

static const std::string Fragment_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    varying vec4 fragment_color;

    void main()
    {
        gl_FragColor = fragment_color;
    }
)"};

static asr::GeometryPair generate_rectangle_geometry_data(
                             asr::GeometryType geometry_type,
                             float width,float height,
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
        for (auto j = 0U; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
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

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500, "Rectangle Test on ASR Version 1.1");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);

    float width{1.0f}, height{1.0f};
    unsigned int width_segments{5}, height_segments{5};

    auto [triangle_vertices, triangle_indices] =
        generate_rectangle_geometry_data(
            Triangles, width, height, width_segments, height_segments
        );
    auto triangles =
        create_geometry(
            Triangles, triangle_vertices, triangle_indices
        );

    glm::vec4 edge_color{1.0f, 0.7f, 0.7f, 1.0f};
    auto [edge_vertices, edge_indices] =
        generate_rectangle_geometry_data(
            Lines, width, height, width_segments, height_segments, edge_color
        );
    for (auto &vertex : edge_vertices) { vertex.z -= 0.01f; }
    auto lines = create_geometry(Lines, edge_vertices, edge_indices);

    glm::vec4 vertex_color{1.0f, 0.0f, 0.0f, 1.0f};
    auto [vertices, vertex_indices] =
        generate_rectangle_geometry_data(
            Points, width, height, width_segments, height_segments, vertex_color
        );
    for (auto &vertex : vertices) { vertex.z -= 0.02f; }
    auto points = create_geometry(Points, vertices, vertex_indices);

    prepare_for_rendering();
    set_line_width(3);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_geometry_current(&triangles);
        render_current_geometry();

        set_geometry_current(&lines);
        render_current_geometry();

        set_geometry_current(&points);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_geometry(triangles);
    destroy_geometry(lines);
    destroy_geometry(points);

    destroy_shader();
    destroy_window();

    return 0;
}
