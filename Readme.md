# asr

`asr` is a simple renderer written in C++ designed for creative coding and data visualization.

## Supported Platforms

* Windows 10, 11
* macOS Sonoma, Sequoia
* Ubuntu 22.04, 24.04

## Prerequisites

Ensure all the prerequisites are installed before proceeding.

* MSVC (with Visual Studio 2022), Clang (with the latest Xcode or Command Line Tools for Xcode), or GCC (any version with support for C++17)
* CMake (version 3.25 or higher)
* Conan (version 2 or higher)
* CLion (version 2024 or higher) with the latest Conan plugin
* GPU drivers (latest version with stable support for OpenGL 2 or ES 2.0)

## Building

1. Ensure the Conan packgage manager has at least the default compiler profile:

   ```
   conan profile detect --force
   ```

2. Install dependencies using Conan for `Release` and `Debug` configurations:

    ```bash
    conan install . --build missing
    conan install . --build missing --settings build_type=Debug
    ```

    On Ubuntu, additional system packages might be required. Follow the recommendations in the output from the previous command or rerun Conan with the following options to install the necessary packages automatically:

    ```bash
    sudo apt install pkg-config libopengl-dev
    conan install . --build missing --conf tools.system.package_manager:mode=install --conf tools.system.package_manager:sudo=True
    conan install . --build missing --conf tools.system.package_manager:mode=install --conf tools.system.package_manager:sudo=True --settings build_type=Debug
    ```

3. Generate build files:

    ```bash
    # On Windows
    cmake --preset conan-default

    # On macOS, GNU/Linux, or Windows with WSL
    cmake --preset conan-release # or `conan-debug` for a build with debugging information
    ```

4. Build the project:

    ```bash
    cmake --build --preset conan-release # or `conan-debug` for a build with debugging information
    ```

5. Run the test program from the `./build/Release/` directory:

    ```bash
    (cd ./build/Release && ./<name of the graphics test executable>) # Note the parentheses
    ```

    You may need to set the Working Directory (CWD) in your IDE manually for some test targets to locate shader or image files.
