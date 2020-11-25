#pragma once

#include "Common.h"

#include <string>



namespace vk2d {



enum class ConsoleColor : uint8_t
{
	BLACK			= 0,
	DARK_BLUE		= 1,
	DARK_GREEN		= 2,
	DARK_CYAN		= 3,
	DARK_RED		= 4,
	DARK_MAGENTA	= 5,
	DARK_YELLOW		= 6,
	GRAY			= 7,
	DARK_GRAY		= 8,
	BLUE			= 9,
	GREEN			= 10,
	CYAN			= 11,
	RED				= 12,
	MAGENTA			= 13,
	YELLOW			= 14,
	WHITE			= 15,
	DEFAULT			= 255
};

VK2D_API void VK2D_APIENTRY ConsolePrint(
	std::string_view		text,
	vk2d::ConsoleColor		text_color				= vk2d::ConsoleColor::DEFAULT,
	vk2d::ConsoleColor		background_color		= vk2d::ConsoleColor::DEFAULT );



} // vk2d
