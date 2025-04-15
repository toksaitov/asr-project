#include "asr.h"

#include <cmath>
#include <string>
#include <utility>
#include <vector>

static const std::string Vertex_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    attribute vec4 position;
    attribute vec4 color;
    attribute vec4 texture_coordinates;

    uniform bool texture_enabled;
    uniform mat4 texture_transformation_matrix;

    uniform mat4 model_view_projection_matrix;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        fragment_color = color;
        if (texture_enabled) {
            vec4 transformed_texture_coordinates = texture_transformation_matrix * vec4(texture_coordinates.st, 0.0, 1.0);
            fragment_texture_coordinates = vec2(transformed_texture_coordinates);
        }

        gl_Position = model_view_projection_matrix * position;
    }
)"};

static const std::string Fragment_Shader_Source{R"( // NOLINT(cert-err58-cpp)
    #version 110

    #define TEXTURING_MODE_ADDITION            0
    #define TEXTURING_MODE_SUBTRACTION         1
    #define TEXTURING_MODE_REVERSE_SUBTRACTION 2
    #define TEXTURING_MODE_MODULATION          3
    #define TEXTURING_MODE_DECALING            4

    uniform bool texture_enabled;
    uniform int texturing_mode;
    uniform sampler2D texture_sampler;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        gl_FragColor = fragment_color;

        if (texture_enabled) {
            if (texturing_mode == TEXTURING_MODE_ADDITION) {
                gl_FragColor += texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_MODULATION) {
                gl_FragColor *= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_DECALING) {
                vec4 texel_color = texture2D(texture_sampler, fragment_texture_coordinates);
                gl_FragColor.rgb = mix(gl_FragColor.rgb, texel_color.rgb, texel_color.a);
            } else if (texturing_mode == TEXTURING_MODE_SUBTRACTION) {
                gl_FragColor -= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_REVERSE_SUBTRACTION) {
                gl_FragColor = texture2D(texture_sampler, fragment_texture_coordinates) - gl_FragColor;
            }
        }
    }
)"};

