Compiling using CMake				{#compiling_using_cmake}
=====================

Compiling manually is usually required for library development only, in most cases you can just use the
[build tool](@ref compiling_using_build_tool) instead.

Compiling the library manually typically involves these steps in order:
- [Install required programs and libraries.](@ref build_requirements)
- [Build project files.](@ref build_project_files)
- [Compile VK2D.](@ref compile_vk2d)
- [Install VK2D package.](@ref install_vk2d)

Or use CMake project in your IDE directly.

------

# Build Requirements				{#build_cmake_requirements}

<span style="color:red">! You will need to use Git, direct download from GitHub will not work because of dependencies !</span>.

Prior to building this library on any platform you will need:
- Git 2.20 or newer
- CMake 3.16 or newer
- Vulkan SDK 1.2 or newer

If you're building the documentation you will also need (disabled by default):
- Doxygen 1.8.20 or newer

------

# Build project files				{#build_cmake_project_files}

## Microsoft Visual Studio 2019

1. Open PowerShell and navigate to a folder where you would like to download the VK2D project sources.
2. Copy and paste the following commands to the PowerShell window and press enter, this will also download everything for you (PowerShell may appear stuck for a couple of minutes).
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
cd Vulkan2DRenderer
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A "x64" ..
```
3. Generated solution file is located in `build/VK2D.sln`

## Microsoft Visual Studio 2017

1. Open PowerShell and navigate to a folder where you would like to download the VK2D project sources.
2. Copy and paste the following commands to the PowerShell window and press enter, this will also download everything for you (PowerShell may appear stuck for a couple of minutes).
```
git clone https://github.com/Noxagonal/Vulkan2DRenderer
cd Vulkan2DRenderer
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
3. Generated solution file is located in `build/VK2D.sln`

## Available CMake options

```
VK2D_DEBUG                          = default OFF   -> Enable or disable library debug features, used only for the library development.
VK2D_BUILD_STATIC_LIBRARY           = default OFF   -> Build the library as static. DOES NOT WORK YET!
VK2D_BUILD_TESTS                    = default OFF   -> Build tests.
VK2D_BUILD_EXAMPLES                 = default ON    -> Build and include examples on how to use this library.
VK2D_BUILD_DOCS                     = default OFF   -> Build documentation. Needs Doxygen so it's off by default.
VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE = default OFF   -> Build documentation for the entire source instead of just the interface.
```
These can be set when you are generating the project files.

For example:
```
cmake -D VK2D_BUILD_TESTS=ON -D VK2D_BUILD_EXAMPLES=OFF -G "Visual Studio 16 2019" -A "x64" ..
```



# Compile VK2D						{#build_cmake_compile_vk2d}

You can compile the VK2D library on any platform using this CMake command.
```
cmake --build . --config Release
```

If you wish to compile the debug version of the library you can use this command.
```
cmake --build . --config Debug
```



# Install VK2D						{#build_cmake_install_vk2d}

Installing is done by this command, which also works on all platforms.
```
cmake --install .
```

On Microsoft Windows:
Everything just moved into an install folder which is located at `build/install/`
