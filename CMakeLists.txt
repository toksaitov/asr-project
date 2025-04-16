cmake_minimum_required(VERSION 3.25)

project(asr)

set(CMAKE_CXX_STANDARD 17)
set(ASR_SOURCES
    "include/vendor/imgui_impl_opengl.h"
    "include/vendor/imgui_impl_sdl.h"
    "include/math/plane.h"
    "include/math/aabb.h"
    "include/math/sphere.h"
    "include/math/ray.h"
    "include/utilities/utilities.h"
    "include/geometries/vertex.h"
    "include/geometries/geometry.h"
    "include/geometries/es2_geometry.h"
    "include/geometries/geometry_generators.h"
    "include/textures/texture.h"
    "include/textures/es2_texture.h"
    "include/materials/material.h"
    "include/materials/constant_material.h"
    "include/materials/es2_constant_material.h"
    "include/materials/phong_material.h"
    "include/materials/es2_phong_material.h"
    "include/objects/object.h"
    "include/objects/mesh.h"
    "include/objects/camera.h"
    "include/lights/light.h"
    "include/lights/ambient_light.h"
    "include/lights/directional_light.h"
    "include/lights/point_light.h"
    "include/lights/spot_light.h"
    "include/scene/scene.h"
    "include/window/window.h"
    "include/window/es2_sdl_window.h"
    "include/renderer/shader.h"
    "include/renderer/es2_shader.h"
    "include/renderer/renderer.h"
    "include/renderer/es2_renderer.h"
    "include/asr.h"
)
set(ASR_LIBRARIES
    "GLEW::GLEW"
    "SDL2::SDL2"
    "glm::glm"
    "stb::stb"
    "imgui::imgui"
    "SDL2_mixer::SDL2_mixer"
)

find_package(GLEW  REQUIRED)
find_package(SDL2  REQUIRED)
find_package(glm   REQUIRED)
find_package(stb   REQUIRED)
find_package(imgui REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories("./include")

if (WIN32 AND MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(window_test ${ASR_SOURCES} "tests/window_test.cpp")
target_link_libraries(window_test ${ASR_LIBRARIES})

add_executable(triangle_test ${ASR_SOURCES} "tests/triangle_test.cpp")
target_link_libraries(triangle_test ${ASR_LIBRARIES})

add_executable(box_test ${ASR_SOURCES} "tests/box_test.cpp")
target_link_libraries(box_test ${ASR_LIBRARIES})

add_executable(transform_test ${ASR_SOURCES} "tests/transform_test.cpp")
target_link_libraries(transform_test ${ASR_LIBRARIES})

add_executable(scene_graph_test ${ASR_SOURCES} "tests/scene_graph_test.cpp")
target_link_libraries(scene_graph_test ${ASR_LIBRARIES})

add_executable(lighting_test ${ASR_SOURCES} "tests/lighting_test.cpp")
target_link_libraries(lighting_test ${ASR_LIBRARIES})

add_executable(material_properties_test ${ASR_SOURCES} "tests/material_properties_test.cpp")
target_link_libraries(material_properties_test ${ASR_LIBRARIES})

add_executable(textured_plane_test ${ASR_SOURCES} "tests/textured_plane_test.cpp")
target_link_libraries(textured_plane_test ${ASR_LIBRARIES})

add_executable(normal_mapping_test ${ASR_SOURCES} "tests/normal_mapping_test.cpp")
target_link_libraries(normal_mapping_test ${ASR_LIBRARIES})

add_executable(general_usage_test ${ASR_SOURCES} "tests/general_usage_test.cpp")
target_link_libraries(general_usage_test ${ASR_LIBRARIES})
