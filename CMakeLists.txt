
# VK2D library root

cmake_minimum_required(VERSION 3.13)

project(VK2D CXX)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



# Update library submodules with git.
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
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
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/freetype2/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ExternalLibraries/glfw/CMakeLists.txt")
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



add_subdirectory("ExternalLibraries")



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

add_subdirectory("Tools")

set(FILES_INCLUDE
	"Include/VK2D.h"
)

set(FILES_INCLUDE_CORE
	"Include/Core/BuildOptions.h"
	"Include/Core/Common.h"
	"Include/Core/Platform.h"
	"Include/Core/Version.hpp"
)

set(FILES_INCLUDE_INTERFACE
	"Include/Interface/FontResource.h"
	"Include/Interface/Mesh.h"
	"Include/Interface/Instance.h"
	"Include/Interface/RenderPrimitives.h"
	"Include/Interface/Resource.h"
	"Include/Interface/ResourceManager.h"
	"Include/Interface/Sampler.h"
	"Include/Interface/TextureResource.h"
	"Include/Interface/Window.h"
)

set(FILES_SOURCE_CORE
	"Source/Core/DescriptorSet.cpp"
	"Source/Core/MeshBuffer.cpp"
	"Source/Core/Platform.cpp"
	"Source/Core/QueueResolver.cpp"
	"Source/Core/ShaderInterface.cpp"
	"Source/Core/SourceCommon.cpp"
	"Source/Core/ThreadPool.cpp"
	"Source/Core/ThreadPrivateResources.cpp"
	"Source/Core/VulkanMemoryManagement.cpp"
)

set(FILES_SOURCE_HEADER_CORE
	"Source/Header/Core/DescriptorSet.h"
	"Source/Header/Core/MeshBuffer.h"
	"Source/Header/Core/QueueResolver.h"
	"Source/Header/Core/ShaderInterface.h"
	"Source/Header/Core/SourceCommon.h"
	"Source/Header/Core/ThreadPool.h"
	"Source/Header/Core/ThreadPrivateResources.h"
	"Source/Header/Core/VulkanMemoryManagement.h"
)

set(FILES_SOURCE_HEADER_IMPL
	"Source/Header/Impl/FontResourceImpl.h"
	"Source/Header/Impl/InstanceImpl.h"
	"Source/Header/Impl/ResourceManagerImpl.h"
	"Source/Header/Impl/SamplerImpl.h"
	"Source/Header/Impl/TextureResourceImpl.h"
	"Source/Header/Impl/WindowImpl.h"
)

set(FILES_SOURCE_IMPL
	"Source/Impl/FontResourceImpl.cpp"
	"Source/Impl/InstanceImpl.cpp"
	"Source/Impl/ResourceManagerImpl.cpp"
	"Source/Impl/SamplerImpl.cpp"
	"Source/Impl/TextureResourceImpl.cpp"
	"Source/Impl/WindowImpl.cpp"
)

set(FILES_SOURCE_INTERFACE
	"Source/Interface/FontResource.cpp"
	"Source/Interface/Mesh.cpp"
	"Source/Interface/Instance.cpp"
	"Source/Interface/Resource.cpp"
	"Source/Interface/ResourceManager.cpp"
	"Source/Interface/Sampler.cpp"
	"Source/Interface/TextureResource.cpp"
	"Source/Interface/Window.cpp"
)

set(FILES_SOURCE_SHADERS
	"Source/Shaders/EntrypointNames.txt"
	"Source/Shaders/SingleTextured.frag"
	"Source/Shaders/SingleTextured.vert"
	"Source/Shaders/TriangleMultitextured.frag"
	"Source/Shaders/TriangleMultitextured.vert"
)

file(GLOB FILES_SOURCE_SHADERS_SPIRV
	"Source/Shaders/Spir-V/*.h"
)

set(INC_FILES
	${FILES_INCLUDE}
	${FILES_INCLUDE_CORE}
	${FILES_INCLUDE_INTERFACE}
)

set(SRC_INC_FILES
	${FILES_SOURCE_HEADER_CORE}
	${FILES_SOURCE_HEADER_IMPL}
)

set(SRC_FILES
	${FILES_SOURCE_CORE}
	${FILES_SOURCE_IMPL}
	${FILES_SOURCE_INTERFACE}
)

set(SHADER_FILES
	${FILES_SOURCE_SHADERS}
	${FILES_SOURCE_SHADERS_SPIRV}
)

source_group(TREE "${CMAKE_SOURCE_DIR}"
	FILES
		${INC_FILES}
		${SRC_INC_FILES}
		${SRC_FILES}
		${SHADER_FILES}
)



if(VK2D_BUILD_STATIC_LIBRARY)
	add_library(VK2D
		STATIC
			${SRC_FILES}
			${SRC_INC_FILES}
			${INC_FILES}
			${SHADER_FILES}
	)
else()
	add_library(VK2D
		SHARED
			${SRC_FILES}
			${SRC_INC_FILES}
			${INC_FILES}
			${SHADER_FILES}
	)
endif()

set_target_properties(VK2D
	PROPERTIES
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set(EXTERNAL_INCLUDES_PATH
	"${CMAKE_SOURCE_DIR}/ExternalLibraries"
)
set(EXTERNAL_LIBRARY_PATH
	"${CMAKE_BINARY_DIR}/ExternalLibraries"
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

target_include_directories(VK2D
	PUBLIC
		"$ENV{VK_SDK_PATH}/Include"
		"${EXTERNAL_INCLUDES}"
)

target_link_directories(VK2D
	PUBLIC
		"$ENV{VK_SDK_PATH}/Lib"
		"${EXTERNAL_LIBRARIES}"
)

target_link_libraries(VK2D
	"vulkan-1"
	freetype
	glfw
)

#target_link_libraries(VK2D
#	"vulkan-1.lib"
#	"freetype.lib"
#	"glfw3.lib"
#)

add_custom_target(RunTool_CompileGLSLShaderToSpirV
	COMMAND
		CompileGLSLShadersToSpirV
		"-shaderpath" "${CMAKE_SOURCE_DIR}/Source/Shaders"
)
add_dependencies(RunTool_CompileGLSLShaderToSpirV
	CompileGLSLShadersToSpirV
)
set_target_properties(RunTool_CompileGLSLShaderToSpirV
	PROPERTIES
		FOLDER						"Utilities"
)

add_dependencies(VK2D
	freetype
	glfw
	RunTool_CompileGLSLShaderToSpirV
)



if(VK2D_BUILD_TESTS)
	add_subdirectory("Tests")
endif()

if(VK2D_BUILD_EXAMPLES)
	add_subdirectory("Examples")
endif()
