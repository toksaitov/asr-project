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

static asr::GeometryPair generate_circle_geometry_data(
                             asr::GeometryType geometry_type,
                             float radius, unsigned int segments,
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

    vertices.push_back(asr::Vertex{
        0.0f, 0.0f, 0.0f,
        color.r, color.g, color.b, color.a
    });
    if (geometry_type == asr::GeometryType::Points) {
        indices.push_back(0);
    }

    float angle{0.0f};
    float angle_delta{asr::two_pi / static_cast<float>(segments)};

    float x{std::cos(angle) * radius};
    float y{std::sin(angle) * radius};
    vertices.push_back(asr::Vertex{
        x, y, 0.0f,
        color.r, color.g, color.b, color.a
    });
    if (geometry_type == asr::GeometryType::Points) {
        indices.push_back(1);
    }

    for (auto i = 0U; i < segments; ++i) {
        if (geometry_type == asr::GeometryType::Triangles ||
                geometry_type == asr::GeometryType::Lines) {
            indices.push_back(0);
            indices.push_back(vertices.size() - 1);
            if (geometry_type == asr::GeometryType::Lines) {
                indices.push_back(vertices.size() - 1);
            }
        }
        float next_x{std::cos(angle + angle_delta) * radius};
        float next_y{std::sin(angle + angle_delta) * radius};
        vertices.push_back(asr::Vertex{
            next_x, next_y, 0.0f,
            color.r, color.g, color.b, color.a
        });
        indices.push_back(vertices.size() - 1);

        angle += angle_delta;
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500, "Circle Test on ASR Version 1.1");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);

    float radius{0.5f};
    unsigned int segments{10};

    auto [triangle_vertices, triangle_indices] = generate_circle_geometry_data(Triangles, radius, segments);
    auto triangles = create_geometry(Triangles, triangle_vertices, triangle_indices);

    glm::vec4 edge_color{1.0f, 0.7f, 0.7f, 1.0f};
    auto [edge_vertices, edge_indices] = generate_circle_geometry_data(Lines, radius, segments, edge_color);
    for (auto &vertex : edge_vertices) { vertex.z -= 0.01f; }
    auto lines = create_geometry(Lines, edge_vertices, edge_indices);

    glm::vec4 vertex_color{1.0f, 0.0f, 0.0f, 1.0f};
    auto [vertices, vertex_indices] = generate_circle_geometry_data(Points, radius, segments, vertex_color);
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
