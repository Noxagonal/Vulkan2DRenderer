
# C++ Vulkan 2D Renderer

:exclamation: This project is a work in progress and is not ready to be used in any project as of yet. API will change frequently. :exclamation:

Vulkan 2D Renderer is a C++17 library designed to be easy to use, powerful and high performance 2D rendering backend.
You only need a few lines of code to create a window and display simple shapes.
On the other hand you can create a variety of different effects using meshes and applying different modifiers to them.
Internally workloads are automatically grouped to create more efficient submissions to the GPU and all resource loading and saving is multithreaded.

------

### Building

Currently only Visual Studio 2019 is supported, this includes the community edition. Just run "Build_VisualStudio2019.bat" file and follow the instructions. This batch file will create the project solution file in "./Build/VisualStudio2019/VK2D.sln".
To get started see the example projects.

Prior to building this library you will need:
- CMake 3.15 or newer
- Visual Studio 2019
- Vulkan SDK, any version

You can also build project files manually using regular CMake.

###### Windows Visual Studio 2019
```
cmake -G "Visual Studio 16 2019" -A "x64" <SOURCE DIRECTORY>
```

To build tests
```
cmake -G "Visual Studio 16 2019" -A "x64" -D VK2D_BUILD_TESTS=ON <SOURCE DIRECTORY>
```

Available CMake options:
```
VK2D_BUILD_STATIC_LIBRARY   = default OFF   -> Build the library as static. EXPERIMENTAL!
VK2D_BUILD_TESTS            = default OFF   -> Build tests.
VK2D_BUILD_EXAMPLES         = default ON    -> Build and include examples on how to use this library.
```

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

### Included libraries

| Library | License | Github page |
| --- | --- | --- |
| Freetype 2.10.1 | [FTL](ExternalLibraries/freetype-2.10.1/docs/FTL.TXT) | https://www.freetype.org |
| GLFW 3.3 | [zlib/libpng](ExternalLibraries/glfw-3.3/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](ExternalLibraries/stb/LICENSE.md) | https://github.com/nothings/stb |

------

### License

Copyright (c) 2019 Niko Kauppi Niko40@gmail.com

This software is released under [MIT license](LICENSE.md)

