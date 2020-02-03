
set build_name=VisualStudio2019

set project="VK2D"
set generator="Visual Studio 16 2019"
set architecture="x64"
set source_dir=..\..\..

mkdir Build
cd Build
mkdir %build_name%
cd %build_name%
cd ..\..

cd ExternalLibraries
call Build_VisualStudio2019.bat

cd ..\Tools
call Build_VisualStudio2019.bat

cd ..

cd Build
cd %build_name%

mkdir %project%
cd %project%
cmake -G %generator% -A %architecture% -D CMAKE_BUILD_TYPE=Release -D BUILD_STATIC_LIBRARY=OFF -D BUILD_TESTS=ON -D BUILD_EXAMPLES=ON "%source_dir%"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" %project%.sln /property:Configuration=Release -m
cd ..

cd ..\..
