
cmake_minimum_required(VERSION 3.15)


################################
# FreeType 2
################################
add_subdirectory("freetype-2.10.1")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(freetype
	PROPERTIES
		FOLDER						"ExternalLibraries"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

################################
# GLFW 3
################################
set(BUILD_SHARED_LIBS		OFF	)
set(GLFW_BUILD_EXAMPLES		OFF	)
set(GLFW_BUILD_TESTS		OFF	)
set(GLFW_BUILD_DOCS			OFF	)
set(GLFW_VULKAN_STATIC		ON	)
add_subdirectory("glfw-3.3")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(glfw
	PROPERTIES
		FOLDER						"ExternalLibraries/GLFW"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

set_target_properties(uninstall
	PROPERTIES
		FOLDER						"ExternalLibraries/GLFW"
		CXX_STANDARD				17
		ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
)

################################
# glslang
################################
add_subdirectory("glslang")
