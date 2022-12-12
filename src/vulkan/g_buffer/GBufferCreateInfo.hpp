#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vulkan {



struct GBufferCreateInfo
{
	size_t						image_count				= {};
	VkFormat					image_format			= {};
	VkExtent2D					extent					= {};
	bool						use_mip_maps			= {};
	VkSampleCountFlagBits		samples					= {};
	VkImageUsageFlags			usage_flags				= {};
};



} // vulkan
} // vk2d
