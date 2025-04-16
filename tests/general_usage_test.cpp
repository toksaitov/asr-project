#include "asr.h"

#include <utility>
#include <memory>
#include <tuple>
#include <chrono>

#include "SDL_mixer.h"

using namespace asr;

static const float CAMERA_SPEED{0.5f};
static const float CAMERA_SENSITIVITY{0.001f};

const glm::vec4 FORWARD{0.0f, 0.0f, 1.0f, 0.0f};
const glm::vec4 RIGHT{1.0f, 0.0f, 0.0f, 0.0f};

class Enemy {
public:
    typedef std::tuple<const std::string, unsigned int, unsigned int> enemy_sprite_data_type;

    enum State {
        Alive,
        Dying,
        Dead
    };

    Enemy(const glm::vec3 &position, float size, float speed, const enemy_sprite_data_type& enemy_sprite_data)
        : _position{position}, _speed(speed)
    {
        const auto&[sprite_file, sprite_frame_count, first_dying_state_sprite_frame] = enemy_sprite_data;

        auto[image_data, image_width, image_height, image_channels] = file_utilities::read_image_file(sprite_file);
        _texture = std::make_shared<ES2Texture>(image_data, image_width, image_height, image_channels);
        _texture->set_minification_filter(Texture::FilterType::Nearest);
        _texture->set_magnification_filter(Texture::FilterType::Nearest);
        _texture->set_mode(Texture::Mode::Modulation);
        _texture->set_transformation_enabled(true);
        _set_texture_frames(sprite_frame_count);
        _set_first_dying_texture_frame(first_dying_state_sprite_frame);

        auto[billboard_indices, billboard_vertices] = geometry_generators::generate_rectangle_geometry_data(size, size, 1, 1);
        auto billboard_geometry = std::make_shared<ES2Geometry>(billboard_indices, billboard_vertices);
        auto billboard_material = std::make_shared<ES2ConstantMaterial>();
        billboard_material->set_texture_1(_texture);
        billboard_material->set_blending_enabled(true);
        billboard_material->set_face_culling_enabled(false);
        billboard_material->set_transparent(true);

        _mesh = std::make_shared<Mesh>(billboard_geometry, billboard_material);
        _mesh->set_position(position);
        _bounding_volume = Sphere{_mesh->get_position(), size / 2};
    }

    [[nodiscard]] const std::shared_ptr<Mesh> &get_mesh() const
    {
        return _mesh;
    }

    void set_target(const std::shared_ptr<Camera> &target)
    {
        _target = target;
    }

    void set_update_rate(int update_rate)
    {
        _update_rate = update_rate;
    }

    void update(float delta_time)
    {
        if (_state == Dying) {
            if (_update_request++ % _update_rate != 0) {
                return;
            }

            unsigned int frame = _texture_frame + 1;
            if (frame >= _texture_frames) {
                _state = Dead;
            } else {
                _set_texture_frame(frame);
            }
        } else if (_state == Alive) {
            if (_update_request++ % _update_rate != 0 || _target == nullptr) {
                return;
            }

            _set_texture_frame((_texture_frame + 1) % _first_dying_texture_frame);

            glm::vec3 target = _target->get_world_position();
            glm::vec3 position = _position;
            target.y = position.y;

            _velocity = glm::normalize(target - position) * _speed * delta_time;
            _position += _velocity * delta_time;
            _mesh->set_position(_position);
            _bounding_volume.set_center(_position);
        }

        _mesh->billboard_toward_camera(_target);
    }

    [[nodiscard]] bool intersects_with_ray(const Ray &ray) const
    {
        return ray.intersects_with_sphere(_bounding_volume).first;
    }

    void kill()
    {
        if (_state == Alive) {
            _state = Dying;
            _set_texture_frame(_first_dying_texture_frame);
        }
    }

private:
    State _state{Alive};

    glm::vec3 _position;
    float _speed{1.0f};
    glm::vec3 _velocity{0.0f};

    std::shared_ptr<Mesh> _mesh;
    Sphere _bounding_volume{glm::vec3{0.0f}, 1.0f};

    std::shared_ptr<Camera> _target{nullptr};

    int _update_request{0};
    int _update_rate{10};

    std::shared_ptr<ES2Texture> _texture;
    unsigned int _texture_frame{0};
    unsigned int _texture_frames{1};
    unsigned int _first_dying_texture_frame{0};

