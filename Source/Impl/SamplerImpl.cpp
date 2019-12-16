
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Sampler.h"
#include "../Header/Impl/SamplerImpl.h"
#include "../Header/Impl/RendererImpl.h"



vk2d::_internal::SamplerImpl::SamplerImpl(
	vk2d::Sampler					*	sampler_parent,
	vk2d::_internal::RendererImpl	*	renderer_parent,
	const vk2d::SamplerCreateInfo	&	create_info
)
{
	this->sampler_parent		= sampler_parent;
	this->renderer_parent		= renderer_parent;
	assert( this->sampler_parent );
	assert( this->renderer_parent );

	vk_device					= renderer_parent->GetVulkanDevice();
	assert( vk_device );

	// Error report here if anything is nullptr or VK_NULL_HANDLE;

	VkFilter magFilter {};
	switch( create_info.magnification_filter ) {
	case vk2d::SamplerFilter::NEAREST:
		magFilter	= VkFilter::VK_FILTER_NEAREST;
		break;
	case vk2d::SamplerFilter::LINEAR:
		magFilter	= VkFilter::VK_FILTER_LINEAR;
		break;
	case vk2d::SamplerFilter::CUBIC:
		magFilter	= VkFilter::VK_FILTER_CUBIC_EXT;
		break;
	default:
		// Error report here
		magFilter	= VkFilter::VK_FILTER_LINEAR;
		break;
	}

	VkFilter minFilter {};
	switch( create_info.minification_filter ) {
	case vk2d::SamplerFilter::NEAREST:
		minFilter	= VkFilter::VK_FILTER_NEAREST;
		break;
	case vk2d::SamplerFilter::LINEAR:
		minFilter	= VkFilter::VK_FILTER_LINEAR;
		break;
	case vk2d::SamplerFilter::CUBIC:
		minFilter	= VkFilter::VK_FILTER_CUBIC_EXT;
		break;
	default:
		// Error report here
		minFilter	= VkFilter::VK_FILTER_LINEAR;
		break;
	}

	VkSamplerMipmapMode mipmapMode {};
	switch( create_info.mipmap_mode ) {
	case vk2d::SamplerMipmapMode::NEAREST:
		mipmapMode	= VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case vk2d::SamplerMipmapMode::LINEAR:
		mipmapMode	= VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	default:
		// Error report here
		mipmapMode	= VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	}

	VkSamplerAddressMode addressModeU {};
	switch( create_info.address_mode_u ) {
	case vk2d::SamplerAddressMode::REPEAT:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case vk2d::SamplerAddressMode::MIRRORED_REPEAT:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		break;
	case vk2d::SamplerAddressMode::CLAMP_TO_EDGE:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case vk2d::SamplerAddressMode::CLAMP_TO_BORDER:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	case vk2d::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		break;
	default:
		// Error report here
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	}

	VkSamplerAddressMode addressModeV {};
	switch( create_info.address_mode_u ) {
	case vk2d::SamplerAddressMode::REPEAT:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case vk2d::SamplerAddressMode::MIRRORED_REPEAT:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		break;
	case vk2d::SamplerAddressMode::CLAMP_TO_EDGE:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case vk2d::SamplerAddressMode::CLAMP_TO_BORDER:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	case vk2d::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		break;
	default:
		// Error report here
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	}

	VkBool32 anisotropyEnable = create_info.mipmap_enable;
	if( anisotropyEnable ) {
		if( create_info.minification_filter == vk2d::SamplerFilter::CUBIC ||
			create_info.magnification_filter == vk2d::SamplerFilter::CUBIC ) {
			// Error report here
			anisotropyEnable	= false;
		}
	}

	float maxAnisotropy = std::min(
		create_info.mipmap_max_anisotropy,
		renderer_parent->GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy
	);

	VkBorderColor borderColor {};
	switch( create_info.border_color ) {
	case vk2d::SamplerBorderColor::BLACK_TRANSPARENT:
		borderColor		= VkBorderColor::VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		break;
	case vk2d::SamplerBorderColor::BLACK_OPAQUE:
		borderColor		= VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		break;
	case vk2d::SamplerBorderColor::WHITE_OPAQUE:
		borderColor		= VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		break;
	default:
		// Error report here
		borderColor		= VkBorderColor::VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		break;
	}

	VkSamplerCreateInfo sampler_create_info {};
	sampler_create_info.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.pNext						= nullptr;
	sampler_create_info.flags						= 0;
	sampler_create_info.magFilter					= magFilter;
	sampler_create_info.minFilter					= minFilter;
	sampler_create_info.mipmapMode					= mipmapMode;
	sampler_create_info.addressModeU				= addressModeU;
	sampler_create_info.addressModeV				= addressModeV;
	sampler_create_info.addressModeW				= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.mipLodBias					= create_info.mipmap_level_of_detail_bias;
	sampler_create_info.anisotropyEnable			= anisotropyEnable;
	sampler_create_info.maxAnisotropy				= maxAnisotropy;
	sampler_create_info.compareEnable				= VK_FALSE;
	sampler_create_info.compareOp					= VkCompareOp::VK_COMPARE_OP_NEVER;
	sampler_create_info.minLod						= create_info.mipmap_min_level_of_detail;
	sampler_create_info.maxLod						= create_info.mipmap_max_level_of_detail;
	sampler_create_info.borderColor					= borderColor;
	sampler_create_info.unnormalizedCoordinates		= VK_FALSE;
	if( vkCreateSampler(
		vk_device,
		&sampler_create_info,
		nullptr,
		&sampler
	) != VK_SUCCESS ) {
		// Error report here
		return;
	}

	is_good			= true;
}

vk2d::_internal::SamplerImpl::~SamplerImpl()
{
	vkDestroySampler(
		vk_device,
		sampler,
		nullptr
	);
}

VkSampler vk2d::_internal::SamplerImpl::GetVulkanSampler()
{
	return sampler;
}

bool vk2d::_internal::SamplerImpl::IsGood()
{
	return is_good;
}
