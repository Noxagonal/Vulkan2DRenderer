#pragma once

#include "Core/Platform.h"

#include <stdint.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#define GLM_FORCE_LEFT_HANDED 1
#include <glm/glm/glm.hpp>



// For documentation only
#ifdef DOXYGEN

/// This tells the compiler how the access to the vk2d functions are handled.
/// For example, when the vk2d library is compiled this macro evaluates
/// to __declspec(dllexport) and when the vk2d API is used from outside
/// this macro evaluates to __declspec(dllimport), this allows using the
/// same header files for both the library and the API.
#define VK2D_API

/// This is basically the calling convention to the library.
/// To make sure the program calls the vk2d library functions correctly
/// this macro enforces a specific calling convention.
#define VK2D_APIENTRY

#endif



namespace vk2d {

constexpr double PI					= 3.14159265358979323846;
constexpr double RAD				= vk2d::PI * 2.0;
constexpr double KINDA_SMALL_VALUE	= 0.001;



/// @brief		Get size of a data type with exception for void which returns 0 size.
///				This is needed by a few templates that accept void as template parameter,
///				and because sizeof( void ) is an error.
/// @tparam		T
///				Data type
template<typename T> static constexpr size_t TypeSizeVoid0 = sizeof( T );
template<> static constexpr size_t TypeSizeVoid0<void> = 0;



} // vk2d



#if !defined( DOXYGEN )
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
			#define VK2D_APIENTRY				// Default calling convention for Linux, change in the future if needed.
	
		#else
			#error "Please add compiler support here!"
		#endif
	
	
	
	#else
		#error "Please add platform support here!"
	#endif
#endif



#if !defined( VK2D_API )
	#error "VK2D_API not defined!"
#endif

#if !defined( VK2D_APIENTRY )
	#error "VK2D_APIENTRY not defined!"
#endif
