cmake_minimum_required(VERSION 3.25)

project(asr)

set(CMAKE_CXX_STANDARD 17)
set(ASR_SOURCES "include/asr.h")
set(ASR_LIBRARIES "GLEW::GLEW" "SDL2::SDL2" "glm::glm")

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm  REQUIRED)
include_directories("./include")

if (WIN32 AND MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(hello_world_test ${ASR_SOURCES} "tests/hello_world_test.cpp")
target_link_libraries(hello_world_test ${ASR_LIBRARIES})

add_executable(circle_test ${ASR_SOURCES} "tests/circle_test.cpp")
target_link_libraries(circle_test ${ASR_LIBRARIES})

add_executable(rectangle_test ${ASR_SOURCES} "tests/rectangle_test.cpp")
target_link_libraries(rectangle_test ${ASR_LIBRARIES})
