@echo off



set /A build_debug=0
set /A build_static=0
set /A build_tests=0
set /A build_examples=1
set /A build_docs=0

:OptionsLoop
call :PrintOptionsMenu ret, %build_debug%, %build_static%, %build_tests%, %build_examples%, %build_docs%
if %ret%==1 call :ToggleValue build_debug, %build_debug%
if %ret%==2 call :ToggleValue build_static, %build_static%
if %ret%==3 call :ToggleValue build_tests, %build_tests%
if %ret%==4 call :ToggleValue build_examples, %build_examples%
if %ret%==5 call :ToggleValue build_docs, %build_docs%
if %ret%==6 goto Build
if %ret%==7 goto End
goto OptionsLoop







:Build

call :GetAsOnOffString build_debug_str, %build_debug%
call :GetAsOnOffString build_static_str, %build_static%
call :GetAsOnOffString build_tests_str, %build_tests%
call :GetAsOnOffString build_examples_str, %build_examples%
call :GetAsOnOffString build_docs_str, %build_docs%


set project="VK2D"
set generator="Visual Studio 15 2017 Win64"
set architecture="x64"
set source_dir=..

mkdir build
cd build

cmake -G %generator% -Wno-dev -D CMAKE_BUILD_TYPE=Release -D VK2D_DEBUG=%build_debug_str% -D VK2D_BUILD_STATIC_LIBRARY=%build_static_str% -D VK2D_BUILD_TESTS=%build_tests_str% -D VK2D_BUILD_EXAMPLES=%build_examples_str% -D VK2D_BUILD_DOCS=%build_docs% "%source_dir%"
echo Done

cd %source_dir%

goto End






:PrintOptionsMenu
call :GetAsOnOffString build_debug_str, %~2
call :GetAsOnOffString build_static_str, %~3
call :GetAsOnOffString build_tests_str, %~4
call :GetAsOnOffString build_examples_str, %~5
call :GetAsOnOffString build_docs_str, %~6

cls
echo:
echo ************************************************************
echo *
echo * Choose from the following list and press enter to select.
echo *
echo * [ 1 ] Enable library debugging options: %build_debug_str%
echo * [ 2 ] Build static library (EXPERIMENTAL): %build_static_str%
echo * [ 3 ] Build tests: %build_tests_str%
echo * [ 4 ] Build examples: %build_examples_str%
echo * [ 5 ] Build documentation: %build_docs_str%
echo *
echo * [ 6 ] Build with current settings
echo * [ 7 ] Exit
echo *
echo ************************************************************
echo:

set /p "%~1=Selection: "
exit /B 0



REM First parameter is return value, second parameter is the original value
:ToggleValue
if %~2 == 0 (set /A "%~1"=1) else (set /A "%~1"=0)
exit /B 0



REM return numerals 0 and 1 as "OFF" and "ON"
REM First parameter is return value, second parameter is the original value
:GetAsOnOffString
if %~2 == 0 (set "%~1=OFF") else (set "%~1=ON")
exit /B 0



:End
pause
exit /B %ERRORLEVEL%
