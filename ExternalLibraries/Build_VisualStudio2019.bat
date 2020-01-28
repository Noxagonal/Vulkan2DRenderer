mkdir VisualStudio2019
cd VisualStudio2019



mkdir glfw-3.3
cd glfw-3.3
cmake -D BUILD_SHARED_LIBS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_DOCS=OFF -D GLFW_VULKAN_STATIC=ON -G "Visual Studio 16 2019" -A "x64" ..\..\glfw-3.3

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" GLFW.sln /property:Configuration=Release
cd ..



mkdir freetype-2.10.1
cd freetype-2.10.1
cmake -D CMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A "x64" ..\..\freetype-2.10.1

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" freetype.sln /property:Configuration=Release
cd ..



mkdir glslang
cd glslang
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="$(pwd)/install" -G "Visual Studio 16 2019" -A "x64" %VK_SDK_PATH%/glslang

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" glslang.sln /property:Configuration=Release
cd ..



pause
