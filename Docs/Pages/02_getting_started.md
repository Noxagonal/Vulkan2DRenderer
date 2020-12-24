Getting Started						{#gettingstarted}
===============

# Getting the library
Currently the only way to get this library is from github and compiling it yourself. Official packages are coming soon.<br>
For now take a look at [compiling the package yourself](@ref compiling_using_build_tool).

On Microsoft Windows: <br>
After compiling and installing the library you should have a dynamic library version compiled at `build_tool/install/`, this is
a self contained package that you can move anywhere you wish.

On other platforms: <br>
You will need to install manually for now.



# Setting up the environment

## Microsoft Windows

The install package contains everything you need to use the VK2D library. <br>
`bin/` folder contains the `VK2D.dll` and `VK2D_d.dll` files which you can copy into your own project somewhere where your program
executable can find it, usually to your projects working directory. <br>
`lib/` folder contains the import libraries, add either `VK2D.lib` to release version of your project or `VK2D_d.lib` for the debug
version. <br>
`include/` This is where all the required header files are located, add this in your project include settings.



## Linux

Once you compile and install VK2D release version you should be able to use it anywhere in your system.



# Examples

1. [Hello world](@ref HelloWorld.cpp)
2. [Draw using mesh shapes](@ref DrawMeshShapes.cpp)
3. [Event handler](@ref EventHandler.cpp)
4. [Multiple windows](@ref MultipleWindows.cpp)
5. [Render target textures](@ref RenderTargetTextures.cpp)
6. [Transformations](@ref Transformations.cpp)
