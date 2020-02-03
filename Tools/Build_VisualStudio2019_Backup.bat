
set build_name=VisualStudio2019

set compile_tool=CompileGLSLShadersToSpir-V

set generator="Visual Studio 16 2019"
set architecture="x64"

set source_tools_dir=..\..\..\..\Tools

cd ..
cd Build
cd %build_name%
mkdir Tools
cd Tools



mkdir %compile_tool%
cd %compile_tool%
cmake -D CMAKE_BUILD_TYPE=Release -G %generator% -A %architecture% "%source_tools_dir%\%compile_tool%"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" %compile_tool%.sln /property:Configuration=Release
cd ..

cd ..\..\..\Tools
