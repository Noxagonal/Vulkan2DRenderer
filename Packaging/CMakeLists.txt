
cmake_minimum_required(VERSION 3.16)

project(VK2DInstaller)

if(WIN32)
	include("Windows.txt")
endif()

include(CPack)
