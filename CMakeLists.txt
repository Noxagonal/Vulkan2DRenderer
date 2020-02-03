
# VK2D library root

cmake_minimum_required(VERSION 3.15)

project(VK2D CXX)

if(MSVC)
	add_compile_options(
		"/MP"
	)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)



add_subdirectory("ExternalLibraries")



option(VK2D_BUILD_STATIC_LIBRARY
	"Build static library instead of shared."
)
option(VK2D_BUILD_TESTS
	"Build test programs and test targets"
)
option(VK2D_BUILD_EXAMPLES
	"Build example programs which will help in learning VK2D"
)

add_subdirectory("Tools")

set(FILES_INCLUDE
	"Include/Vulkan2DRenderer.h"
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
	"Include/Interface/Renderer.h"
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
	"Source/Header/Impl/RendererImpl.h"
	"Source/Header/Impl/ResourceManagerImpl.h"
	"Source/Header/Impl/SamplerImpl.h"
	"Source/Header/Impl/TextureResourceImpl.h"
	"Source/Header/Impl/WindowImpl.h"
)

set(FILES_SOURCE_IMPL
	"Source/Impl/FontResourceImpl.cpp"
	"Source/Impl/RendererImpl.cpp"
	"Source/Impl/ResourceManagerImpl.cpp"
	"Source/Impl/SamplerImpl.cpp"
	"Source/Impl/TextureResourceImpl.cpp"
	"Source/Impl/WindowImpl.cpp"
)

set(FILES_SOURCE_INTERFACE
	"Source/Interface/FontResource.cpp"
	"Source/Interface/Mesh.cpp"
	"Source/Interface/Renderer.cpp"
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

source_group(TREE "${CMAKE_SOURCE_DIR}"
	FILES
		${INC_FILES}
		${SRC_INC_FILES}
		${SRC_FILES}
)



if(VK2D_BUILD_STATIC_LIBRARY)
	add_library("${PROJECT_NAME}"
		STATIC
			${SRC_FILES}
			${SRC_INC_FILES}
			${INC_FILES}
	)
else()
	add_library("${PROJECT_NAME}"
		SHARED
			${SRC_FILES}
			${SRC_INC_FILES}
			${INC_FILES}
	)
endif()

set_target_properties("${PROJECT_NAME}"
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
	"${CMAKE_BINARY_DIR}/../ExternalLibraries"
)

set(EXTERNAL_INCLUDES
	"${EXTERNAL_INCLUDES_PATH}/freetype-2.10.1/include"
	"${EXTERNAL_INCLUDES_PATH}/glfw-3.3/include"
	"${EXTERNAL_INCLUDES_PATH}/stb"
)
set(EXTERNAL_LIBRARIES
	"${EXTERNAL_LIBRARY_PATH}/freetype-2.10.1"
	"${EXTERNAL_LIBRARY_PATH}/freetype-2.10.1/Release"
	"${EXTERNAL_LIBRARY_PATH}/glfw-3.3/src"
	"${EXTERNAL_LIBRARY_PATH}/glfw-3.3/src/Release"
)

target_include_directories("${PROJECT_NAME}"
	PUBLIC
		"$ENV{VK_SDK_PATH}/Include"
		"${EXTERNAL_INCLUDES}"
)

target_link_directories("${PROJECT_NAME}"
	PUBLIC
		"$ENV{VK_SDK_PATH}/Lib"
		"${EXTERNAL_LIBRARIES}"
)

target_link_libraries("${PROJECT_NAME}"
	"vulkan-1.lib"
	"freetype.lib"
	"glfw3.lib"
)

if(MSVC)
add_custom_target(RunTool_CompileGLSLShaderToSpirV
	COMMAND
		"${CMAKE_BINARY_DIR}/../Tools/CompileGLSLShadersToSpir-V/Bin/Release/CompileGLSLShadersToSpir-V.exe"
		"-shaderpath" "${CMAKE_SOURCE_DIR}/Source/Shaders"
)
add_dependencies(
	"${PROJECT_NAME}"
	RunTool_CompileGLSLShaderToSpirV
)
set_target_properties(RunTool_CompileGLSLShaderToSpirV
	PROPERTIES
		FOLDER						"Utilities"
)

else()
message(FATAL_ERROR
	"Please add platform support here!"
)
endif()



if(VK2D_BUILD_TESTS)
	add_subdirectory("Tests")
endif()

if(VK2D_BUILD_EXAMPLES)
	add_subdirectory("Examples")
endif()