    void _set_texture_frame(unsigned int texture_frame)
    {
        _texture_frame = texture_frame;
        glm::mat4 matrix = _texture->get_transformation_matrix();
        matrix[3][0] = static_cast<float>(_texture_frame) / static_cast<float>(_texture_frames);
        _texture->set_transformation_matrix(matrix);
    }

    void _set_texture_frames(unsigned int texture_frames)
    {
        _texture_frames = texture_frames;
        glm::mat4 matrix = _texture->get_transformation_matrix();
        matrix[0][0] = 1.0f / static_cast<float>(_texture_frames);
        matrix[3][0] = static_cast<float>(_texture_frame) / static_cast<float>(_texture_frames);
        _texture->set_transformation_matrix(matrix);
    }

    void _set_first_dying_texture_frame(unsigned int first_dying_texture_frame)
    {
        _first_dying_texture_frame = first_dying_texture_frame;
    }
};

class Gun {
public:
    typedef std::tuple<const std::string, unsigned int> gun_sprite_data_type;

    enum State {
        Idling,
        Shooting
    };

    Gun(const glm::vec3 &position, float gun_size, const glm::vec2 &target, const gun_sprite_data_type& gun_sprite_data) : _target{target}
    {
        const auto&[sprite_file, sprite_frame_count] = gun_sprite_data;

        auto[image2_data, image2_width, image2_height, image2_channels] = file_utilities::read_image_file(sprite_file);
        _texture = std::make_shared<ES2Texture>(image2_data, image2_width, image2_height, image2_channels);
        _texture->set_minification_filter(Texture::FilterType::Nearest);
        _texture->set_magnification_filter(Texture::FilterType::Nearest);
        _texture->set_mode(Texture::Mode::Modulation);
        _texture->set_transformation_enabled(true);
        _set_texture_frames(sprite_frame_count);

        auto[overlay_indices, overlay_vertices] = geometry_generators::generate_rectangle_geometry_data(2, 2, 1, 1);
        auto overlay_geometry = std::make_shared<ES2Geometry>(overlay_indices, overlay_vertices);
        auto overlay_material = std::make_shared<ES2ConstantMaterial>();
        overlay_material->set_texture_1(_texture);
        overlay_material->set_blending_enabled(true);
        overlay_material->set_overlay(true);

        _mesh = std::make_shared<Mesh>(overlay_geometry, overlay_material);
        _mesh->set_position(position);
        _mesh->set_scale(glm::vec3(gun_size));
    }

    [[nodiscard]] const std::shared_ptr<Mesh> &get_mesh() const
    {
        return _mesh;
    }

    void set_point_of_view(const std::shared_ptr<Camera> &point_of_view)
    {
        _point_of_view = point_of_view;
    }

    void set_update_rate(int update_rate)
    {
        _update_rate = update_rate;
    }

    void update()
    {
        if (_state == Shooting) {
            if (_update_request++ % _update_rate != 0) {
                return;
            }

            unsigned int frame = _texture_frame + 1;
            if (frame >= _texture_frames) {
                _state = Idling;
                _set_texture_frame(0);
            } else {
                _set_texture_frame(frame);
            }
        }
    }

    void shoot(const std::vector<std::shared_ptr<Enemy>> &enemies)
    {
        if (_state == Idling) {
            _state = Shooting;

            if (_point_of_view == nullptr) {
                return;
            }

            for (auto &enemy : enemies) {
                if (enemy->intersects_with_ray(_point_of_view->world_ray_from_screen_point(
                        static_cast<int>(_target.x),
                        static_cast<int>(_target.y)
                    ))) {
                    enemy->kill();
                    return;
                }
            }
        }
    }

private:
    State _state{Idling};

    std::shared_ptr<Mesh> _mesh;
    std::shared_ptr<Camera> _point_of_view;
    glm::vec2 _target;

    int _update_request{0};
    int _update_rate{7};

    std::shared_ptr<ES2Texture> _texture;
    unsigned int _texture_frame{0};
    unsigned int _texture_frames{1};

    void _set_texture_frame(unsigned int texture_frame)
    {
        _texture_frame = texture_frame;
        glm::mat4 matrix = _texture->get_transformation_matrix();
        matrix[3][0] = static_cast<float>(_texture_frame) / static_cast<float>(_texture_frames);
        _texture->set_transformation_matrix(matrix);
    }

    void _set_texture_frames(unsigned int texture_frames)
    {
        _texture_frames = texture_frames;

        glm::mat4 matrix = _texture->get_transformation_matrix();
        matrix[0][0] = 1.0f / static_cast<float>(_texture_frames);
        matrix[3][0] = static_cast<float>(_texture_frame) / static_cast<float>(_texture_frames);
        _texture->set_transformation_matrix(matrix);
    }
};

