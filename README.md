
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

Prior to building this library on any platform you will need:
- Git
- CMake 3.10 or newer
- Vulkan SDK, Version 1.2

##### Microsoft Visual Studio 2019
1. Download project directly or via git. ( You will need Git installed for direct download )
2. Run Build_VisualStudio2019.bat
3. Generated solution file is in `build/VK2D.sln`

##### Microsoft Visual Studio 2017
1. Download project directly or via git. ( You will need Git installed for direct download )
2. Run Build_VisualStudio2017.bat
3. Generated solution file is in `build/VK2D.sln`


#### Available CMake options:
```
VK2D_DEBUG                  = default OFF   -> Enable or disable library debug features, used only for the library development.
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

Copyright (c) 2020 Niko Kauppi Noxagonal@gmail.com

This software is released under [MIT license](LICENSE.md)

