#pragma once

#include "../Core/SourceCommon.h"

#include "../Core/DescriptorSet.h"


namespace vk2d {
namespace _internal {



struct SamplerTextureDescriptorPoolData
{
	vk2d::_internal::PoolDescriptorSet						descriptor_set								= {};
	std::chrono::time_point<std::chrono::steady_clock>		previous_access_time						= {};	// For cleanup
};



} // _internal
} // vk2d
