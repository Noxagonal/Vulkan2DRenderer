
# Vulkan 2D Renderer library

:exclamation: This project is a work in progress (pre-alpha). Frequent API changes, ABI changes, crashes... :exclamation:

Vulkan 2D Renderer is a C++17 library, it is designed to be easy to use, high performance 2D rendering backend for realtime applications.
You only need a few lines of code to create a window and display simple shapes.
On the other hand you can create a variety of different effects using meshes and applying different modifiers to them.
Internally workloads are automatically grouped to create more efficient submissions to the GPU and all resource loading and saving is multithreaded.

![latest screenshot](/Screenshots/GaussianBlur.png)

------

### Motivation

- This project is primarily a passion project.
- Vulkan 2D rendering engines are not common and I didn't find any fitting my needs.
- This should also work pretty well as an example for people wanting to see concrete Vulkan API use.

------

### Capabilities

- Generate and draw simple 2D shapes. ( Box, Circle, Pie... )
- Draw direcly from vertex and index lists. ( Useful for GUI rendering and for more complex stuff )
- Mesh modifiers to warp and distort shapes.
- Textures, Samplers, Color and alpha blending.
- Render target textures.
- Gaussian blur.
- Multi-threaded resource loading and saving.
- Multi-window support.
- Multisampling.
- Keyboard, mouse and gamepad input.

------

### Limitations

- Does not draw 3D, this is meant to be a simple library to use.
- No HDRI image support. Didn't find any use for this in a 2D application.
- Work in progress. We'll know more about limitations later.
- Crashes will likely happen frequently.

------

### Building project files

:exclamation: You will need to use Git, direct download from GitHub will not work because of dependencies. :exclamation:

Prior to building this library on any platform you will need:
- Git
- CMake 3.16 or newer
- Vulkan SDK, Version 1.2
- Doxygen ( optional, only needed if you're building the documentation )


##### Windows Visual Studio 2019
1. Open PowerShell and navigate to a folder where you would like to download the VK2D project sources.
2. Copy and paste the following commands to the PowerShell window and press enter, this will also download everything for you (Don't close PowerShell even if it appears stuck, this can take a few minutes).
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
cd Vulkan2DRenderer
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A "x64" ..
```
( Generated solution file is located in `build/VK2D.sln` )

3. If you wish to create an easier to use package without extra sources you can also run these commands in the PowerShell after you've created the project and solution files above.
```
cmake --build . --config Release
cmake --install .
```
( Install package is located in `build/install` )


##### Microsoft Visual Studio 2017
1. Open PowerShell and navigate to a folder where you would like to download the VK2D project sources.
2. Copy and paste the following commands to the PowerShell window and press enter, this will also download everything for you (Don't close PowerShell even if it appears stuck, this can take a few minutes).
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
cd Vulkan2DRenderer
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
( Generated solution file is located in `build/VK2D.sln` )

3. If you wish to create an easier to use package without extra sources you can also run these commands in the PowerShell after you've created the project and solution files above.
```
cmake --build . --config Release
cmake --install .
```
( Install package is located in `build/install` )



#### Available CMake options:
```
VK2D_DEBUG                          = default OFF   -> Enable or disable library debug features, used only for the library development.
VK2D_BUILD_STATIC_LIBRARY           = default OFF   -> Build the library as static. DOES NOT WORK YET!
VK2D_BUILD_TESTS                    = default OFF   -> Build tests.
VK2D_BUILD_EXAMPLES                 = default ON    -> Build and include examples on how to use this library.
VK2D_BUILD_DOCS                     = default OFF   -> Build documentation. Needs Doxygen so it's off by default.
VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE = default OFF   -> Build documentation for the entire source instead of just the interface.
```
These can be set when you're generating the project files.

For example:
```
cmake -D VK2D_BUILD_TESTS=ON -D VK2D_BUILD_EXAMPLES=OFF -G "Visual Studio 16 2019" -A "x64" ..
```

------

### External libraries used in this project

| Library | License | Web page |
| --- | --- | --- |
| Freetype 2 | [FTL](https://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/FTL.TXT) | https://www.freetype.org |
| GLFW 3 | [zlib/libpng](https://github.com/glfw/glfw/blob/master/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](https://github.com/nothings/stb/blob/master/LICENSE) | https://github.com/nothings/stb |
| glslang | ['*Custom*'](https://github.com/KhronosGroup/glslang/blob/master/LICENSE.txt) | https://github.com/KhronosGroup/glslang |

------

### License

Copyright (c) 2020 Niko Kauppi Noxagonal@gmail.com

This software is released under [MIT license](LICENSE.md)
