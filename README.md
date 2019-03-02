# Random terrain generator

Small program that generates a random terrain based on given parameters. Sets the height positions on a grid and also creates a triangle mesh from it so it can be rendered with OpenGL.

## Tools 

- C++11
- GLM as submodule
- GLFW for window
- Glad for OpenGL

## Clone and build

Clone the submodules as well with

`git clone --recurse-submodules -j4 https://github.com/jparimaa/random-terrain.git`

Run CMake and build the project. For CMake the `GLFW_PATH` parameter needs to be setup, for example

`cmake . -DGLFW_PATH=/path/to/glfw_3.2.1 && make`

## Screenshot

![screenshot](screenshot.png?raw=true "screenshot")
