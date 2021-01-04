
# Tests root

cmake_minimum_required(VERSION 3.16)

include(CTest)

function(BuildTestcase EXECUTABLE_NAME)

	add_executable("${EXECUTABLE_NAME}"
		"${EXECUTABLE_NAME}.cpp"
		"TestCommon.cpp"
		"TestCommon.h"
	)
	target_include_directories("${EXECUTABLE_NAME}"
		PUBLIC
			"${PROJECT_SOURCE_DIR}/Include"
			"${EXTERNAL_SUBMODULES_PATH}/glm/"
	)
	target_link_libraries("${EXECUTABLE_NAME}"
		PUBLIC
			${VK2D_NAME}
	)
	set_target_properties("${EXECUTABLE_NAME}"
		PROPERTIES
			FOLDER						"Tests"
			CXX_STANDARD				17
			ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
			LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
			RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Bin"
	)
	add_dependencies("${EXECUTABLE_NAME}"
		${VK2D_NAME}
	)
	add_test("${EXECUTABLE_NAME}"
		"${CMAKE_CURRENT_BINARY_DIR}/Bin/${EXECUTABLE_NAME}"
	)
	
endfunction()

# TODO: Automate these based on the files found on disk...
BuildTestcase("ContainerArray")
BuildTestcase("BasicRender")
BuildTestcase("DrawShapes")
