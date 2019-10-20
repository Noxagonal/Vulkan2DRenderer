#pragma once

#ifdef _WIN32

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR		1

#else

#error "please add platform support here!"

#endif
