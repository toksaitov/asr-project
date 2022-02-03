asr
===

*asr* is a simple renderer written in C++ for creative coding and data visualization purposes.

## Prerequisites

Install all the prerequisites first.

* MSVC (with Visual Studio 2022), Clang (with Latest Xcode or Command Line Tools for Xcode), GCC (any version with support of C++17)
* CMake (version `3.22` or higher)
* Conan (version `1.57` or higher)
* GPU drivers (latest version with stable support of OpenGL 2 or ES 2.0)

## Building

Create a `build` directory and set it as a current working directory:

```bash
mkdir build
cd build
```

Install dependencies with the Conan package manager once:

```bash
conan install .. --build missing
```

On Ubuntu 20.04, you may also have to install the following system packages first:

```bash
sudo apt install pkg-config libegl-dev libglu1-mesa-dev
```

Configure the project to generate IDE or Makefiles:

```bash
# On Windows for Visual Studio 2022
cmake .. -G "Visual Studio 17 2022"

# On macOS for Xcode
cmake .. -G "Xcode"

# on macOS or GNU/Linux to generate Makefiles
cmake .. -G "Unix Makefiles"
```

Build the project:

```bash
cmake --build .
```

Run the test program from the `asr/build/bin/` directory:

```bash
cd .. # Ensure that the current working directory is set to the root asr folder.
./build/bin/<name of the graphics test executable>
```

You may have to set the Working Directory (CWD) in your IDE for some test targets to open image files.
