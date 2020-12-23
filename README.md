
# Vulkan 2D Renderer library

:exclamation: This project is a work in progress (pre-alpha). Frequent API changes, ABI changes, crashes... :exclamation:

Vulkan 2D Renderer is a C++17 library designed to be easy to use, high performance 2D rendering backend for realtime applications.
You only need a few lines of code to create a window and display simple shapes.
On the other hand you can create a variety of different effects using meshes and applying different modifiers to them.
Internally workloads are automatically grouped to create more efficient submissions to the GPU and all resource loading and saving is multithreaded.

![latest screenshot](/Screenshots/GaussianBlur.png)

### Motivation

- This project is primarily a passion project.
- Vulkan 2D rendering engines/libraries are not common.
- I didn't find any easy to use 2D rendering libraries with multi-window support.
- This should work pretty well as an example for people wanting to see concrete Vulkan API use. (Feel free to message me if you have any questions about anything)

### Capabilities

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

### Limitations

- Does not render 3D, this is meant to be a simple library to use.
- This is not a game engine, the purpose of this library is strictly limited to 2D drawing.
- No HDRI image support. Didn't find any use for this in a 2D application.
- No scene management, although transformation hierarchy is implemented so you can create your own.
- No custom shaders, might implement these later if there's any need for these.
- Work in progress. We'll know more about limitations later.
- Crashes and garbage renders will likely happen frequently.

------

## Building from source

:exclamation: You will need to use Git, direct download from GitHub will not work because of dependencies. :exclamation:

Prior to building this library on any platform you will need:
- Git 2.20 or newer
- CMake 3.16 or newer
- Vulkan SDK 1.2 or newer

If using the build tool you will need (recommended for new users):
- Python 3.8 or newer

If you're building the documentation you will also need these (disabled by default):
- Doxygen 1.8.20 or newer

### Getting the sources

Getting the sources is the same process on all platforms. On your command line or terminal, navigate to the folder where you
wish to download the sources and copy-paste this line.
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
```

### Compiling the sources

There are couple of ways to compile this library, you can use standard CMake or you can use the provided build tool.

#### Using the build tool

Navigate to the `Vulkan2DRenderer` folder and simply run `Build.bat` on Windows or `Build.py` on Linux.
To get started quickly, you can select the quick build option, this will generate the project files in `build/Release`
and `build/Debug` folders and compiles the project, on windows it's also installed into a ready-to-use folder.

Creating project files and compiling the sources manually is of course possible via CMake if you prefer. However you
should note that the included build tool generates a `build/Release` and `build/Debug` folders so you should avoid
those paths.

##### Windows specifics

On Windows installing will create an `install` folder which will contain everything you need to use this library on
your project. You will have to manually link it to your project and move the `VK2D.dll` and `VK2D_d.dll` file somewhere
where your executable can see it, usually to your project's working directory.

##### Linux specifics

On Linux installing requires superuser priviledges so you need to do this separately, the build tool will build the project
files and compile but it will not install this library for you.

------

## External libraries used in this project

| Library | License | Web page |
| --- | --- | --- |
| Freetype 2 | [FTL](https://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/FTL.TXT) | https://www.freetype.org |
| GLFW 3 | [zlib/libpng](https://github.com/glfw/glfw/blob/master/LICENSE.md) | https://github.com/glfw/glfw |
| STB | [MIT](https://github.com/nothings/stb/blob/master/LICENSE) | https://github.com/nothings/stb |
| glslang | ['*Custom*'](https://github.com/KhronosGroup/glslang/blob/master/LICENSE.txt) | https://github.com/KhronosGroup/glslang |

------

## License

Copyright (c) 2020 Niko Kauppi Noxagonal@gmail.com

This software is released under [MIT license](LICENSE.md)
