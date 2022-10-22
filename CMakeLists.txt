
# VK2D library root

cmake_minimum_required(VERSION 3.16)

project(VK2D CXX)

message("========== VK2D ==========")

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



################################################################
# Check required packages available
################################################################
find_package(Vulkan REQUIRED)
find_package(Git REQUIRED QUIET)



################################################################
# Update library submodules.
################################################################
set(VK2D_EXTERNAL_SUBMODULES_DIR_NAME "external_submodules")
set(VK2D_EXTERNAL_SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${VK2D_EXTERNAL_SUBMODULES_DIR_NAME}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
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

if(NOT EXISTS "${VK2D_EXTERNAL_SUBMODULES_DIR}/glm/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${VK2D_EXTERNAL_SUBMODULES_DIR}/glfw/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${VK2D_EXTERNAL_SUBMODULES_DIR}/stb/stb.h")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${VK2D_EXTERNAL_SUBMODULES_DIR}/freetype2/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${VK2D_EXTERNAL_SUBMODULES_DIR}/glslang/CMakeLists.txt")
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
option(VK2D_ENABLE_LIBRARY_DEBUG_FEATURES
	"This enables debug printing, Vulkan validation and extra checks to make sure VK2D is performing correctly, this is meant only for the developers of this library."
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

set(EXTERNAL_INCLUDES
	"${VK2D_EXTERNAL_SUBMODULES_DIR}/"
	"${VK2D_EXTERNAL_SUBMODULES_DIR}/freetype2/include/"
	"${VK2D_EXTERNAL_SUBMODULES_DIR}/glfw/include/"
	"${VK2D_EXTERNAL_SUBMODULES_DIR}/stb/"
	"${VK2D_EXTERNAL_SUBMODULES_DIR}/glm/"
)

# All files in Include folder
file(GLOB_RECURSE FILES_INCLUDE
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*"
)
# All files in source folder
file(GLOB_RECURSE FILES_SOURCE
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)
# GLSL shaders
file(GLOB FILES_GLSL_SHADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/*"
)
# Compiled shaders, only really need the "IncludeAllShaders.h",
set(FILES_COMPILED_SHADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/spir-v/IncludeAllShaders.h"
)



################################################################
# Project prerequisites
################################################################
add_subdirectory("${VK2D_EXTERNAL_SUBMODULES_DIR_NAME}" EXCLUDE_FROM_ALL)
add_subdirectory("tools")



################################################################
# VK2D
################################################################
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES
		${FILES_INCLUDE}
		${FILES_SOURCE}
		${FILES_GLSL_SHADERS}
		${FILES_COMPILED_SHADERS}
)

if(VK2D_BUILD_STATIC_LIBRARY)
	add_library(VK2D
		STATIC
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
			${FILES_COMPILED_SHADERS}
	)
else()
	add_library(VK2D
		SHARED
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
			${FILES_COMPILED_SHADERS}
	)
endif()

target_precompile_headers(VK2D
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src/core/PreCompiledHeader.h"
)

add_dependencies(VK2D
	freetype
	glfw
)

set_target_properties(VK2D
	PROPERTIES
		CXX_STANDARD				20
		DEBUG_POSTFIX				"_d"
		ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib/"
		LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib/"
		RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/bin/"
)

# Internal includes
target_include_directories(VK2D PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(VK2D PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(VK2D PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

# External includes
target_include_directories(VK2D
	PRIVATE
		Vulkan::Vulkan
		${EXTERNAL_INCLUDES}
)

target_link_libraries(VK2D
	Vulkan::Vulkan
	freetype
	glfw
)

if(${VK2D_ENABLE_LIBRARY_DEBUG_FEATURES})
	target_compile_definitions(VK2D
		PRIVATE
			VK2D_DEBUG_ENABLE=1)
else()
	target_compile_definitions(VK2D
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
	#		"-shaderpath" "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/"
	#)
	#add_dependencies(RunTool_CompileGLSLShaderToSpirV
	#	CompileGLSLShadersToSpirV
	#)
	#set_target_properties(RunTool_CompileGLSLShaderToSpirV
	#	PROPERTIES
	#		FOLDER						"Utilities"
	#)
	
	add_custom_command(
		TARGET VK2D
		PRE_BUILD
		COMMAND CompileGLSLShadersToSpirV
			"-shaderpath" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
	)
	add_dependencies(VK2D
		CompileGLSLShadersToSpirV
	)

else()
	# Other than Visual Studio compilers.

	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/shaders/spir-v/includeAllShaders.h"
		COMMAND CompileGLSLShadersToSpirV
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
		DEPENDS ${FILES_GLSL_SHADERS}
	)
endif()



################################################################
# Additionals
################################################################
if(VK2D_BUILD_TESTS)
	add_subdirectory("tests")
endif()

if(VK2D_BUILD_EXAMPLES)
	add_subdirectory("examples")
endif()

if(VK2D_BUILD_DOCS)
	add_subdirectory("docs")
endif()



################################################################
# Install target
################################################################
install(
	TARGETS
		VK2D
	LIBRARY DESTINATION "lib/"
	ARCHIVE DESTINATION "lib/"
	RUNTIME DESTINATION "bin/"
)
install(
	DIRECTORY
		"${CMAKE_CURRENT_SOURCE_DIR}/include/"
	DESTINATION
		"include/VK2D/"
)
install(
	DIRECTORY
		"${VK2D_EXTERNAL_SUBMODULES_DIR}/glm/"
	DESTINATION
		"include/glm/"
)
install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
		"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md"
	DESTINATION
		"./"
)

