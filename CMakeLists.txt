cmake_minimum_required(VERSION 3.25)

project(asr)

set(CMAKE_CXX_STANDARD 17)
set(ASR_SOURCES "include/asr.h")
set(ASR_LIBRARIES "GLEW::GLEW" "SDL2::SDL2" "glm::glm" "stb::stb")

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm  REQUIRED)
find_package(stb  REQUIRED)
include_directories("./include")

if (WIN32 AND MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(hello_world_test ${ASR_SOURCES} "tests/hello_world_test.cpp")
target_link_libraries(hello_world_test ${ASR_LIBRARIES})

add_executable(circle_test ${ASR_SOURCES} "tests/circle_test.cpp")
target_link_libraries(circle_test ${ASR_LIBRARIES})

add_executable(ring_test ${ASR_SOURCES} "tests/ring_test.cpp")
target_link_libraries(ring_test ${ASR_LIBRARIES})

add_executable(rectangle_test ${ASR_SOURCES} "tests/rectangle_test.cpp")
target_link_libraries(rectangle_test ${ASR_LIBRARIES})

add_executable(sphere_test ${ASR_SOURCES} "tests/sphere_test.cpp")
target_link_libraries(sphere_test ${ASR_LIBRARIES})

add_executable(box_test ${ASR_SOURCES} "tests/box_test.cpp")
target_link_libraries(box_test ${ASR_LIBRARIES})

add_executable(triangle_test ${ASR_SOURCES} "tests/triangle_test.cpp")
target_link_libraries(triangle_test ${ASR_LIBRARIES})

add_executable(transform1_test ${ASR_SOURCES} "tests/transform1_test.cpp")
target_link_libraries(transform1_test ${ASR_LIBRARIES})

add_executable(transform2_test ${ASR_SOURCES} "tests/transform2_test.cpp")
target_link_libraries(transform2_test ${ASR_LIBRARIES})

add_executable(shader_test ${ASR_SOURCES} "tests/shader_test.cpp")
target_link_libraries(shader_test ${ASR_LIBRARIES})

add_executable(no_instancing_test ${ASR_SOURCES} "tests/no_instancing_test.cpp")
target_link_libraries(no_instancing_test ${ASR_LIBRARIES})

add_executable(instancing_test ${ASR_SOURCES} "tests/instancing_test.cpp")
target_link_libraries(instancing_test ${ASR_LIBRARIES})

add_executable(lighting_test ${ASR_SOURCES} "tests/lighting_test.cpp")
target_link_libraries(lighting_test ${ASR_LIBRARIES})
