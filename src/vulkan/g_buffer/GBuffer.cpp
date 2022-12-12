
#include <core/SourceCommon.hpp>
#include "GBuffer.hpp"

#include <interface/instance/InstanceImpl.hpp>

#include <vulkan/utils/Format.hpp>
#include <vulkan/utils/MipMaps.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::GBuffer::GBuffer(
	vk2d_internal::InstanceImpl				&	instance,
	GBufferCreateInfo						&	create_info
) :
	instance( instance ),
	create_info( create_info )
{
	auto & memory_pool = instance.GetVulkanDevice().GetDeviceMemoryPool();
	auto mip_levels = CalculateMipLevels( create_info.extent );

	auto CreateImage = [ &memory_pool, &mip_levels, &create_info ]()
	{
		auto image_create_info = VkImageCreateInfo();
		image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.pNext						= nullptr;
		image_create_info.flags						= 0;
		image_create_info.imageType					= VK_IMAGE_TYPE_2D;
		image_create_info.format					= create_info.image_format;
		image_create_info.extent					= { create_info.extent.width, create_info.extent.height, 1 };
		image_create_info.mipLevels					= mip_levels.size();
		image_create_info.arrayLayers				= 1;
		image_create_info.samples					= create_info.samples;
		image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage						= create_info.usage_flags;
		image_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.queueFamilyIndexCount		= 0;
		image_create_info.pQueueFamilyIndices		= nullptr;
		image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;

		auto image_view_create_info = VkImageViewCreateInfo();
		image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext				= nullptr;
		image_view_create_info.flags				= 0;
		image_view_create_info.image				= VK_NULL_HANDLE;
		image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format				= create_info.image_format;
		image_view_create_info.components			= {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		image_view_create_info.subresourceRange.aspectMask		= GetImageAspectFromFormat( create_info.image_format );
		image_view_create_info.subresourceRange.baseMipLevel	= 0;
		image_view_create_info.subresourceRange.levelCount		= mip_levels.size();
		image_view_create_info.subresourceRange.baseArrayLayer	= 0;
		image_view_create_info.subresourceRange.layerCount		= 1;

		return memory_pool.CreateCompleteImageResource(
			&image_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&image_view_create_info
		);
	};

	image_resources.reserve( create_info.image_count );
	for( size_t i = 0; i < create_info.image_count; i++ )
	{
		image_resources.push_back( CreateImage() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::GBuffer::~GBuffer()
{
	for( auto & r : image_resources )
	{
		instance.GetVulkanDevice().GetDeviceMemoryPool().FreeCompleteResource( r );
	}
	image_resources.clear();
}
