
# CompileGLSLShaderToSpir-V is a tool to convert GLSL shaders into Spir-V header files.
# The Vulkan 2D renderer library uses this to automatically convert GLSL shaders into Spir-V
# when compiling the library.

cmake_minimum_required(VERSION 3.15)

set(GLSLANG_PATH
	"$ENV{VK_SDK_PATH}/glslang"
)

set(GLSLANG_LIBRARY_PATH
	"${CMAKE_CURRENT_BINARY_DIR}/../../ExternalLibraries/glslang"
)

add_executable(CompleGLSLShadersToSpirV
	"Main.cpp"
)

set_target_properties(CompleGLSLShadersToSpirV
	PROPERTIES
		FOLDER						"Tools"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

target_include_directories(CompleGLSLShadersToSpirV
	PUBLIC
		"${GLSLANG_PATH}"
)

target_link_directories(CompleGLSLShadersToSpirV
	PUBLIC
		"${GLSLANG_LIBRARY_PATH}"
)

target_link_libraries(CompleGLSLShadersToSpirV
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
