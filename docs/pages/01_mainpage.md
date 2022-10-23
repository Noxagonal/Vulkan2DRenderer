Vulkan2DLibrary Documentation		{#mainpage}
=============================

# What is VK2D

VK2D is a Vulkan 2D renderer written in C++17, it is designed to be easy to use, high performance 2D rendering backend for realtime applications.
VK2D enables rapid software development as you only need a few lines of code to create a window and display simple shapes.
You can also create a variety of different effects using meshes and applying different modifiers to them.

<span style="color:red"><b>!!! Please note that this library is on it's alpha stage and crashes and garbage renders may happen. !!!</b></span>.

## Capabilities

- Easily generate and draw simple shapes. ( Box, Circle, Pie... )
- For more control you can also draw custom shapes direcly from vertex and index lists.
- Transformations and transformation hierarchy.
- Instanced rendering.
- Multi-window support.
- Mesh modifiers to warp and distort shapes.
- Textures, Samplers, Color and alpha blending.
- Render target textures with blurring.
- Multisampling.
- Multi-threaded resource loading and saving.
- Keyboard, mouse and gamepad input.

## Limitations

- Does not render 3D, this is meant to be a simple library to use.
- This is not a game engine, the purpose of this library is strictly limited to 2D drawing.
- No HDRI image support.
- No scene management, although transformation hierarchy is implemented so you can create your own.
- No custom shaders. (I might implement these later if there's any need for these)



# External libraries used in this project

| Library | License | Web page |
| --- | --- | --- |
| Freetype 2 | [FTL](https://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/FTL.TXT) | https://www.freetype.org |
| GLFW 3 | [zlib/libpng](https://github.com/glfw/glfw/blob/master/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](https://github.com/nothings/stb/blob/master/LICENSE) | https://github.com/nothings/stb |
| glslang | ['*Custom*'](https://github.com/KhronosGroup/glslang/blob/master/LICENSE.txt) | https://github.com/KhronosGroup/glslang |



# License

Copyright (c) 2020 Niko Kauppi Noxagonal@gmail.com

This software is released under [MIT license](https://github.com/Noxagonal/Vulkan2DRenderer/blob/master/LICENSE.md)
