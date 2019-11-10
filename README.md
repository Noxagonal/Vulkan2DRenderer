
# C++ Vulkan 2D Renderer

:exclamation: This project is a work in progress and is not ready to be used in any project as of yet. :exclamation:

Vulkan 2D Renderer is a C++17 library designed to be easy to use 2D rendering backend, you only need a few lines of code to create a window and display simple shapes.

------

### Motivation

- This project is primarily a passion project.
- I needed a high performance, multi-threaded and multi-window 2D rendering engine specifically for another project and wanted to use Vulkan backend to achieve that.
- Vulkan 2D rendering engines are not common and I didn't find any fitting my needs.

------

### Capabilities

- Draws predefined 2D shapes. ( Box, Circle, Pie... )
- Can also draw direcly from vertex and index lists. ( Useful for GUI rendering )
- Textured rendering.
- Multi-threaded resource loading automatically loads textures and other resources in the background, automatically synchronizes resources when needed.
- Color controlled by vertex color.
- Alpha blending.
- Draws polygons, lines and points.
- Window scaling on the fly.

------

### Limitations

- Does not draw 3D, this is meant to be a simple library to use.
- No multi-sampling yet.
- No line width scaling yet.
- No point size scaling yet.
- Work in progress. We'll know more about limitations later.
- Crashes will happen frequently.

------

### Included libraries

| Library | License | Github page |
| --- | --- | --- |
| GLFW 3.3 | [zlib/libpng](ExternalLibraries/glfw-3.3/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](ExternalLibraries/stb/LICENSE.md) | https://github.com/nothings/stb |

------

### License

Copyright (c) 2019 Niko Kauppi Niko40@gmail.com

This software is released under [MIT license](LICENSE.md)

