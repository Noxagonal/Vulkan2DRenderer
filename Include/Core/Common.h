#pragma once

#include "BuildOptions.h"
#include "Platform.h"

#include <stdint.h>



namespace vk2d {

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;

} // vk2d



#if defined( VK_USE_PLATFORM_WIN32_KHR )

// define dynamic library export/import
#if defined( VK2D_LIBRARY_STATIC )
#define VK2D_API
#else
#if defined( VK2D_LIBRARY_EXPORT )
#define VK2D_API			__declspec(dllexport)
#else
#define VK2D_API			__declspec(dllimport)
#endif
#endif

#define VK2D_APIENTRY		__stdcall

#if defined( _DEBUG )
#define VK2D_DEBUG			1
#endif

#else

#error "Please add platform support here!"

#endif



#if !defined( VK2D_API )
#error "VK2D_API not defined!"
#endif

#if !defined( VK2D_APIENTRY )
#error "VK2D_APIENTRY not defined!"
#endif






namespace vk2d {

#ifdef VK_USE_PLATFORM_WIN32_KHR

enum class ConsoleColor : uint8_t {
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
	WHITE			= 15
};

inline void SetConsoleColor(
	vk2d::ConsoleColor		text_color				= vk2d::ConsoleColor::GRAY,
	vk2d::ConsoleColor		background_color		= vk2d::ConsoleColor::BLACK
)
{
	auto tc		= uint8_t( text_color );
	auto bc		= uint8_t( background_color );
	unsigned short attributes = ( (unsigned)bc << 4 ) | (unsigned)tc;
	HANDLE std_out_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( std_out_handle, attributes );
}

#else

#error "Please add platform support!"

inline void SetConsoleColor(
	ConsoleColor		text_color				= ConsoleColor::WHITE,
	ConsoleColor		background_color		= ConsoleColor::BLACK
)
{}

#endif

} // vk2d
