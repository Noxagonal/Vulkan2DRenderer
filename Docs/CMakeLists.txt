
# VK2D Documentation generator

# All required dependencies to compile documentation:
# - Doxygen

cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake"
	"${CMAKE_MODULE_PATH}"
)



###########################
# Doxygen
###########################

find_package(Doxygen REQUIRED)

set(DOXYGEN_INPUT_SOURCE_DIR
	"${PROJECT_SOURCE_DIR}/Source"
)
if(NOT VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE)
set(DOXYGEN_INPUT_SOURCE_DIR
	""
)
endif()

set(DOXYGEN_INPUT_INCLUDE_DIR
	"${PROJECT_SOURCE_DIR}/Include" 
)
set(DOXYGEN_OUTPUT_DIR
	"${CMAKE_CURRENT_BINARY_DIR}/Docs/Doxygen"
)
set(DOXYGEN_STRIP_FROM_PATH
	${PROJECT_SOURCE_DIR}
)
set(DOXYGEN_INDEX_FILE
	"${DOXYGEN_OUTPUT_DIR}/xml/index.xml"
)
set(DOXYFILE_IN
	"${CMAKE_CURRENT_SOURCE_DIR}/Doxygen/vk2d.Doxyfile.in"
)
set(DOXYFILE_OUT
	"${CMAKE_CURRENT_BINARY_DIR}/Doxygen/vk2d.Doxyfile"
)

#Replace variables inside @@ with the current values
configure_file(
	${DOXYFILE_IN}
	${DOXYFILE_OUT}
	@ONLY
)

file(MAKE_DIRECTORY
	${DOXYGEN_OUTPUT_DIR}
)

add_custom_target(VK2DDocumentation)
add_custom_command(
	TARGET
		VK2DDocumentation
	PRE_BUILD
	DEPENDS
		${FILES_INCLUDE}
		${FILES_SOURCE}
	COMMAND
		${DOXYGEN_EXECUTABLE}
		${DOXYFILE_OUT}
	MAIN_DEPENDENCY
		${DOXYFILE_OUT}
		${DOXYFILE_IN}
	COMMENT
		"Generating Doxygen docs"
	VERBATIM
)
add_dependencies(VK2D
	VK2DDocumentation
)

install(
	DIRECTORY
		"${CMAKE_CURRENT_BINARY_DIR}/Docs/Doxygen/html/"
	DESTINATION
		"docs/"
)
