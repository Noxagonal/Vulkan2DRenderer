mkdir VisualStudio2019
cd VisualStudio2019

mkdir glfw-3.3
cd glfw-3.3
cmake -D BUILD_SHARED_LIBS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_DOCS=OFF -D GLFW_VULKAN_STATIC=ON -G "Visual Studio 16 2019" -A "x64" ..\..\glfw-3.3
cd ..

mkdir freetype-2.10.1
cd freetype-2.10.1
cmake -G "Visual Studio 16 2019" -A "x64" ..\..\freetype-2.10.1
cd ..

pause
