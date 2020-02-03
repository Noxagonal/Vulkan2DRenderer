
# CompileGLSLShaderToSpir-V is a tool to convert GLSL shaders into Spir-V header files.
# The Vulkan 2D renderer library uses this to automatically convert GLSL shaders into Spir-V
# when compiling the library.

cmake_minimum_required(VERSION 3.8)

project("CompileGLSLShadersToSpir-V")

set(SRC_FILES
	"Main.cpp"
)

set(GLSLANG_INCLUDE_PATH
	"$ENV{VK_SDK_PATH}/glslang"
)

set(GLSLANG_LIBRARY_PATH
	"${CMAKE_CURRENT_BINARY_DIR}/../../ExternalLibraries/glslang"
)

add_executable(${PROJECT_NAME}
	"${SRC_FILES}"
)

set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD				17
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

#set_target_properties( targets...
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		"${GLSLANG_INCLUDE_PATH}"
)

target_link_directories(${PROJECT_NAME}
	PUBLIC
		"${GLSLANG_LIBRARY_PATH}"
)

target_link_libraries(${PROJECT_NAME}
	"${GLSLANG_LIBRARY_PATH}/glslang/Release/glslang.lib"
	"${GLSLANG_LIBRARY_PATH}/glslang/OSDependent/Windows/Release/OSDependent.lib"
	"${GLSLANG_LIBRARY_PATH}/hlsl/Release/HLSL.lib"
	"${GLSLANG_LIBRARY_PATH}/OGLCompilersDLL/Release/OGLCompiler.lib"
	"${GLSLANG_LIBRARY_PATH}/SPIRV/Release/SPIRV.lib"
	"${GLSLANG_LIBRARY_PATH}/SPIRV/Release/SPVRemapper.lib"
	"${GLSLANG_LIBRARY_PATH}/StandAlone/Release/glslang-default-resource-limits.lib"
	"${GLSLANG_LIBRARY_PATH}/External/spirv-tools/source/Release/SPIRV-Tools.lib"
	"${GLSLANG_LIBRARY_PATH}/External/spirv-tools/source/opt/Release/SPIRV-Tools-opt.lib"
)
