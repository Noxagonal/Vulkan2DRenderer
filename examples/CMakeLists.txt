
#Examples root

cmake_minimum_required(VERSION 3.16)

function(BuildExample EXECUTABLE_NAME)
	message("Adding Example: ${EXECUTABLE_NAME}")
	add_executable("${EXECUTABLE_NAME}"
		"${EXECUTABLE_NAME}.cpp"
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
			FOLDER						"examples"
			C_STANDARD					17
			CXX_STANDARD				20
			ARCHIVE_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib"
			LIBRARY_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/lib"
			RUNTIME_OUTPUT_DIRECTORY	"${PROJECT_BINARY_DIR}/bin"
	)
	add_dependencies("${EXECUTABLE_NAME}"
		VK2D
	)
endfunction()


# Create project/executable for each .cpp file in this directory.
file(GLOB ExampleFiles
	CONFIGURE_DEPENDS
	"*.cpp"
)
foreach(ExampleFile ${ExampleFiles})
	if(NOT IS_DIRECTORY ${ExampleFile})
		get_filename_component(ExampleFileName ${ExampleFile} NAME_WE)
		BuildExample(${ExampleFileName})
	endif()
endforeach()
