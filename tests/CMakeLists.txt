
# Tests root

cmake_minimum_required(VERSION 3.16)

include(CTest)

function(BuildTestcase EXECUTABLE_NAME)
	message("Adding Test: ${EXECUTABLE_NAME}")
	add_executable("${EXECUTABLE_NAME}"
		"${EXECUTABLE_NAME}.cpp"
		"Testbed/Testbed.cpp"
		"Testbed/Testbed.h"
	)
	target_include_directories("${EXECUTABLE_NAME}"
		PUBLIC
			"${PROJECT_SOURCE_DIR}/include/"
			"${VK2D_EXTERNAL_SUBMODULES_DIR}/"
	)
	target_link_libraries("${EXECUTABLE_NAME}"
		PUBLIC
			VK2D
	)
	set_target_properties("${EXECUTABLE_NAME}"
		PROPERTIES
			FOLDER						"tests"
			C_STANDARD					17
			CXX_STANDARD				20
			ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib"
			LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib"
			RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/bin"
	)
	add_dependencies("${EXECUTABLE_NAME}"
		VK2D
	)
	add_test("${EXECUTABLE_NAME}"
		"${CMAKE_CURRENT_BINARY_DIR}/bin/${EXECUTABLE_NAME}"
	)
endfunction()

# Create project/executable for each .cpp file in this directory.
file(GLOB TestFiles
	CONFIGURE_DEPENDS
	"*.cpp"
)
foreach(TestFile ${TestFiles})
	if(NOT IS_DIRECTORY ${TestFile})
		get_filename_component(TestFileName ${TestFile} NAME_WE)
		BuildTestcase(${TestFileName})
	endif()
endforeach()
