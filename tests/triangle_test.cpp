#include "asr.h"

#include <cstddef>
#include <string>

static const std::string Vertex_Shader_Source = R"(
    #version 110

    attribute vec4 position;
    attribute vec4 color;

    uniform float time;

    varying vec4 fragment_color;

    void main()
    {
        fragment_color = color;

        vec4 rotated_position = position;
        rotated_position.x = position.x * cos(time) - position.y * sin(time);
        rotated_position.y = position.x * sin(time) + position.y * cos(time);

        gl_Position = rotated_position;
    }
)";

static const std::string Fragment_Shader_Source = R"(
    #version 110

    varying vec4 fragment_color;

    void main()
    {
        gl_FragColor = fragment_color;
    }
)";

static const float Triangle_Geometry_Data[] = {
//   Position             Color (RGBA)
     0.5f, -0.305f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.0f,  0.565f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.305f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
};
static const size_t Triangle_Geometry_Vertex_Count{3};

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500, "Triangle Test on ASR Version 1.0");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);
    create_geometry(GeometryType::Triangles, Triangle_Geometry_Data, Triangle_Geometry_Vertex_Count);

    prepare_for_rendering();
    bool should_stop = false;
    while (!should_stop) {
        process_window_events(&should_stop);
        render_next_frame();
    }

    destroy_geometry();
    destroy_shader();
    destroy_window();

    return 0;
}
