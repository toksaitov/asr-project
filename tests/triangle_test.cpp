#include "asr.h"

[[noreturn]] int main()
{
    using namespace asr;

    auto window = std::make_shared<ES2SDLWindow>("asr 2.0", 1280, 720);

    auto[triangle_indices, triangle_vertices] = geometry_generators::generate_triangle_geometry_data(4.0f);
    auto triangle_geometry = std::make_shared<ES2Geometry>(triangle_indices, triangle_vertices);

    auto &vertices = triangle_geometry->get_vertices();
    vertices[0].color = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
    vertices[1].color = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
    vertices[2].color = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};

    auto triangle_material = std::make_shared<ES2ConstantMaterial>();
    triangle_material->set_face_culling_enabled(false);
    auto triangle = std::make_shared<Mesh>(triangle_geometry, triangle_material);

    std::vector<std::shared_ptr<Object>> objects{triangle};
    auto scene = std::make_shared<Scene>(objects);
    auto camera = scene->get_camera();
    camera->set_z(5.0f);

    ES2Renderer renderer(scene, window);
    while (true) {
        window->poll();

        triangle->add_to_rotation_z(0.01f); // no dt in this version ;( handle it yourself

        renderer.render();
    }
}
