#pragma once

#include "../Core/SourceCommon.h"

#include "../Core/DescriptorSet.h"


namespace vk2d {
namespace _internal {



class RenderTargetTextureImpl;



struct SamplerTextureDescriptorPoolData
{
	vk2d::_internal::PoolDescriptorSet						descriptor_set								= {};
	std::chrono::time_point<std::chrono::steady_clock>		previous_access_time						= {};	// For cleanup
};


struct RenderTargetTextureDependencyInfo
{
	vk2d::_internal::RenderTargetTextureImpl			*	render_target								= {};
	uint32_t												swap_buffer_index							= {};
};




} // _internal
} // vk2d
