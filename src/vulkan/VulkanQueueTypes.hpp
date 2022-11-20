#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vk2d_internal {



enum class VulkanQueueType : size_t {
	PRIMARY_RENDER = 0,
	SECONDARY_RENDER,
	PRIMARY_COMPUTE,
	PRIMARY_TRANSFER,
};



} // vk2d_internal
} // vk2d
