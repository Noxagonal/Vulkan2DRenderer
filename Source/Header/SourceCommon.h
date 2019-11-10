#pragma once

#define VK2D_LIBRARY_EXPORT				1

#include "../../Include/vk2d/Common.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

namespace vk2d {

namespace _internal {

struct ResolvedQueue {
	VkQueue									queue						= {};	// VkQueue handle.
	uint32_t								queueFamilyIndex			= {};	// Typical queueFamilyIndex.
	VkBool32								supportsPresentation		= {};	// VK_TRUE if you can present using this queue, VK_FALSE if you can not.
	uint32_t								basedOn						= {};	// Which other queue this one is based off.
};

} // _internal

} // vk2d
