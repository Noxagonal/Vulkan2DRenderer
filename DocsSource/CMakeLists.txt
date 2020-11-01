
# VK2D Documentation generator

# All required dependencies to compile documentation:
# - Doxygen
# - Sphinx
# - Breathe
# - sphinx_pdj_theme
# Doxygen has an installer for Windows, the rest can be installed via pip.

cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()



set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake"
	"${CMAKE_MODULE_PATH}"
)

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)


###########################
# Doxygen
###########################

set(DOXYGEN_INPUT_INCLUDE_DIR
	"${PROJECT_SOURCE_DIR}/Include" 
)
set(DOXYGEN_INPUT_SOURCE_DIR
	"${PROJECT_SOURCE_DIR}/Source"
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

message(DEBUG ${FILES_INCLUDE})

add_custom_command(
	OUTPUT
		${DOXYGEN_INDEX_FILE}
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

#add_custom_target(Doxygen ALL
#	DEPENDS
#		${DOXYGEN_INDEX_FILE}
#)


###########################
#Sphinx
###########################

set(SPHINX_SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/Sphinx"
)
set(SPHINX_BUILD
	"${CMAKE_CURRENT_BINARY_DIR}/Docs/Sphinx"
)
set(SPHINX_INDEX_FILE
	"${SPHINX_BUILD}/index.html"
)

add_custom_command(
	OUTPUT
		${SPHINX_INDEX_FILE}
	COMMAND
		${SPHINX_EXECUTABLE} -b html
		-Dbreathe_projects.VK2D="${DOXYGEN_OUTPUT_DIR}/xml"
		${SPHINX_SOURCE}
		${SPHINX_BUILD}
	WORKING_DIRECTORY
		${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS
		"${SPHINX_SOURCE}/index.rst"
		${DOXYGEN_INDEX_FILE}
	MAIN_DEPENDENCY
		"${SPHINX_SOURCE}/conf.py"
	COMMENT
		"Generating Sphinx docs"
)

add_custom_target(Documentation ALL
	DEPENDS
		${SPHINX_INDEX_FILE}
)



# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY
	${SPHINX_BUILD}
	DESTINATION
		${CMAKE_INSTALL_DOCDIR}
)
