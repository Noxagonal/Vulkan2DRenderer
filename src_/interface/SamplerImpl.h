#pragma once

#include "core/SourceCommon.h"

#include "system/DescriptorSet.h"
#include "system/VulkanMemoryManagement.h"

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
	glm::uvec2									GetBorderColorEnable() const;
	bool										IsAnyBorderColorEnabled() const;

	bool										IsGood() const;



	struct BufferData {
		alignas( 16 )	vk2d::Colorf			borderColor			= {};	// Border color
		alignas( 8 )	glm::uvec2				borderColorEnable	= {};	// Border color enable
	};

private:
	vk2d::Sampler							*	my_interface		= {};
	vk2d::_internal::InstanceImpl			*	instance			= {};
	VkDevice									vk_device			= {};

	VkSampler									vk_sampler			= {};
	vk2d::_internal::CompleteBufferResource		sampler_data		= {};

	glm::uvec2									border_color_enable	= {};

	bool										is_good				= {};
};



} // _internal

} // vk2d
