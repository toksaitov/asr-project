#include "asr.h"

#include <string>
#include <vector>

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

static const asr::Vertices Triangle_Geometry_Vertices = { // NOLINT(cert-err58-cpp)
    //           Position             Normal            Color (RGBA)            Texture Coordinates (UV)
    asr::Vertex{ 0.5f,   0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  0.5f },
    asr::Vertex{-0.25f,  0.43f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.25f, 0.07f},
    asr::Vertex{-0.25f, -0.43f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f, 0.93f}
};
static const asr::Indices Triangle_Geometry_Indices = { // NOLINT(cert-err58-cpp)
    0U, 1U, 2U
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500U, 500U, "Hello World Test on ASR Version 1.3");

    auto material = create_material(Vertex_Shader_Source, Fragment_Shader_Source);
    auto geometry = create_geometry(Triangles, Triangle_Geometry_Vertices, Triangle_Geometry_Indices);

    prepare_for_rendering();

    set_material_current(&material);
    set_geometry_current(&geometry);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_geometry(geometry);
    destroy_material(material);

    destroy_window();

    return 0;
}
