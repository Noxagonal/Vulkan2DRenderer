#pragma once

#include <core/SourceCommon.hpp>

#include <system/DescriptorSet.hpp>
#include <interface/Sampler.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>

namespace vk2d {
namespace vk2d_internal {



class SamplerImpl {
public:
	SamplerImpl(
		Sampler								&	sampler,
		InstanceImpl						&	instance,
		const SamplerCreateInfo				&	create_info );

	~SamplerImpl();

	VkSampler									GetVulkanSampler() const;
	VkBuffer									GetVulkanBufferForSamplerData() const;
	glm::uvec2									GetBorderColorEnable() const;
	bool										IsAnyBorderColorEnabled() const;

	bool										IsGood() const;



	struct BufferData {
		alignas( 16 )	Colorf					borderColor			= {};	// Border color
		alignas( 8 )	glm::uvec2				borderColorEnable	= {};	// Border color enable
	};

private:
	Sampler									&	my_interface;
	InstanceImpl							&	instance;
	VkDevice									vk_device			= {};

	VkSampler									vk_sampler			= {};
	CompleteBufferResource						sampler_data		= {};

	glm::uvec2									border_color_enable	= {};

	bool										is_good				= {};
};



} // vk2d_internal

} // vk2d
