
# CompileGLSLShaderToSpir-V is a tool to convert GLSL shaders into Spir-V header files.
# The Vulkan 2D renderer library uses this to automatically convert GLSL shaders into Spir-V
# when compiling the library.

cmake_minimum_required(VERSION 3.15)

set(GLSLANG_PATH
	"${CMAKE_SOURCE_DIR}/ExternalLibraries/glslang"
)

add_executable(CompileGLSLShadersToSpirV
	"Main.cpp"
)

set_target_properties(CompileGLSLShadersToSpirV
	PROPERTIES
		FOLDER						"Tools"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

target_include_directories(CompileGLSLShadersToSpirV
	PUBLIC
		"${GLSLANG_PATH}"
)

target_link_libraries(CompileGLSLShadersToSpirV
	glslang
	glslang-default-resource-limits
	OGLCompiler
	OSDependent
	SPIRV
	SPVRemapper
#	HLSL
)

add_dependencies(CompileGLSLShadersToSpirV
	glslang
	glslang-default-resource-limits
	OGLCompiler
	OSDependent
	SPIRV
	SPVRemapper
#	HLSL
	glslangValidator
	spirv-remap
)

