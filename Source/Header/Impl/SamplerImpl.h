#pragma once

#include "../Core/SourceCommon.h"

#include "../Core/DescriptorSet.h"
#include "../Core/VulkanMemoryManagement.h"

namespace vk2d {

namespace _internal {



class SamplerImpl {
public:
	SamplerImpl(
		vk2d::Sampler						*	sampler,
		vk2d::_internal::RendererImpl		*	renderer,
		const vk2d::SamplerCreateInfo		&	create_info );

	~SamplerImpl();

	VkSampler									GetVulkanSampler();
//	VkDescriptorSet								GetVulkanDescriptorSet();
	VkBuffer									GetVulkanBufferForSamplerData();

	bool										IsGood();



	struct BufferData {
		alignas( 16 ) vk2d::Colorf				borderColor			= {};
		alignas( 16 ) std::array<float, 4>		borderColorEnable	= {};
	};

private:
	vk2d::Sampler							*	sampler_parent		= {};
	vk2d::_internal::RendererImpl			*	renderer_parent		= {};
	VkDevice									vk_device			= {};

	VkSampler									sampler				= {};
//	vk2d::_internal::PoolDescriptorSet			descriptor_set		= {};

	vk2d::_internal::CompleteBufferResource		sampler_data		= {};

	bool										is_good				= {};
};



} // _internal

} // vk2d
