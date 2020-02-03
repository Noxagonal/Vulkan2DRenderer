
#Examples root

cmake_minimum_required(VERSION 3.8)

function(BuildExample EXECUTABLE_NAME)
	add_executable("${EXECUTABLE_NAME}"
		"${EXECUTABLE_NAME}.cpp"
	)
	target_include_directories("${EXECUTABLE_NAME}"
		PUBLIC
			"${CMAKE_SOURCE_DIR}/Include"
	)
	target_link_libraries("${EXECUTABLE_NAME}"
		PUBLIC
			"${PROJECT_NAME}"
	)
	set_target_properties("${EXECUTABLE_NAME}"
		PROPERTIES
			FOLDER						"Examples"
			CXX_STANDARD				17
			ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
			LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Lib"
			RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/Bin"
	)
endfunction()

BuildExample("DrawSimpleShapes")
