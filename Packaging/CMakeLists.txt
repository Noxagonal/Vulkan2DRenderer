
cmake_minimum_required(VERSION 3.16)

project(VK2DPackaging)

set(CPACK_PACKAGE_NAME "VK2D")
set(CPACK_PACKAGE_VENDOR "Niko Kauppi")
set(CPACK_PACKAGE_CONTACT "Niko Kauppi - Noxagonal@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VK2D - Easy to use vulkan 2D rendering library")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
#set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "VK2D")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

if(WIN32)
	include("Windows.txt")
elseif(UNIX AND NOT APPLE)
	include("Linux(debian).txt")
endif()

include(CPack)
