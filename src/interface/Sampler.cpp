
#include "core/SourceCommon.h"

#include "interface/InstanceImpl.h"

#include "interface/Sampler.h"
#include "interface/SamplerImpl.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::Sampler::Sampler(
	vk2d::_internal::InstanceImpl			*	instance,
	const vk2d::SamplerCreateInfo			&	create_info
)
{
	impl			= std::make_unique<vk2d::_internal::SamplerImpl>(
		this,
		instance,
		create_info
	);

	if( !impl | !impl->IsGood() ) {
		impl		= nullptr;
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create sampler implementation!" );
	}
}

VK2D_API vk2d::Sampler::~Sampler()
{}

VK2D_API bool VK2D_APIENTRY vk2d::Sampler::IsGood() const
{
	return !!impl;
}







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







vk2d::_internal::SamplerImpl::SamplerImpl(
	vk2d::Sampler					*	my_interface,
	vk2d::_internal::InstanceImpl	*	instance,
	const vk2d::SamplerCreateInfo	&	create_info
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	this->my_interface		= my_interface;
	this->instance		= instance;
	assert( this->my_interface );
	assert( this->instance );

	vk_device					= instance->GetVulkanDevice();
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
//		case vk2d::SamplerFilter::CUBIC:
//			magFilter	= VkFilter::VK_FILTER_CUBIC_EXT;
//			break;
		default:
			instance->Report(
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
//		case vk2d::SamplerFilter::CUBIC:
//			minFilter	= VkFilter::VK_FILTER_CUBIC_EXT;
//			break;
		default:
			instance->Report(
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
			instance->Report(
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
			instance->Report(
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
			instance->Report(
				vk2d::ReportSeverity::WARNING,
				"Sampler parameter: 'vk2d::SamplerCreateInfo::address_mode_v'\n"
				"was none of 'vk2d::SamplerAddressMode' options,\n"
				"defaulting to 'vk2d::SamplerAddressMode::REPEAT'" );
			addressModeV	= VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
	}

	VkBool32 anisotropyEnable = create_info.anisotropy_enable;
	/*
	// TODO: Sampler cubic filtering is disabled for now. Consider implementing this in shader later.
	// TODO: Consider making more custom sampler filters.
	if( anisotropyEnable ) {
		if( create_info.minification_filter == vk2d::SamplerFilter::CUBIC ||
			create_info.magnification_filter == vk2d::SamplerFilter::CUBIC ) {
			instance->Report(
				vk2d::ReportSeverity::WARNING,
				"Sampler parameter: 'vk2d::SamplerCreateInfo::mipmap_enable'\n"
				"was set to 'true' even though\n"
				"'vk2d::SamplerCreateInfo::minification_filter' or\n"
				"'vk2d::SamplerCreateInfo::minification_filter'\n"
				"was set to 'vk2d::SamplerFilter::CUBIC'.\n"
				"When using cubic filtering, mipmaps must be disabled." );
			anisotropyEnable	= false;
		}
	}
	*/

	float maxAnisotropy = std::min(
		create_info.mipmap_max_anisotropy,
		instance->GetVulkanPhysicalDeviceProperties().limits.maxSamplerAnisotropy
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
		instance->Report( result, "Internal error: Cannot create Vulkan sampler!" );
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
	sampler_data = instance->GetDeviceMemoryPool()->CreateCompleteBufferResource(
		&buffer_create_info,
		VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_CACHED_BIT
	);
	if( sampler_data != VK_SUCCESS ) {
		instance->Report( sampler_data.result, "Internal error: Cannot create sampler data!" );
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
	VK2D_ASSERT_MAIN_THREAD( instance );

	instance->GetDeviceMemoryPool()->FreeCompleteResource( sampler_data );
	vkDestroySampler(
		vk_device,
		vk_sampler,
		nullptr
	);
//	instance->GetDescriptorPool()->FreeDescriptorSet( descriptor_set );
}

VkSampler vk2d::_internal::SamplerImpl::GetVulkanSampler() const
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return vk_sampler;
}

VkBuffer vk2d::_internal::SamplerImpl::GetVulkanBufferForSamplerData() const
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return sampler_data.buffer;
}

glm::uvec2 vk2d::_internal::SamplerImpl::GetBorderColorEnable() const
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return border_color_enable;
}

bool vk2d::_internal::SamplerImpl::IsAnyBorderColorEnabled() const
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return bool( border_color_enable.x || border_color_enable.y );
}

bool vk2d::_internal::SamplerImpl::IsGood() const
{
	return is_good;
}
