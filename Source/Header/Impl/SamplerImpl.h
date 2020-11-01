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
		vk2d::_internal::InstanceImpl		*	instance,
		const vk2d::SamplerCreateInfo		&	create_info );

	~SamplerImpl();

	VkSampler									GetVulkanSampler() const;
	VkBuffer									GetVulkanBufferForSamplerData() const;
	vk2d::Vector2u								GetBorderColorEnable() const;
	bool										IsAnyBorderColorEnabled() const;

	bool										IsGood() const;



	struct BufferData {
		alignas( 16 )	vk2d::Colorf			borderColor			= {};	// Border color
		alignas( 8 )	vk2d::Vector2u			borderColorEnable	= {};	// Border color enable
	};

private:
	vk2d::Sampler							*	sampler_parent		= {};
	vk2d::_internal::InstanceImpl			*	instance		= {};
	VkDevice									vk_device			= {};

	VkSampler									vk_sampler			= {};
	vk2d::_internal::CompleteBufferResource		sampler_data		= {};

	vk2d::Vector2u								border_color_enable	= {};

	bool										is_good				= {};
};



} // _internal

} // vk2d
