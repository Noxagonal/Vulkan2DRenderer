#pragma once

#include "../Core/SourceCommon.h"

namespace vk2d {

namespace _internal {



class SamplerImpl {
public:
	SamplerImpl(
		vk2d::Sampler					*	sampler,
		vk2d::_internal::RendererImpl	*	renderer,
		const vk2d::SamplerCreateInfo	&	create_info );

	~SamplerImpl();

	VkSampler								GetVulkanSampler();

	bool									IsGood();

private:
	vk2d::Sampler						*	sampler_parent		= {};
	vk2d::_internal::RendererImpl		*	renderer_parent		= {};
	VkDevice								vk_device			= {};

	VkSampler								sampler				= {};

	bool									is_good				= {};
};



} // _internal

} // vk2d
