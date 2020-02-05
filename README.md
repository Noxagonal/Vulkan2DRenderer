
# C++ Vulkan 2D Renderer

:exclamation: This project is a work in progress and is not ready to be used in any project as of yet. API will change frequently. :exclamation:

Vulkan 2D Renderer is a C++17 library designed to be easy to use, powerful and high performance 2D rendering backend.
You only need a few lines of code to create a window and display simple shapes.
On the other hand you can create a variety of different effects using meshes and applying different modifiers to them.
Internally workloads are automatically grouped to create more efficient submissions to the GPU and all resource loading and saving is multithreaded.

------

### Motivation

- This project is primarily a passion project.
- Vulkan 2D rendering engines are not common and I didn't find any fitting my needs.

------

### Capabilities

- Generate and draw predefined 2D shapes. ( Box, Circle, Pie... )
- Draw direcly from vertex and index lists. ( Useful for GUI rendering )
- Mesh modifiers to warp and distort shapes.
- Textures, Samplers, Color and alpha blending.
- Multi-threaded resource loading and saving.
- Multi-window support.
- Multisampling.
- keyboard, mouse and gamepad input.

------

### Limitations

- Does not draw 3D, this is meant to be a simple library to use.
- Work in progress. We'll know more about limitations later.
- Crashes will likely happen frequently.

------

### Building

:exclamation: You will need to use Git, downloading plain Zip file from GitHub will not work because of dependencies. :exclamation:

Currently only Visual Studio 2019 is supported, this includes the community edition.

Prior to building this library you will need:
- Git
- CMake 3.15 or newer
- Visual Studio 2019
- Vulkan SDK, any version


###### Windows Visual Studio 2019
1. Open the folder where you would like the VK2D library to be installed.
2. Hold shift key and right click on the file explorer, click "Open PowerShell window here".
3. copy and paste the following commands to the PowerShell window and press enter (Don't close PowerShell even if it appears stuck, this can take a few minutes).
```
git clone https://github.com/Niko40/Vulkan2DRenderer
cd Vulkan2DRenderer
mkdir Build
cd Build
cmake -G "Visual Studio 16 2019" -A "x64" ..
```
4. Generated solution is in `Build/VK2D.sln`

Available CMake options:
```
VK2D_BUILD_STATIC_LIBRARY   = default OFF   -> Build the library as static. EXPERIMENTAL!
VK2D_BUILD_TESTS            = default OFF   -> Build tests.
VK2D_BUILD_EXAMPLES         = default ON    -> Build and include examples on how to use this library.
```

------

### External libraries

| Library | License | Web page |
| --- | --- | --- |
| Freetype 2 | [FTL](https://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/FTL.TXT) | https://www.freetype.org |
| GLFW 3 | [zlib/libpng](https://github.com/glfw/glfw/blob/master/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](https://github.com/nothings/stb/blob/master/LICENSE) | https://github.com/nothings/stb |
| glslang | ['*Custom*'](https://github.com/KhronosGroup/glslang/blob/master/LICENSE.txt) | https://github.com/KhronosGroup/glslang |

------

### License

Copyright (c) 2020 Niko Kauppi Niko40@gmail.com

This software is released under [MIT license](LICENSE.md)