/* Example, how you can start abstracting gameplay elements...
class Player {
public:

private:
    glm::vec3 _position;

    float _speed{1.0f};
    glm::vec3 _velocity{0.0f};

    std::shared_ptr<Mesh> _mesh;
    Sphere _bounding_volume{glm::vec3{0.0f}, 1.0f};

    std::shared_ptr<Gun> _gun;
};
*/

[[noreturn]]
int main(int argc, char **argv)
{
    // Window

    auto window = std::make_shared<ES2SDLWindow>("asr 2.0", 1280, 720);
    window->set_capture_mouse_enabled(true);
    window->set_relative_mouse_mode_enabled(true);

    // Room Ground

    auto[room_ground_indices, room_ground_vertices] = geometry_generators::generate_rectangle_geometry_data(50, 50, 1, 1);
    auto room_ground_geometry = std::make_shared<ES2Geometry>(room_ground_indices, room_ground_vertices);
    auto room_ground_material = std::make_shared<ES2PhongMaterial>();
    room_ground_material->set_specular_exponent(1.0f);
    room_ground_material->set_specular_color(glm::vec3{0.0f});
    room_ground_material->set_diffuse_color(glm::vec4{0.25f});
    auto room_ground = std::make_shared<Mesh>(room_ground_geometry, room_ground_material);
    room_ground->set_position(glm::vec3(0.0f, -2.5f, 0.0f));
    room_ground->set_rotation(glm::vec3(-M_PI / 2.0f, 0.0f, 0.0f));

    // Monsters

    float enemies_size = 9;
    float enemies_speed = 10.01f;
    int enemies_sprite_frames = 10;
    int enemies_dying_first_sprite_frame = 4;
    std::tuple enemies_sprite_data =
        std::make_tuple(
            "data/images/cacodemon.png",
            enemies_sprite_frames,
            enemies_dying_first_sprite_frame
        );

    glm::vec3 enemy1_position{-5.0f, 1.5f, 0.0f};
    auto enemy1 = std::make_shared<Enemy>(enemy1_position, enemies_size, enemies_speed, enemies_sprite_data);

    glm::vec3 enemy2_position{5.0f, 1.5f, 0.0f};
    auto enemy2 = std::make_shared<Enemy>(enemy2_position, enemies_size, enemies_speed, enemies_sprite_data);

    std::vector<std::shared_ptr<Enemy>> enemies{enemy1, enemy2};

    // Gun

    glm::vec3 gun_position{0.0f, -1.0f, 0.0f};
    float gun_size = 2.0f;
    glm::vec2 gun_target{window->get_width() / 2, window->get_height() / 2};
    int gun_sprite_frames = 6;
    std::tuple gun_sprite_data = std::make_tuple("data/images/gun.png", gun_sprite_frames);

    auto gun = std::make_shared<Gun>(gun_position, gun_size, gun_target, gun_sprite_data);

    // Lamps

    auto[lamp_indices, lamp_vertices] = geometry_generators::generate_sphere_geometry_data(0.2f, 20, 20);
    auto lamp_sphere_geometry = std::make_shared<ES2Geometry>(lamp_indices, lamp_vertices);
    auto lamp_material = std::make_shared<ES2ConstantMaterial>();
    auto lamp1 = std::make_shared<Mesh>(lamp_sphere_geometry, lamp_material);
    auto lamp2 = std::make_shared<Mesh>(lamp_sphere_geometry, lamp_material);
    auto lamp3 = std::make_shared<Mesh>(lamp_sphere_geometry, lamp_material);
    auto lamp4 = std::make_shared<Mesh>(lamp_sphere_geometry, lamp_material);

    std::vector<std::shared_ptr<Object>> objects{room_ground, enemy1->get_mesh(), enemy2->get_mesh(), gun->get_mesh()};
    auto scene = std::make_shared<Scene>(objects);

    // Point Lights

    auto point_light1 = std::make_shared<PointLight>();
    point_light1->set_intensity(4000.0f);
    point_light1->set_constant_attenuation(0.0f);
    point_light1->set_linear_attenuation(0.2f);
    point_light1->set_quadratic_attenuation(0.8f);
    point_light1->set_two_sided(true);
    point_light1->set_position(glm::vec3(-25.0f, 4.0f, 25.0f));
    point_light1->add_child(lamp1);
    scene->get_root()->add_child(point_light1);
    scene->get_point_lights().push_back(point_light1);

    auto point_light2 = std::make_shared<PointLight>();
    point_light2->set_intensity(4000.0f);
    point_light2->set_constant_attenuation(0.0f);
    point_light2->set_linear_attenuation(0.2f);
    point_light2->set_quadratic_attenuation(0.8f);
    point_light2->set_two_sided(true);
    point_light2->set_position(glm::vec3(25.0f, 4.0f, 25.0f));
    point_light2->add_child(lamp2);
    scene->get_root()->add_child(point_light2);
    scene->get_point_lights().push_back(point_light2);

    auto point_light3 = std::make_shared<PointLight>();
    point_light3->set_intensity(4000.0f);
    point_light3->set_constant_attenuation(0.0f);
    point_light3->set_linear_attenuation(0.2f);
    point_light3->set_quadratic_attenuation(0.8f);
    point_light3->set_two_sided(true);
    point_light3->set_position(glm::vec3(25.0f, 4.0f, -25.0f));
    point_light3->add_child(lamp3);
    scene->get_root()->add_child(point_light3);
    scene->get_point_lights().push_back(point_light3);

    auto point_light4 = std::make_shared<PointLight>();
    point_light4->set_intensity(4000.0f);
    point_light4->set_constant_attenuation(0.0f);
    point_light4->set_linear_attenuation(0.2f);
    point_light4->set_quadratic_attenuation(0.8f);
    point_light4->set_two_sided(true);
    point_light4->set_position(glm::vec3(-25.0f, 4.0f, -25.0f));
    point_light4->add_child(lamp4);
    scene->get_root()->add_child(point_light4);
    scene->get_point_lights().push_back(point_light4);

    // Camera

    auto camera = scene->get_camera();
    camera->set_position(glm::vec3(0.0f, 0.0f, 20.0f));
    camera->set_zoom(3.0f);

    enemy1->set_target(camera);
    enemy2->set_target(camera);
    gun->set_point_of_view(camera);

    // Music

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        printf("Mix_OpenAudio(...): %s\n", Mix_GetError());
        // this might be a critical error... up to you
    }
    Mix_Chunk *shotgun_sound = Mix_LoadWAV("data/audio/shotgun.wav");
    if(!shotgun_sound) {
        printf("Mix_LoadWAV(\"data/audio/shotgun.wav\"): %s\n", Mix_GetError());
        // this might be a critical error... up to you
    }
    Mix_Music *music = Mix_LoadMUS("data/audio/E1M1.ogg");
    if(!music) {
        printf("Mix_LoadMUS(\"data/audio/E1M1.ogg\"): %s\n", Mix_GetError());
        // this might be a critical error... up to you
    }
    Mix_PlayMusic(music, -1);

    // Input

    window->set_on_late_keys_down([&](const uint8_t *keys) {
        glm::mat4 model_matrix = camera->get_model_matrix();

        if (keys[SDL_SCANCODE_W]) {
            camera->add_to_position(-glm::vec3(model_matrix * FORWARD * CAMERA_SPEED));
        }
        if (keys[SDL_SCANCODE_A]) {
            camera->add_to_position(-glm::vec3(model_matrix * RIGHT * CAMERA_SPEED));
        }
        if (keys[SDL_SCANCODE_S]) {
            camera->add_to_position(glm::vec3(model_matrix * FORWARD * CAMERA_SPEED));
        }
        if (keys[SDL_SCANCODE_D]) {
            camera->add_to_position(glm::vec3(model_matrix * RIGHT * CAMERA_SPEED));
        }
    });

    window->set_on_mouse_down([&](int button, int x, int y) {
        Mix_PlayChannel(-1, shotgun_sound, 0);
        gun->shoot(enemies);
    });

    window->set_on_mouse_move([&](int x, int y, int x_rel, int y_rel) {
        camera->add_to_rotation_y(static_cast<float>(-x_rel) * CAMERA_SENSITIVITY);
    });

    // Rendering

    auto prev_frame_time = std::chrono::high_resolution_clock::now();

    ES2Renderer renderer(scene, window);
    while (true) {
        window->poll();

        auto current_frame_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> time_span = current_frame_time - prev_frame_time;
        float delta_time = time_span.count() / 1000.0f;

        for (auto &enemy : enemies) {
            enemy->update(delta_time);
        }
        gun->update();

        renderer.render();

        prev_frame_time = current_frame_time;
    }
}
