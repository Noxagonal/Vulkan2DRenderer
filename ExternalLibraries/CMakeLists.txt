
cmake_minimum_required(VERSION 3.10)



if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



################################
# FreeType 2
################################
add_subdirectory("freetype2")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(freetype
	PROPERTIES
		FOLDER						"ExternalLibraries"
		CXX_STANDARD				17
		POSITION_INDEPENDENT_CODE	ON
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

################################
# GLFW 3
################################
set(BUILD_SHARED_LIBS		OFF	CACHE BOOL	"We need static library so we can include it within our shared library.")
set(GLFW_BUILD_EXAMPLES		OFF	CACHE BOOL	"No need for examples.")
set(GLFW_BUILD_TESTS		OFF	CACHE BOOL	"No need for tests.")
set(GLFW_BUILD_DOCS			OFF	CACHE BOOL	"No need for docs.")
set(GLFW_VULKAN_STATIC		OFF	CACHE BOOL	"We plan to link with static vulkan library.")
add_subdirectory("glfw")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(glfw
	PROPERTIES
		FOLDER						"ExternalLibraries/GLFW"
		CXX_STANDARD				17
		POSITION_INDEPENDENT_CODE	ON
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(uninstall
	PROPERTIES
		FOLDER						"ExternalLibraries/GLFW"
		CXX_STANDARD				17
		POSITION_INDEPENDENT_CODE	ON
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

################################
# glslang
################################
set(BUILD_SHARED_LIBS		OFF	CACHE BOOL	"We need static library so we can include it within our shared library.")
set(SKIP_GLSLANG_INSTALL	ON	CACHE BOOL	"No need for install options, we are building static library.")
set(ENABLE_HLSL				OFF	CACHE BOOL	"We do not plan to use HLSL so we can skip it.")
set(ENABLE_CTEST			OFF	CACHE BOOL	"We are not testing glslang library.")
add_subdirectory("glslang")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(glslang
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(glslang-default-resource-limits
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(OGLCompiler
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(OSDependent
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(SPIRV
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(SPVRemapper
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

#set_target_properties(HLSL
#	PROPERTIES
#		FOLDER						"ExternalLibraries/glslang/hlsl"
#		CXX_STANDARD				17
#		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
#		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
#		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
#)

set_target_properties(glslangValidator
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang/tools"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(spirv-remap
	PROPERTIES
		FOLDER						"ExternalLibraries/glslang/tools"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)
