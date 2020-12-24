
cmake_minimum_required(VERSION 3.16)

project(VK2DInstaller)

if(WIN32)
	include("WindowsInstaller.txt")
endif()

include(CPack)
