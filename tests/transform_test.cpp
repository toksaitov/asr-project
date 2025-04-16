#include "asr.h"

[[noreturn]] int main()
{
    using namespace asr;

    auto window = std::make_shared<ES2SDLWindow>("asr 2.0", 1280, 720);

    auto [sun_image, sun_image_width, sun_image_height, sun_image_depth]         = file_utilities::read_image_file("data/images/sun.jpg");
    auto [venus_image, venus_image_width, venus_image_height, venus_image_depth] = file_utilities::read_image_file("data/images/venus.jpg");
    auto [earth_image, earth_image_width, earth_image_height, earth_image_depth] = file_utilities::read_image_file("data/images/earth.jpg");
    auto [moon_image, moon_image_width,  moon_image_height,  moon_image_depth]   = file_utilities::read_image_file("data/images/moon.jpg");

    auto sun_texture   = std::make_shared<ES2Texture>(sun_image, sun_image_width, sun_image_height, sun_image_depth);
    auto venus_texture = std::make_shared<ES2Texture>(venus_image, venus_image_width, venus_image_height, venus_image_depth);
    auto earth_texture = std::make_shared<ES2Texture>(earth_image, earth_image_width, earth_image_height, earth_image_depth);
    auto moon_texture  = std::make_shared<ES2Texture>(moon_image, moon_image_width, moon_image_height, moon_image_depth);

    auto sun_material   = std::make_shared<ES2ConstantMaterial>();
    auto venus_material = std::make_shared<ES2ConstantMaterial>();
    auto earth_material = std::make_shared<ES2ConstantMaterial>();
    auto moon_material  = std::make_shared<ES2ConstantMaterial>();

    sun_material->set_texture_1(sun_texture);
    venus_material->set_texture_1(venus_texture);
    earth_material->set_texture_1(earth_texture);
    moon_material->set_texture_1(moon_texture);

    auto [sphere_indices, sphere_vertices] = geometry_generators::generate_sphere_geometry_data(1.0f, 30U, 30U);
    auto sphere_geometry = std::make_shared<ES2Geometry>(sphere_indices, sphere_vertices);

    auto sun_mesh   = std::make_shared<Mesh>(sphere_geometry, sun_material);
    auto venus_mesh = std::make_shared<Mesh>(sphere_geometry, venus_material);
    auto earth_mesh = std::make_shared<Mesh>(sphere_geometry, earth_material);
    auto moon_mesh  = std::make_shared<Mesh>(sphere_geometry, moon_material);

    sun_mesh->set_scale(glm::vec3{1.0f});
    venus_mesh->set_scale(glm::vec3{0.21f});
    earth_mesh->set_scale(glm::vec3{0.2f});
    moon_mesh->set_scale(glm::vec3{0.08f});

    auto sun = std::make_shared<Object>();
    auto venus_rotator = std::make_shared<Object>();
    auto earth_rotator = std::make_shared<Object>();
    auto earth = std::make_shared<Object>();
    auto moon_rotator = std::make_shared<Object>();

    sun->add_child(sun_mesh);
    sun->add_child(venus_rotator);
    sun->add_child(earth_rotator);
    earth->add_child(earth_mesh);
    earth->add_child(moon_rotator);
    venus_rotator->add_child(venus_mesh);
    earth_rotator->add_child(earth);
    moon_rotator->add_child(moon_mesh);

    venus_mesh->set_x(3.0f);
    earth->set_x(5.0f);
    moon_mesh->set_x(0.5f);

    std::vector<std::shared_ptr<Object>> objects{sun};
    auto scene = std::make_shared<Scene>(objects);

    auto camera = scene->get_camera();
    camera->set_position(glm::vec3{0.0f, 3.23f, 6.34f});
    camera->set_rotation(glm::vec3{-0.6f, 0.0f, 0.0f});

    static const float CAMERA_SPEED{0.1f};
    static const float CAMERA_ROT_SPEED{0.01f};
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
            case SDLK_ESCAPE:
                exit(0);
            default:
                break;
        }
    });

    ES2Renderer renderer(scene, window);
    while (true) {
        window->poll();

        sun_mesh->add_to_rotation_y(0.015f);
        venus_mesh->add_to_rotation_y(-0.01f);
        earth_mesh->add_to_rotation_y(0.02f);
        moon_mesh->add_to_rotation_y(-0.06f);

        venus_rotator->add_to_rotation_y(-0.005f);
        earth_rotator->add_to_rotation_y(0.005f);
        moon_rotator->add_to_rotation_y(0.01f);

        renderer.render();
    }
}
