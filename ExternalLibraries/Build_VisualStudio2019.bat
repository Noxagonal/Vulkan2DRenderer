
set build_name=VisualStudio2019

set glfw_dir=glfw-3.3
set freetype_dir=freetype-2.10.1
set glslang_dir=glslang

cd ..
cd Build
cd %build_name%
mkdir ExternalLibraries
cd ExternalLibraries



mkdir %glfw_dir%
cd %glfw_dir%
cmake -D BUILD_SHARED_LIBS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_DOCS=OFF -D GLFW_VULKAN_STATIC=ON -G "Visual Studio 16 2019" -A "x64" "..\..\..\..\ExternalLibraries\%glfw_dir%"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" GLFW.sln /property:Configuration=Release
cd ..



mkdir %freetype_dir%
cd %freetype_dir%
cmake -D CMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A "x64" "..\..\..\..\ExternalLibraries\%freetype_dir%"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" freetype.sln /property:Configuration=Release
cd ..



mkdir %glslang_dir%
cd %glslang_dir%
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="$(pwd)/install" -G "Visual Studio 16 2019" -A "x64" "%VK_SDK_PATH%/glslang"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" glslang.sln /property:Configuration=Release
cd ..

cd ..\..\..\ExternalLibraries

