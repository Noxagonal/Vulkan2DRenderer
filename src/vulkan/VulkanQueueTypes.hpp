#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vulkan {



enum class QueueType : size_t {
	PRIMARY_RENDER = 0,
	SECONDARY_RENDER,
	PRIMARY_COMPUTE,
	PRIMARY_TRANSFER,
};



} // vulkan
} // vk2d
