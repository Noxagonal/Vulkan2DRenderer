#pragma once

#ifdef _WIN32

#define VK_USE_PLATFORM_WIN32_KHR		1
#define NOMINMAX
#include <Windows.h>

#else

#error "please add platform support here!"

#endif
