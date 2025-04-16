#include "asr.h"

[[noreturn]] int main()
{
    using namespace asr;

    auto window = std::make_shared<ES2SDLWindow>("asr 2.0", 1280, 720);

    auto[box_indices, box_vertices] = geometry_generators::generate_box_geometry_data(5.0f, 5.0f, 5.0f, 5, 5, 5);
    auto box_geometry = std::make_shared<ES2Geometry>(box_indices, box_vertices);
    auto box_material = std::make_shared<ES2PhongMaterial>();
    box_material->set_face_culling_enabled(false);
    auto[image1_data, image1_width, image1_height, image1_channels] = file_utilities::read_image_file("data/images/room_cubemap.png");
    auto[image1_normals_data, image1_normals_width, image1_normals_height, image1_normals_channels] = file_utilities::read_image_file("data/images/room_normalmap.png");
    auto box_texture1 = std::make_shared<ES2Texture>(image1_data, image1_width, image1_height, image1_channels);
    auto box_texture1_normals = std::make_shared<ES2Texture>(image1_normals_data, image1_normals_width, image1_normals_height, image1_normals_channels);
    box_material->set_texture_1(box_texture1);
    box_material->set_texture_1_normals(box_texture1_normals);
    box_material->set_ambient_color(glm::vec3{0.1f});
    auto box = std::make_shared<Mesh>(box_geometry, box_material);

    auto[lamp_indices, lamp_vertices] = geometry_generators::generate_sphere_geometry_data(0.05f, 20, 20);
    auto lamp_sphere_geometry = std::make_shared<ES2Geometry>(lamp_indices, lamp_vertices);
    auto lamp_material = std::make_shared<ES2ConstantMaterial>();
    lamp_material->set_emission_color(glm::vec4(1.0f));
    auto lamp = std::make_shared<Mesh>(lamp_sphere_geometry, lamp_material);

    std::vector<std::shared_ptr<Object>> objects{box};
    auto scene = std::make_shared<Scene>(objects);

    auto point_light = std::make_shared<PointLight>();
    point_light->set_intensity(2.45f);
    point_light->set_y(-0.5f);
    point_light->set_two_sided(true);
    point_light->add_child(lamp);
    scene->get_root()->add_child(point_light);
    scene->get_point_lights().push_back(point_light);

    auto camera = scene->get_camera();
    camera->set_position(glm::vec3{1.12f, 0.88f, 1.4f});
    camera->set_rotation(glm::vec3{-0.5f, 0.7f, 0.0f});

    static const float CAMERA_SPEED{0.1f};
    static const float CAMERA_ROT_SPEED{0.05f};
    static const glm::vec4 FORWARD{0.0f, 0.0f, 1.0f, 0.0f};
    window->set_on_key_down([&](int key) {
        switch (key) {
            case SDLK_w:
                camera->add_to_rotation_x(-CAMERA_ROT_SPEED);
                break;
            case SDLK_a:
                camera->add_to_rotation_y(CAMERA_ROT_SPEED);
                break;
            case SDLK_s:
                camera->add_to_rotation_x(CAMERA_ROT_SPEED);
                break;
            case SDLK_d:
                camera->add_to_rotation_y(-CAMERA_ROT_SPEED);
                break;
            case SDLK_e:
                camera->add_to_y(CAMERA_ROT_SPEED);
                break;
            case SDLK_q:
                camera->add_to_y(-CAMERA_ROT_SPEED);
                break;
            case SDLK_UP:
                camera->add_to_position(-glm::vec3(camera->get_model_matrix() * FORWARD * CAMERA_SPEED));
                break;
            case SDLK_DOWN:
                camera->add_to_position(glm::vec3(camera->get_model_matrix() * FORWARD * CAMERA_SPEED));
                break;
            case SDLK_ESCAPE: exit(0);
            default: break;
        }
    });

    float point_light_angle = 0.0f;
    float point_light_radius = 1.0f;

    ES2Renderer renderer(scene, window);
    for (;;) {
        window->poll();

        point_light->set_x(cosf(point_light_angle) * point_light_radius);
        point_light->set_z(sinf(point_light_angle) * point_light_radius);
        point_light_angle += 0.01f;

        renderer.render();
    }
}
