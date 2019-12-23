#pragma once

#include "BuildOptions.h"
#include "Platform.h"



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
