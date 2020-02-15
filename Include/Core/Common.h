#pragma once

#include "Platform.h"

#include <stdint.h>



namespace vk2d {

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;

} // vk2d



#if defined( VK2D_PLATFORM_WINDOWS )

	#if defined( _MSC_VER )
		#if ( _MSC_VER < 1910 )
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

	#else
		#error "Please add compiler support here!"
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
	WHITE			= 15,
	DEFAULT			= 255
};

void SetConsoleColor(
	vk2d::ConsoleColor		text_color				= vk2d::ConsoleColor::DEFAULT,
	vk2d::ConsoleColor		background_color		= vk2d::ConsoleColor::DEFAULT );

} // vk2d
