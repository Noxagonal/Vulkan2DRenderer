
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Sampler.h"
#include "../Header/Impl/SamplerImpl.h"
#include "../Header/Impl/InstanceImpl.h"



vk2d::_internal::SamplerImpl::SamplerImpl(
	vk2d::Sampler					*	sampler_parent,
	vk2d::_internal::InstanceImpl	*	instance_parent,
	const vk2d::SamplerCreateInfo	&	create_info
)
{
	this->sampler_parent		= sampler_parent;
	this->instance_parent		= instance_parent;
	assert( this->sampler_parent );
	assert( this->instance_parent );

	vk_device					= instance_parent->GetVulkanDevice();
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
		instance_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Sampler parameter: 'vk2d::SamplerCreateInfo::magnification_filter'\n"
			"was none of 'vk2d::SamplerFilter' options,\n"
			"defaulting to 'vk2d::SamplerFilter::LINEAR'" );
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
		instance_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Sampler parameter: 'vk2d::SamplerCreateInfo::minification_filter'\n"
			"was none of 'vk2d::SamplerFilter' options,\n"
			"defaulting to 'vk2d::SamplerFilter::LINEAR'" );
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
		instance_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Sampler parameter: 'vk2d::SamplerCreateInfo::mipmap_mode'\n"
			"was none of 'vk2d::SamplerMipmapMode' options,\n"
			"defaulting to 'vk2d::SamplerMipmapMode::LINEAR'" );
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
		// addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;		// Handled in shader
		break;
	case vk2d::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		break;
	default:
		instance_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Sampler parameter: 'vk2d::SamplerCreateInfo::address_mode_u'\n"
			"was none of 'vk2d::SamplerAddressMode' options,\n"
			"defaulting to 'vk2d::SamplerAddressMode::REPEAT'" );
		addressModeU	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	}

	VkSamplerAddressMode addressModeV {};
	switch( create_info.address_mode_v ) {
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
		// addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;		// Handled in shader
		break;
	case vk2d::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		break;
	default:
		instance_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Sampler parameter: 'vk2d::SamplerCreateInfo::address_mode_v'\n"
			"was none of 'vk2d::SamplerAddressMode' options,\n"
			"defaulting to 'vk2d::SamplerAddressMode::REPEAT'" );
		addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	}

	VkBool32 anisotropyEnable = create_info.mipmap_enable;
	if( anisotropyEnable ) {
		if( create_info.minification_filter == vk2d::SamplerFilter::CUBIC ||
			create_info.magnification_filter == vk2d::SamplerFilter::CUBIC ) {
			instance_parent->Report(
				vk2d::ReportSeverity::WARNING,
				"Sampler parameter: 'vk2d::SamplerCreateInfo::mipmap_enable'\n"
				"was set to 'true' even though\n"
				"'vk2d::SamplerCreateInfo::minification_filter' or\n"
				"'vk2d::SamplerCreateInfo::minification_filter'\n"
				"was set to 'vk2d::SamplerFilter::CUBIC'.\n"
				"When using cubic filtering, mipmaps must be disabled.");
			anisotropyEnable	= false;
		}
	}

	float maxAnisotropy = std::min(
		create_info.mipmap_max_anisotropy,
		instance_parent->GetVulkanPhysicalDeviceProperties().limits.maxSamplerAnisotropy
	);

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
	sampler_create_info.borderColor					= VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK; // CHECK THIS WORKS!
	sampler_create_info.unnormalizedCoordinates		= VK_FALSE;
	auto result = vkCreateSampler(
		vk_device,
		&sampler_create_info,
		nullptr,
		&vk_sampler
	);
	if( result != VK_SUCCESS ) {
		instance_parent->Report( result, "Internal error: Cannot create Vulkan sampler!" );
		return;
	}

	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext					= nullptr;
	buffer_create_info.flags					= 0;
	buffer_create_info.size						= sizeof( vk2d::_internal::SamplerImpl::BufferData );
	buffer_create_info.usage					= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount	= 0;
	buffer_create_info.pQueueFamilyIndices		= nullptr;
	sampler_data = instance_parent->GetDeviceMemoryPool()->CreateCompleteBufferResource(
		&buffer_create_info,
		VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_CACHED_BIT
	);
	if( sampler_data != VK_SUCCESS ) {
		instance_parent->Report( sampler_data.result, "Internal error: Cannot create sampler data!" );
		return;
	}
	{
		border_color_enable.x		= uint32_t( create_info.address_mode_u == vk2d::SamplerAddressMode::CLAMP_TO_BORDER );
		border_color_enable.y		= uint32_t( create_info.address_mode_v == vk2d::SamplerAddressMode::CLAMP_TO_BORDER );

		vk2d::_internal::SamplerImpl::BufferData sd;
		sd.borderColor				= create_info.border_color;
		sd.borderColorEnable		= border_color_enable;
		sampler_data.memory.DataCopy( &sd, sizeof( vk2d::_internal::SamplerImpl::BufferData ) );
	}

	is_good			= true;
}

vk2d::_internal::SamplerImpl::~SamplerImpl()
{
	instance_parent->GetDeviceMemoryPool()->FreeCompleteResource( sampler_data );
	vkDestroySampler(
		vk_device,
		vk_sampler,
		nullptr
	);
//	instance_parent->GetDescriptorPool()->FreeDescriptorSet( descriptor_set );
}

VkSampler vk2d::_internal::SamplerImpl::GetVulkanSampler() const
{
	return vk_sampler;
}

//VkDescriptorSet vk2d::_internal::SamplerImpl::GetVulkanDescriptorSet()
//{
//	return descriptor_set.descriptorSet;
//}

VkBuffer vk2d::_internal::SamplerImpl::GetVulkanBufferForSamplerData() const
{
	return sampler_data.buffer;
}

vk2d::Vector2u vk2d::_internal::SamplerImpl::GetBorderColorEnable() const
{
	return border_color_enable;
}

bool vk2d::_internal::SamplerImpl::IsAnyBorderColorEnabled() const
{
	return bool( border_color_enable.x || border_color_enable.y );
}

bool vk2d::_internal::SamplerImpl::IsGood() const
{
	return is_good;
}
