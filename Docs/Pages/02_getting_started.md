Getting Started						{#gettingstarted}
===============

# Getting the library
Currently the only way to get this library is from github and compiling it yourself. Official packages are coming soon.<br>
For now take a look at [compiling the package yourself](@ref compiling).

On Microsoft Windows: <br>
After compiling and installing the library you should have a dynamic library version compiled at `build/install/`, this is
a self contained package now that you can move anywhere you wish.

On other platforms: <br>
You should be all set.



# Setting up the environment

## Microsoft Windows

The install package contains everything needed. <br>
`bin/` folder contains the VK2D.dll file which you can copy into your own project somewhere where your program executable can find it. <br>
`lib/` folder contains the import library for the VK2D.dll, add this to your project's linker additional dependencies setting. <br>
`include/` This is where all the required header files are located, add this in your project include settings.



## Linux

TODO...



# Examples

1. [Hello world](@ref HelloWorld.cpp)
2. [Draw using mesh shapes](@ref DrawMeshShapes.cpp)
3. [Event handler](@ref EventHandler.cpp)
4. [Multiple windows](@ref MultipleWindows.cpp)
5. [Render target textures](@ref RenderTargetTextures.cpp)
6. [Transformations](@ref Transformations.cpp)
