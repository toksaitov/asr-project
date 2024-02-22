cmake_minimum_required(VERSION 3.25)

project(asr)

set(CMAKE_CXX_STANDARD 17)
set(ASR_SOURCES "include/asr.h")
set(ASR_LIBRARIES "GLEW::GLEW" "SDL2::SDL2")

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
include_directories("./include")

add_executable(triangle_test ${ASR_SOURCES} "tests/triangle_test.cpp")
target_link_libraries(triangle_test ${ASR_LIBRARIES})

add_executable(circle_test ${ASR_SOURCES} "tests/circle_test.cpp")
target_link_libraries(circle_test ${ASR_LIBRARIES})

add_executable(rectangle_test ${ASR_SOURCES} "tests/rectangle_test.cpp")
target_link_libraries(rectangle_test ${ASR_LIBRARIES})