static asr::GeometryPair generate_sphere_geometry_data(
                             asr::GeometryType geometry_type,
                             float radius,
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

    for (auto i = 0U; i <= height_segments_count; ++i) {
        float v{static_cast<float>(i) / static_cast<float>(height_segments_count)};
        float phi{v * asr::pi};

        for (auto j = 0U; j <= width_segments_count; ++j) {
            float u{static_cast<float>(j) / static_cast<float>(width_segments_count)};
            float theta{u * asr::two_pi};

            float cos_phi{std::cos(phi)};
            float sin_phi{std::sin(phi)};
            float cos_theta{std::cos(theta)};
            float sin_theta{std::sin(theta)};

            float x{cos_theta * sin_phi};
            float y{cos_phi};
            float z{sin_phi * sin_theta};

            vertices.push_back(asr::Vertex{
                x * radius, y * radius, z * radius,
                x, y, z,
                color.r, color.g, color.b, color.a,
                1.0f - u, v
            });
            if (geometry_type == asr::GeometryType::Points) {
                indices.push_back(vertices.size() - 1);
            }
        }
    }

    if (geometry_type == asr::GeometryType::Lines || geometry_type == asr::GeometryType::Triangles) {
        for (auto rows = 0U; rows < height_segments_count; ++rows) {
            for (auto columns = 0U; columns < width_segments_count; ++columns) {
                unsigned int index_a{rows * (width_segments_count + 1) + columns};
                unsigned int index_b{index_a + 1};
                unsigned int index_c{index_a + (width_segments_count + 1)};
                unsigned int index_d{index_c + 1};
                if (geometry_type == asr::GeometryType::Lines) {
                    if (rows != 0) {
                        indices.insert(indices.end(), {
                            index_a, index_b, index_b, index_c, index_c, index_a
                        });
                    }
                    if (rows != height_segments_count - 1) {
                        indices.insert(indices.end(), {
                            index_b, index_d, index_d, index_c, index_c, index_b
                        });
                    }
                } else {
                    if (rows != 0) {
                        indices.insert(indices.end(), {index_a, index_b, index_c});
                    }
                    if (rows != height_segments_count - 1) {
                        indices.insert(indices.end(), {index_b, index_d, index_c});
                    }
                }
            }
        }
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(1280U, 720U, "Transformation Test on ASR Version 1.3");

    auto material = create_material(Vertex_Shader_Source, Fragment_Shader_Source);

    float radius{0.5f};
    unsigned int width_segments{20U}, height_segments{20U};

    auto [geometry_vertices, geometry_indices] = generate_sphere_geometry_data(Triangles, radius, width_segments, height_segments);
    auto geometry = create_geometry(Triangles, geometry_vertices, geometry_indices);

    bool generate_mipmaps = true;
    auto sun_image     = read_image_file("data/images/sun.jpg");
    auto sun_texture   = create_texture(sun_image, generate_mipmaps);
    auto venus_image   = read_image_file("data/images/venus.jpg");
    auto venus_texture = create_texture(venus_image, generate_mipmaps);
    auto earth_image   = read_image_file("data/images/earth.jpg");
    auto earth_texture = create_texture(earth_image, generate_mipmaps);
    auto moon_image    = read_image_file("data/images/moon.jpg");
    auto moon_texture  = create_texture(moon_image, generate_mipmaps);

    prepare_for_rendering();

    set_material_current(&material);
    set_material_face_culling_enabled(true);
    set_material_depth_test_enabled(true);

    static const float CAMERA_SPEED{6.0f};
    static const float CAMERA_ROT_SPEED{1.5f};
    static const float CAMERA_FOV{1.13f};
    static const float CAMERA_NEAR_PLANE{0.1f};
    static const float CAMERA_FAR_PLANE{100.0f};

    glm::vec3 camera_position{0.0f, 3.23f, 6.34f};
    glm::vec3 camera_rotation{-0.6f, 0.0f, 0.0f};
    set_keys_down_event_handler([&](const uint8_t *keys) {
        if (keys[SDL_SCANCODE_ESCAPE]) exit(0);
        if (keys[SDL_SCANCODE_W]) camera_rotation.x -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_A]) camera_rotation.y += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_S]) camera_rotation.x += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_D]) camera_rotation.y -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_UP]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position -= shift;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position += shift;
        }
    });
    set_matrix_mode(Projection);
    load_perspective_projection_matrix(CAMERA_FOV, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

    float sun_rotation{0.0f};
    float sun_delta_angle{0.2f};
    float sun_size{2.0f};

    float venus_rotation{0.0f};
    float venus_delta_angle{-0.8f};
    float venus_sun_rotation{0.0f};
    float venus_sun_delta_angle{-0.1f};
    float venus_sun_distance{3.0f};
    float venus_size{0.42f};

    float earth_rotation{0.0f};
    float earth_delta_angle{-0.8f};
    float earth_sun_rotation{0.0f};
    float earth_sun_delta_angle{0.5f};
    float earth_sun_distance{5.0f};
    float earth_size{0.4f};

    float moon_rotation{0.0f};
    float moon_delta_angle{2.6f};
    float moon_earth_rotation{0.0f};
    float moon_earth_delta_angle{1.0f};
    float moon_earth_distance{0.5f};
    float moon_size{0.15f};

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_matrix_mode(View);
        load_identity_matrix();
        translate_matrix(camera_position);
        rotate_matrix(camera_rotation);

        set_matrix_mode(Model);

        // Sun

        load_identity_matrix();
        rotate_matrix(glm::vec3{0.0f, sun_rotation, 0.0f});
        sun_rotation += sun_delta_angle * get_dt();
        scale_matrix(glm::vec3{sun_size});

        set_texture_current(&sun_texture);
        set_geometry_current(&geometry);
        render_current_geometry();

        // Venus

        load_identity_matrix();
        rotate_matrix(glm::vec3{0.0f, venus_sun_rotation, 0.0f});
        translate_matrix(glm::vec3{venus_sun_distance, 0.0f, 0.0f});
        rotate_matrix(glm::vec3{0.0f, -venus_sun_rotation, 0.0f});
        venus_sun_rotation += venus_sun_delta_angle * get_dt();
        rotate_matrix(glm::vec3{0.0f, venus_rotation, 0.0f});
        venus_rotation += venus_delta_angle * get_dt();
        scale_matrix(glm::vec3{venus_size});

        set_texture_current(&venus_texture);
        set_geometry_current(&geometry);
        render_current_geometry();

        // Earth

        load_identity_matrix();
        rotate_matrix(glm::vec3{0.0f, earth_sun_rotation, 0.0f});
        translate_matrix(glm::vec3{earth_sun_distance, 0.0f, 0.0f});
        rotate_matrix(glm::vec3{0.0f, -earth_sun_rotation, 0.0f});
        earth_sun_rotation += earth_sun_delta_angle * get_dt();

        push_matrix();
        rotate_matrix(glm::vec3{0.0f, earth_rotation, 0.0f});
        earth_rotation += earth_delta_angle * get_dt();
        scale_matrix(glm::vec3{earth_size});

        set_texture_current(&earth_texture);
        set_geometry_current(&geometry);
        render_current_geometry();

        // Moon

        pop_matrix();
        rotate_matrix(glm::vec3{0.0f, moon_earth_rotation, 0.0f});
        translate_matrix(glm::vec3{moon_earth_distance, 0.0f, 0.0f});
        rotate_matrix(glm::vec3{0.0f, -moon_earth_rotation, 0.0f});
        moon_earth_rotation += moon_earth_delta_angle * get_dt();
        rotate_matrix(glm::vec3{0.0f, moon_rotation, 0.0f});
        moon_rotation += moon_delta_angle * get_dt();
        scale_matrix(glm::vec3{moon_size});

        set_texture_current(&moon_texture);
        set_geometry_current(&geometry);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_texture(moon_texture);
    destroy_texture(earth_texture);
    destroy_texture(venus_texture);
    destroy_texture(sun_texture);

    destroy_geometry(geometry);

    destroy_material(material);

    destroy_window();

    return 0;
}
