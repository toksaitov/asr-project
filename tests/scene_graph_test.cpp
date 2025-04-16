#include "asr.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

[[noreturn]] int main()
{
    using namespace asr;

    auto window = std::make_shared<ES2SDLWindow>("asr 2.0", 1280, 720);

    auto[plane_indices, plane_vertices] = geometry_generators::generate_rectangle_geometry_data(0.15f, 0.15f, 2, 2);
    auto plane_geometry = std::make_shared<ES2Geometry>(plane_indices, plane_vertices);

    auto[circle_indices, circle_vertices] = geometry_generators::generate_circle_geometry_data(0.025f, 20);
    auto circle_geometry = std::make_shared<ES2Geometry>(circle_indices, circle_vertices);
    circle_geometry->set_type(Geometry::Type::TriangleFan);

    auto[sphere_indices, sphere_vertices] = geometry_generators::generate_sphere_geometry_data(0.15f, 10, 10);
    auto sphere_geometry = std::make_shared<ES2Geometry>(sphere_indices, sphere_vertices);

    auto seconds_material = std::make_shared<ES2ConstantMaterial>();
    seconds_material->set_emission_color(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    seconds_material->set_face_culling_enabled(false);
    auto quarters_material = std::make_shared<ES2ConstantMaterial>();
    quarters_material->set_emission_color(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    quarters_material->set_face_culling_enabled(false);
    auto hours_material = std::make_shared<ES2ConstantMaterial>();
    hours_material->set_emission_color(glm::vec4{1.0f, 0.5f, 0.5f, 1.0f});
    hours_material->set_face_culling_enabled(false);
    auto hand_material = std::make_shared<ES2ConstantMaterial>();
    hand_material->set_emission_color(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    hand_material->set_face_culling_enabled(false);

    std::vector<std::shared_ptr<Object>> objects;
    for (int i = 0; i < 60; ++i) {
        float angle = static_cast<float>(i) / 60.0f * 2.0f * static_cast<float>(M_PI);
        float x = cosf(angle) * 2.0f;
        float y = sinf(angle) * 2.0f;

        auto second = std::make_shared<Mesh>(circle_geometry, seconds_material, glm::vec3{x, y, 0.0f});
        second->set_name("second");

        objects.push_back(second);
    }
    for (int i = 0; i < 12; ++i) {
        float angle = static_cast<float>(i) / 12.0f * 2.0f * static_cast<float>(M_PI);
        float x = cosf(angle) * 2.15f;
        float y = sinf(angle) * 2.15f;

        auto hours = std::make_shared<Mesh>(plane_geometry, hours_material, glm::vec3{x, y, 0.0f});
        hours->set_scale(glm::vec3{0.4f});
        hours->set_rotation_z(static_cast<float>(M_PI) / 4.0f);
        hours->set_z(0.2f);
        hours->set_name("hours");

        objects.push_back(hours);
    }
    for (int i = 0; i < 4; ++i) {
        float angle = static_cast<float>(i) / 4.0f * 2.0f * static_cast<float>(M_PI);
        float x = cosf(angle) * 2.15f;
        float y = sinf(angle) * 2.15f;

        auto quarter = std::make_shared<Mesh>(plane_geometry, quarters_material, glm::vec3{x, y, 0.0f});
        quarter->set_rotation_z(static_cast<float>(M_PI) / 4.0f);
        quarter->set_z(0.1f);
        quarter->set_name("quarter");

        objects.push_back(quarter);
    }

    auto hand_base = std::make_shared<Mesh>(sphere_geometry, hand_material, glm::vec3{0.0f});
    hand_base->set_name("hand base");
    hand_base->set_scale(glm::vec3{0.5f});
    objects.push_back(hand_base);

    auto hands = std::make_shared<Object>("hands");
    auto hours_hand = std::make_shared<Object>("hours hand");
    auto minutes_hand = std::make_shared<Object>("minutes hand");
    auto seconds_hand = std::make_shared<Object>("seconds hand");
    hands->add_child(hours_hand);
    hands->add_child(minutes_hand);
    hands->add_child(seconds_hand);

    auto hours_hand_line = std::make_shared<Mesh>(plane_geometry, hand_material, glm::vec3{0.6f, 0.0f, 0.0f});
    hours_hand_line->set_name("hours hand line");
    hours_hand_line->set_scale(glm::vec3{8.2f, 0.5f, 1.0f});

    auto minutes_hand_line = std::make_shared<Mesh>(plane_geometry, hand_material, glm::vec3{0.75f, 0.0f, 0.0f});
    minutes_hand_line->set_name("minutes hand line");
    minutes_hand_line->set_scale(glm::vec3{10.0f, 0.25f, 1.0f});

    auto seconds_hand_line = std::make_shared<Mesh>(plane_geometry, hand_material, glm::vec3{0.9f, 0.0f, 0.0f});
    seconds_hand_line->set_name("seconds hand line");
    seconds_hand_line->set_scale(glm::vec3{12.0f, 0.1f, 1.0f});

    hours_hand->add_child(hours_hand_line);
    minutes_hand->add_child(minutes_hand_line);
    seconds_hand->add_child(seconds_hand_line);

    objects.push_back(hands);

    auto scene = std::make_shared<Scene>(objects);
    auto root = scene->get_root();

    auto camera = scene->get_camera();
    camera->set_z(5.0f);

    static const float CAMERA_ROT_SPEED{0.05f};
    static const float CAMERA_SPEED{0.1f};
    static const glm::vec4 FORWARD{ 0.0f, 0.0f, 1.0f, 0.0f};

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
            default: break;
        }
    });

    ES2Renderer renderer(scene, window);

    while (true) {
        window->poll();

        time_t current_time{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
        struct tm *local_time{std::localtime(&current_time)};
        float hours{static_cast<float>(local_time->tm_hour) + static_cast<float>(local_time->tm_min) / 60};
        float minutes{static_cast<float>(local_time->tm_min)};
        float seconds{static_cast<float>(local_time->tm_sec)};

        hours_hand->set_rotation_z((12 - hours) / 12.0f * 2.0f * static_cast<float>(M_PI) + static_cast<float>(M_PI) * 0.5f);
        minutes_hand->set_rotation_z((60 - minutes) / 60.0f * 2.0f * static_cast<float>(M_PI) + static_cast<float>(M_PI) * 0.5f);
        seconds_hand->set_rotation_z((60 - seconds) / 60.0f * 2.0f * static_cast<float>(M_PI) + static_cast<float>(M_PI) * 0.5f);
        root->add_to_rotation_y(0.01f);

        renderer.render();
    }
}
