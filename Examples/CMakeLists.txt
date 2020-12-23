
#Examples root

cmake_minimum_required(VERSION 3.16)

function(BuildExample EXECUTABLE_NAME)

	add_executable("${EXECUTABLE_NAME}"
		"${EXECUTABLE_NAME}.cpp"
	)
	target_include_directories("${EXECUTABLE_NAME}"
		PUBLIC
			"${PROJECT_SOURCE_DIR}/Include"
	)
	target_link_libraries("${EXECUTABLE_NAME}"
		PUBLIC
			${VK2D_NAME}
	)
	set_target_properties("${EXECUTABLE_NAME}"
		PROPERTIES
			FOLDER						"Examples"
			CXX_STANDARD				17
			ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
			LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Lib"
			RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/Bin"
	)
	add_dependencies("${EXECUTABLE_NAME}"
		${VK2D_NAME}
	)

endfunction()


# Create project/executable for each .cpp file in this directory.
file(GLOB ExampleFiles
	CONFIGURE_DEPENDS
	"*.cpp"
)
foreach(ExampleFile ${ExampleFiles})
	get_filename_component(ExampleFileName ${ExampleFile} NAME_WE)
	BuildExample(${ExampleFileName})
endforeach()
