
# VK2D library root

cmake_minimum_required(VERSION 3.16)

project(VK2D CXX)

message("========== VK2D ==========")

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()


if(NOT Vulkan_FOUND)
find_package(Vulkan REQUIRED)
endif()

# Update library submodules with git.
find_package(Git REQUIRED QUIET)
if(GIT_FOUND)
	if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	# Update submodules as needed
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
else()
	message(FATAL_ERROR "Git not found")
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



# No reason not to compile using multiple cores.
if(MSVC)
	add_compile_options(
		"/MP"
	)
endif()

# Disable in source build.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Enable folder separators for targets.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



add_subdirectory("ExternalLibraries" EXCLUDE_FROM_ALL)



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

add_subdirectory("Tools")

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

list(APPEND FILES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Spir-V/IncludeAllShaders.h")

source_group(TREE "${PROJECT_SOURCE_DIR}"
	FILES
		${FILES_INCLUDE}
		${FILES_SOURCE}
		${FILES_GLSL_SHADERS}
)

if(VK2D_BUILD_STATIC_LIBRARY)
	add_library(VK2D
		STATIC
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
	)
else()
	add_library(VK2D
		SHARED
			${FILES_INCLUDE}
			${FILES_SOURCE}
			${FILES_GLSL_SHADERS}
	)
endif()

set_target_properties(VK2D
	PROPERTIES
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Bin"
)

target_precompile_headers(VK2D
	PRIVATE
		"${PROJECT_SOURCE_DIR}/Source/Core/PreCompiledHeader.h"
)

target_include_directories(VK2D PUBLIC "${PROJECT_SOURCE_DIR}/Shaders/")

set(EXTERNAL_INCLUDES_PATH
	"${PROJECT_SOURCE_DIR}/ExternalLibraries"
)
set(EXTERNAL_LIBRARY_PATH
	"${PROJECT_BINARY_DIR}/ExternalLibraries"
)

set(EXTERNAL_INCLUDES
	"${EXTERNAL_INCLUDES_PATH}/freetype2/include"
	"${EXTERNAL_INCLUDES_PATH}/glfw/include"
	"${EXTERNAL_INCLUDES_PATH}/stb"
)
set(EXTERNAL_LIBRARIES
	"${EXTERNAL_LIBRARY_PATH}/freetype2"
	"${EXTERNAL_LIBRARY_PATH}/glfw/src"
)

target_include_directories(VK2D PRIVATE Source)

target_include_directories(VK2D PUBLIC Include)

target_include_directories(VK2D
	PUBLIC
		Vulkan::Vulkan
		"${EXTERNAL_INCLUDES}"
)

target_link_libraries(VK2D
	Vulkan::Vulkan
	freetype
	glfw
)

if(${VK2D_DEBUG})
	target_compile_definitions(VK2D
		PRIVATE
			VK2D_DEBUG_ENABLE=1)
else()
	target_compile_definitions(VK2D
		PRIVATE
			VK2D_DEBUG_ENABLE=0)
endif()

# Custom tools to run before building VK2D
if(MSVC)
	# This tool is supposed to run only when any of the glsl shader
	# files have changed. Unfortunately it looks like Visual Studio
	# ignores file dependencies. Shaders are only compiled when the
	# library is built, and every time the library is built.
	
	#add_custom_target(RunTool_CompileGLSLShaderToSpirV
	#	COMMAND
	#		CompileGLSLShadersToSpirV
	#		"-shaderpath" "${PROJECT_SOURCE_DIR}/Source/Shaders"
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
			"-shaderpath" "${PROJECT_SOURCE_DIR}/Shaders"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
	)
	add_dependencies(VK2D
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

add_dependencies(VK2D
	freetype
	glfw
)



if(VK2D_BUILD_TESTS)
	add_subdirectory("Tests")
endif()

if(VK2D_BUILD_EXAMPLES)
	add_subdirectory("Examples")
endif()

if(VK2D_BUILD_DOCS)
	add_subdirectory("Docs")
endif()



install(
	TARGETS
		VK2D
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	RUNTIME DESTINATION "bin"
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
