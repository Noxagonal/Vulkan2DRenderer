#pragma once

#include "Core/Common.h"

#include <string>



namespace vk2d {



/// @brief		Text or background color used when printing stuff to the console.
/// @see		vk2d::ConsolePrint()
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

/// @brief		Print a message to the console, usually used for debugging only as
///				in the final application the console is not usually visible.
/// @note		Multithreading: Any thread.
/// @param[in]	text
///				Message you wish to print to the console window.
/// @param[in]	text_color
///				Color of the message.
/// @param[in]	background_color
///				Color of the background of the letters you wish to print. ( not entire console window )
VK2D_API void VK2D_APIENTRY ConsolePrint(
	std::string_view		text,
	vk2d::ConsoleColor		text_color				= vk2d::ConsoleColor::DEFAULT,
	vk2d::ConsoleColor		background_color		= vk2d::ConsoleColor::DEFAULT );



} // vk2d
