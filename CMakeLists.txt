cmake_minimum_required(VERSION 3.22)

project(asr)

set(CMAKE_CXX_STANDARD 17)

include_directories("./include")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(ASR_SOURCES include/asr.h)
set(ASR_LIBRARIES ${CONAN_LIBS})

if (WIN32 AND MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(triangle_test ${ASR_SOURCES} tests/triangle_test.cpp)
target_link_libraries(triangle_test ${ASR_LIBRARIES})
