#pragma once

#include "BuildOptions.h"
#include "Platform.h"

#include <stdint.h>



namespace vk2d {

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;

} // vk2d



#if defined( VK2D_PLATFORM_WINDOWS )

	#if defined( _MSV_VER )
		#if ( _MSV_VER < 1910 )
			#error "Need at least visual studio 2017"
		#endif

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

		// Calling convension
		#define VK2D_APIENTRY				__stdcall



		// Debug enable or disable
		#if defined( _DEBUG )
			#define VK2D_DEBUG				1
		#else
			#define VK2D_DEBUG 				0
		#endif

	#else
		#error "Please add compiler support here!"
	#endif

	// Warning helper for visual studio, other compilers can use #warning
	#define _VK2D_WARNING_STRINGIFY_SUB(x) #x
	#define _VK2D_WARNING_STRINGIFY(x) _VK2D_WARNING_STRINGIFY_SUB(x)
	// Usage: #pragma VK2D_WARNING( "message" )
	#define VK2D_WARNING( txt ) message( __FILE__"(" _VK2D_WARNING_STRINGIFY(__LINE__) ") : warning: " txt )



#elif defined( VK2D_PLATFORM_LINUX )


	#if defined(__GNUC__)
		#if ( __GNUC__ < 8 )
			#error "Need at least GCC 8"
		#endif
		// define dynamic library export/import
		#if defined( VK2D_LIBRARY_STATIC )
			#define VK2D_API
		#else
			#if defined( VK2D_LIBRARY_EXPORT )
				#define VK2D_API			__attribute__((visibility("default")))
			#else
				#define VK2D_API
			#endif
		#endif

		// Calling convension
		#define VK2D_APIENTRY				// Default, change in the future if needed.



		// Debug enable or disable
		#if !defined( NDEBUG )
			#define VK2D_DEBUG				1
		#else
			#define VK2D_DEBUG 				0
		#endif

	#else
		#error "Please add compiler support here!"
	#endif



#else
	#error "Please add platform support here!"
#endif



#if !defined( VK2D_DEBUG )
#error "VK2D_DEBUG not defined!"
#endif

#if !defined( VK2D_API )
#error "VK2D_API not defined!"
#endif

#if !defined( VK2D_APIENTRY )
#error "VK2D_APIENTRY not defined!"
#endif






namespace vk2d {

enum ConsoleColor : uint8_t {
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
bool operator<( vk2d::ConsoleColor c1, vk2d::ConsoleColor c2 ){
	return uint8_t( c1 ) < uint8_t( c2 );
}

#if defined( VK2D_PLATFORM_WINDOWS )

inline void SetConsoleColor(
	vk2d::ConsoleColor		text_color				= vk2d::ConsoleColor::DEFAULT,
	vk2d::ConsoleColor		background_color		= vk2d::ConsoleColor::DEFAULT
)
{
	auto tc		= uint8_t( text_color );
	auto bc		= uint8_t( background_color );
	if( tc == uint8_t( vk2d::ConsoleColor::DEFAULT ) ) tc = uint8_t( vk2d::ConsoleColor::GRAY );
	if( bc == uint8_t( vk2d::ConsoleColor::DEFAULT ) ) bc = uint8_t( vk2d::ConsoleColor::BLACK );
	unsigned short attributes = ( (unsigned)bc << 4 ) | (unsigned)tc;
	HANDLE std_out_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( std_out_handle, attributes );
}

#elif defined( VK2D_PLATFORM_LINUX )

#include <iostream>
#include <map>

std::map<vk2d::ConsoleColor, char*> text_color_map {
	{ vk2d::ConsoleColor::DEFAULT, "39" },
	{ vk2d::ConsoleColor::BLACK, "30" },
	{ vk2d::ConsoleColor::DARK_RED, "31" },
	{ vk2d::ConsoleColor::DARK_GREEN, "32" },
	{ vk2d::ConsoleColor::DARK_YELLOW, "33" },
	{ vk2d::ConsoleColor::DARK_BLUE, "34" },
	{ vk2d::ConsoleColor::DARK_MAGENTA, "35" },
	{ vk2d::ConsoleColor::DARK_CYAN, "36" },
	{ vk2d::ConsoleColor::GRAY, "37" },
	{ vk2d::ConsoleColor::DARK_GRAY, "90" },
	{ vk2d::ConsoleColor::RED, "91" },
	{ vk2d::ConsoleColor::GREEN, "92" },
	{ vk2d::ConsoleColor::YELLOW, "93" },
	{ vk2d::ConsoleColor::BLUE, "94" },
	{ vk2d::ConsoleColor::MAGENTA, "95" },
	{ vk2d::ConsoleColor::CYAN, "96" },
	{ vk2d::ConsoleColor::WHITE, "97" }
};

std::map<vk2d::ConsoleColor, char*> background_color_map {
	{ vk2d::ConsoleColor::DEFAULT, "49" },
	{ vk2d::ConsoleColor::BLACK, "40" },
	{ vk2d::ConsoleColor::RED, "41" },
	{ vk2d::ConsoleColor::GRAY, "42" },
	{ vk2d::ConsoleColor::YELLOW, "43" },
	{ vk2d::ConsoleColor::BLUE, "44" },
	{ vk2d::ConsoleColor::MAGENTA, "45" },
	{ vk2d::ConsoleColor::CYAN, "46" },
	{ vk2d::ConsoleColor::GRAY, "47" },
	{ vk2d::ConsoleColor::DARK_GRAY, "100" },
	{ vk2d::ConsoleColor::RED, "101" },
	{ vk2d::ConsoleColor::GREEN, "102" },
	{ vk2d::ConsoleColor::YELLOW, "103" },
	{ vk2d::ConsoleColor::BLUE, "104" },
	{ vk2d::ConsoleColor::MAGENTA, "105" },
	{ vk2d::ConsoleColor::CYAN, "106" },
	{ vk2d::ConsoleColor::WHITE, "107" }
};

inline void SetConsoleColor(
	ConsoleColor		text_color				= ConsoleColor::WHITE,
	ConsoleColor		background_color		= ConsoleColor::BLACK
)
{
	std::cout << "\003[0;" << text_color_map[ text_color ] << ";" << background_color_map[ background_color ] << ";m";
}

#else

#error "Please add platform support!"

#endif

} // vk2d
