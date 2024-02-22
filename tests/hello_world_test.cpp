#include "asr.h"

#include <string>

static const std::string Vertex_Shader_Source{R"( // NOLINT(cert-err58-cpp)
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
)"};

static const std::string Fragment_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    varying vec4 fragment_color;

    void main()
    {
        gl_FragColor = fragment_color;
    }
)"};

static const asr::Vertices Triangle_Geometry_Vertices{ // NOLINT(cert-err58-cpp)
    //           Position             Color (RGBA)
    asr::Vertex{ 0.5f, -0.305f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    asr::Vertex{ 0.0f,  0.565f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    asr::Vertex{-0.5f, -0.305f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}
};
static const asr::Indices Triangle_Geometry_Indices{0U, 1U, 2U}; // NOLINT(cert-err58-cpp)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500, "Simple Triangle Test on ASR Version 1.1");
    create_shader(Vertex_Shader_Source, Fragment_Shader_Source);

    auto triangle = create_geometry(Triangles, Triangle_Geometry_Vertices, Triangle_Geometry_Indices);

    prepare_for_rendering();

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_geometry_current(&triangle);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_geometry(triangle);

    destroy_shader();
    destroy_window();

    return 0;
}
