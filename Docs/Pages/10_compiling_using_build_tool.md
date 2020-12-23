Compiling using build tool			{#compiling_using_build_tool}
==========================

VK2D includes a build tool to get you started quickly if you're interested just testing the latest features of this library.
It is only meant for building the library into a self contained package for distribution.

You can also [compile via CMake](@ref compiling_using_cmake).

------

# Build Requirements				{#build_tool_requirements}

<span style="color:red">! You will need to use Git, direct download from GitHub will not work because of dependencies !</span>.

Prior to building this library on any platform you will need:
- Git 2.20 or newer
- CMake 3.16 or newer
- Vulkan SDK 1.2 or newer
- Python 3.8 or newer

If you're building the documentation you will also need (disabled by default):
- Doxygen 1.8.20 or newer

------

# Getting the source

You will need to use Git, direct download from github will not work becauseof external dependencies. <br>
On all platforms, open command line/PowerShell/Terminal, navigate to the folder where you wish to download
VK2D source and use this command:
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
```

------

# Building and compiling			{#build_tool_build_and_compile}

The build tool is a python script at the project base directory. Just run `Build.bat` on Windows or `./Build.py`
via terminal on Linux

The build tool has a quick build option if you wish to only get the install package.

You can also set up and build release or debug versions of the library separately with different options.
Just follow the instructions of the build tool.

------

# Install VK2D						{#build_tool_install_vk2d}

## Installing on Microsoft Windows

Installing can be done via the build tool, in this case everything is moved into a folder called `install`.
This folder contains everything you need to use VK2D in your own project.

## Installing on Linux

Installing via this build tool is disabled on Linux as this requires superuser rights.
For now you'll have to install separately with:
```
sudo cmake --install tool_build/Release
```
