
# VK2D Documentation generator

# All required dependencies to compile documentation:
# - Doxygen

cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



################################################################
# Doxygen
################################################################

find_package(Doxygen REQUIRED)



################################################################
# Settings and files that will be placed in Doxyfile
################################################################
set(DOXYFILE_STRIP_FROM_PATH
	${PROJECT_SOURCE_DIR}
)
set(DOXYFILE_SOURCE_DIR
	"${PROJECT_SOURCE_DIR}/src/"
)
if(NOT VK2D_BUILD_DOCS_FOR_COMPLETE_SOURCE)
set(DOXYFILE_SOURCE_DIR
	""
)
endif()

set(DOXYFILE_INCLUDE_DIR
	"${PROJECT_SOURCE_DIR}/include/" 
)
set(DOXYFILE_EXAMPLES_DIR
	"${PROJECT_SOURCE_DIR}/examples/"
)
set(DOXYFILE_PAGES_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}/pages/"
)



################################################################
# General settings
################################################################
set(DOXYGEN_OUTPUT_DIR
	"${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen"
)
set(DOXYGEN_INDEX_FILE
	"${DOXYGEN_OUTPUT_DIR}/xml/index.xml"
)
set(DOXYFILE_IN
	"${CMAKE_CURRENT_SOURCE_DIR}/doxygen/vk2d.Doxyfile.in"
)
set(DOXYFILE_OUT
	"${CMAKE_CURRENT_BINARY_DIR}/doxygen/vk2d.Doxyfile"
)

################################################################
# Create doxygen project
################################################################
#Replace variables inside @@ with the current values
configure_file(
	${DOXYFILE_IN}
	${DOXYFILE_OUT}
	@ONLY
)

file(MAKE_DIRECTORY
	${DOXYGEN_OUTPUT_DIR}
)

file(GLOB_RECURSE DOXYFILE_PAGES
	CONFIGURE_DEPENDS
	"${DOXYFILE_PAGES_DIR}/*.md"
)
add_custom_target(VK2DDocumentation)
add_custom_command(
	TARGET
		VK2DDocumentation
	PRE_BUILD
	DEPENDS
		${FILES_INCLUDE}
		${FILES_SOURCE}
		${DOXYFILE_PAGES}
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



################################################################
# Documentation install
################################################################
install(
	DIRECTORY
		"${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen/html/"
	DESTINATION
		"docs/"
)
