#pragma once

#if defined( _WIN32 )
// We're compiling on windows.

#define VK2D_PLATFORM_WINDOWS		1

#define NOMINMAX
#include <Windows.h>



#elif defined( __linux__ ) && !defined( __ANDROID__ )
// We're compiling for linux (not android)

#define VK2D_PLATFORM_LINUX			1



#elif defined( __ANDROID__ )
// We're compiling for android

#define VK2D_PLATFORM_ANDROID		1



#elif defined( __APPLE__ )
// We're compiling for apple

#define VK2D_PLATFORM_APPLE			1



#else

#error "please add platform support here!"

#endif
