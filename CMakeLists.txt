
# VK2D library root

cmake_minimum_required(VERSION 3.16)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Need both Release and Debug versions on VS, I want to put these
	# into the same folder for ease of use on the user application,
	# renaming the .dll and .lib files won't work so I'm just changing
	# the name of the entire project based on build type.
	# If this causes any problems on other platforms I'll have to rethink
	# this, for now I'll just leave it as is.
	set(VK2D_NAME VK2D_d)
else()
	set(VK2D_NAME VK2D)
endif()

project(${VK2D_NAME} CXX)

message("========== VK2D ==========")

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



################################################################
# Check required packages available
################################################################
if(NOT Vulkan_FOUND)
find_package(Vulkan REQUIRED)
endif()

if(NOT Git_FOUND)
find_package(Git REQUIRED QUIET)
endif()



################################################################
# Update library submodules.
################################################################
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	option(GIT_SUBMODULE "Check submodules during build" ON)
	if(GIT_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()
else()
	message(FATAL_ERROR "You must download this repository via Git")
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/glfw/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/stb/stb.h")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/freetype2/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/glslang/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()



################################################################
# Set global project settings.
################################################################
# Use multiple cores when compiling, not automatically
# enabled in Visual Studio. 
if(MSVC)
	add_compile_options(
		"/MP"
	)
endif()

# Disable in source build.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Enable folder separators for targets.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



################################################################
# Options, variables, lists, files.
################################################################
option(VK2D_DEBUG
	"This enables debug printing, Vulkan validation and extra checks to make sure VK2D is performing correctly, this does not help with library usage debugging."
	OFF
)
option(VK2D_BUILD_STATIC_LIBRARY
	"Build static library instead of shared."
	OFF
)
option(VK2D_BUILD_TESTS
	"Build test programs and test targets"
	OFF
)
option(VK2D_BUILD_EXAMPLES
	"Build example programs which will help in learning VK2D"
	ON
)
option(VK2D_BUILD_DOCS
	"Build documentation"
	OFF
)
option(VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE
	"Build documentation for internal source files too"
	OFF
)

set(EXTERNAL_INCLUDES_PATH
	"${PROJECT_SOURCE_DIR}/ExternalLibraries/"
)
set(EXTERNAL_LIBRARY_PATH
	"${PROJECT_BINARY_DIR}/ExternalLibraries/"
)

set(EXTERNAL_INCLUDES
	"${EXTERNAL_INCLUDES_PATH}/freetype2/include/"
	"${EXTERNAL_INCLUDES_PATH}/glfw/include/"
	"${EXTERNAL_INCLUDES_PATH}/stb/"
)
set(EXTERNAL_LIBRARIES
	"${EXTERNAL_LIBRARY_PATH}/freetype2"
	"${EXTERNAL_LIBRARY_PATH}/glfw/src"
)

# All files in Include folder
file(GLOB_RECURSE FILES_INCLUDE
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*"
)
# All files in source folder
file(GLOB_RECURSE FILES_SOURCE
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*"
)
# GLSL shaders
file(GLOB FILES_GLSL_SHADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*"
)
# Compiled shaders, only really need the "IncludeAllShaders.h",
set(FILES_COMPILED_SHADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Spir-V/IncludeAllShaders.h"
)



################################################################
# Project prerequisites
################################################################
add_subdirectory("ExternalLibraries" EXCLUDE_FROM_ALL)
add_subdirectory("Tools")



################################################################
# VK2D
################################################################
source_group(TREE "${PROJECT_SOURCE_DIR}"
	FILES
		${FILES_INCLUDE}
		${FILES_SOURCE}
		${FILES_GLSL_SHADERS}
		${FILES_COMPILED_SHADERS}
)

if(VK2D_BUILD_STATIC_LIBRARY)
	add_library(${VK2D_NAME}
		STATIC
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
			${FILES_COMPILED_SHADERS}
	)
else()
	add_library(${VK2D_NAME}
		SHARED
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
			${FILES_COMPILED_SHADERS}
	)
endif()

target_precompile_headers(${VK2D_NAME}
	PRIVATE
		"${PROJECT_SOURCE_DIR}/Source/Core/PreCompiledHeader.h"
)

add_dependencies(${VK2D_NAME}
	freetype
	glfw
)

set_target_properties(${VK2D_NAME}
	PROPERTIES
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib/"
		LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib/"
		RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Bin/"
)

# Internal includes
target_include_directories(${VK2D_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/Source/")
target_include_directories(${VK2D_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Include/")
target_include_directories(${VK2D_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/Shaders/")

# External includes
target_include_directories(${VK2D_NAME}
	PRIVATE
		Vulkan::Vulkan
		${EXTERNAL_INCLUDES}
)

target_link_libraries(${VK2D_NAME}
	Vulkan::Vulkan
	freetype
	glfw
)

if(${VK2D_DEBUG})
	target_compile_definitions(${VK2D_NAME}
		PRIVATE
			VK2D_DEBUG_ENABLE=1)
else()
	target_compile_definitions(${VK2D_NAME}
		PRIVATE
			VK2D_DEBUG_ENABLE=0)
endif()



################################################################
# Tools to run before building VK2D
################################################################
if(MSVC)
	# This tool is supposed to run only when any of the glsl shader
	# files have changed. Unfortunately it looks like Visual Studio
	# ignores file dependencies. Shaders are only compiled when the
	# library is built, and every time the library is built.
	
	#add_custom_target(RunTool_CompileGLSLShaderToSpirV
	#	COMMAND
	#		CompileGLSLShadersToSpirV
	#		"-shaderpath" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/"
	#)
	#add_dependencies(RunTool_CompileGLSLShaderToSpirV
	#	CompileGLSLShadersToSpirV
	#)
	#set_target_properties(RunTool_CompileGLSLShaderToSpirV
	#	PROPERTIES
	#		FOLDER						"Utilities"
	#)
	
	add_custom_command(
		TARGET ${VK2D_NAME}
		PRE_BUILD
		COMMAND CompileGLSLShadersToSpirV
			"-shaderpath" "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
	)
	add_dependencies(${VK2D_NAME}
		CompileGLSLShadersToSpirV
	)

else()
	# Other than Visual Studio compilers.

	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Spir-V/IncludeAllShaders.h"
		COMMAND CompileGLSLShadersToSpirV
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
		DEPENDS ${FILES_GLSL_SHADERS}
	)
endif()



################################################################
# Additionals
################################################################
if(VK2D_BUILD_TESTS)
	add_subdirectory("Tests")
endif()

if(VK2D_BUILD_EXAMPLES)
	add_subdirectory("Examples")
endif()

if(VK2D_BUILD_DOCS)
	add_subdirectory("Docs")
endif()



################################################################
# Install target
################################################################
install(
	TARGETS
		${VK2D_NAME}
	LIBRARY DESTINATION "lib/"
	ARCHIVE DESTINATION "lib/"
	RUNTIME DESTINATION "bin/"
)
install(
	DIRECTORY
		"${CMAKE_CURRENT_SOURCE_DIR}/Include/"
	DESTINATION
		"include/"
)
install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
		"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md"
	DESTINATION
		"./"
)
