
#include <core/SourceCommon.hpp>

#include "TextureResourceImpl.hpp"

#include <system/ThreadPrivateResources.hpp>
#include <system/DescriptorSet.hpp>
#include <system/CommonTools.hpp>
#include <system/ImageFormatConverter.hpp>

#include <interface/Instance.hpp>
#include <interface/instance/InstanceImpl.hpp>

#include <interface/resources/ResourceManager.hpp>
#include <interface/resource_manager/ResourceManagerImpl.hpp>

#include <stb_image.h>



vk2d::vk2d_internal::TextureResourceImpl::TextureResourceImpl(
	TextureResource								&	my_interface,
	ResourceManagerImpl							&	resource_manager,
	uint32_t										loader_thread,
	ResourceBase								*	parent_resource,
	const std::vector<std::filesystem::path>	&	file_paths_listing
) :
	ResourceImplBase(
		my_interface,
		loader_thread,
		resource_manager,
		parent_resource,
		file_paths_listing
	),
	my_interface( my_interface ),
	resource_manager( resource_manager )
{
	is_good						= true;
}

vk2d::vk2d_internal::TextureResourceImpl::TextureResourceImpl(
	TextureResource									&	my_interface,
	ResourceManagerImpl								&	resource_manager,
	uint32_t											loader_thread,
	ResourceBase									*	parent_resource,
	glm::uvec2											size,
	const std::vector<const std::vector<Color8>*>	&	texels
) :
	ResourceImplBase(
		my_interface,
		loader_thread,
		resource_manager,
		parent_resource
	),
	my_interface( my_interface ),
	resource_manager( resource_manager ),
	extent( { size.x, size.y } )
{
	texture_data.resize( texels.size() );
	for( size_t i = 0; i < texels.size(); ++i ) {
		texture_data[ i ] = *( texels[ i ] );
	}

	is_good						= true;
}

bool vk2d::vk2d_internal::TextureResourceImpl::MTLoad(
	ThreadPrivateResource	*	thread_resource
)
{
	auto result = VK_SUCCESS;

	// 1. Load and process image from file.
	// 2. Create staging buffer, we'll also need memory pool for this.
	// 3. Create image and image view Vulkan objects.
	// 4. Allocate a command buffer from thread resources
	// 5. Record commands to upload image into the GPU.
	// 6. Record commands to make mipmaps of the image in the GPU.
	// 7. Make image available in a shader.
	// 8. Submit command buffer to the GPU, get a fence handle to indicate when the image is ready to be used.
	// 9. Allocate descriptor set that points to the image.

	loader_thread_resource	= dynamic_cast<ThreadLoaderResource*>( thread_resource );
	auto memory_pool		= loader_thread_resource->GetDeviceMemoryPool();

	assert( loader_thread_resource );
	if( !loader_thread_resource ) return false;

	// Get data into a staging buffer, and create staging buffer
	struct
	{
		uint32_t	x			= UINT32_MAX;
		uint32_t	y			= UINT32_MAX;
		uint32_t	channels	= 0;
	} image_info;

	auto & instance		= resource_manager.GetInstance();

	auto primary_render_queue		= instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_RENDER );
	auto secondary_render_queue		= instance.GetVulkanDevice().GetQueue( VulkanQueueType::SECONDARY_RENDER );
	auto primary_transfer_queue		= instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_TRANSFER );

	bool is_primary_render_needed				= secondary_render_queue.GetQueueFamilyIndex() != primary_render_queue.GetQueueFamilyIndex();

	if( IsFromFile() ) {
		// 1. Load and process images from files.

		for( auto & path : GetFilePaths() ) {
			// Create texture from a file
			int image_size_x				= 0;
			int image_size_y				= 0;
			int stbi_image_channel_count	= 0;
			uint32_t image_channel_count	= 4;

			auto stbi_image_data = stbi_load(
				path.string().c_str(),
				&image_size_x,
				&image_size_y,
				&stbi_image_channel_count,
				4 );
			if( !stbi_image_data ) {
				instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create texture: Cannot load image file: " + path.string() );
				return false;
			}

			// Check that file images have the same dimensions if we're creating array textures.
			if( image_info.x == UINT32_MAX ) {
				// First image
				image_info.x		= uint32_t( image_size_x );
				image_info.y		= uint32_t( image_size_y );
			} else {
				if( image_info.x != image_size_x ||
					image_info.y != image_size_y ) {
					instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create array texture: File images are different dimensions!" );
					stbi_image_free( stbi_image_data );
					return false;
				}
			}

			// 2. Create staging buffer, we'll also need memory pool for this.

			auto staging_buffer = memory_pool->CreateCompleteHostBufferResourceWithData(
				stbi_image_data,
				VkDeviceSize( image_size_x ) * VkDeviceSize( image_size_y ) * VkDeviceSize( image_channel_count ),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			);

			stbi_image_free( stbi_image_data );

			if( staging_buffer != VK_SUCCESS ) {
				instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource staging buffer!" );
				return false;
			}
			staging_buffers.push_back( std::move( staging_buffer ) );

			// Set image extent so we'll know it later
			extent				= { uint32_t( image_size_x ), uint32_t( image_size_y ) };

			image_info.x		= uint32_t( image_size_x );
			image_info.y		= uint32_t( image_size_y );
			image_info.channels	= uint32_t( image_channel_count );
		}
	} else {
		for( size_t i = 0; i < texture_data.size(); ++i ) {
			// Create texture from data

			// Image extent already set by resource manager, we can just use it.
			image_info.x		= extent.width;
			image_info.y		= extent.height;
			image_info.channels	= 4;

			if( texture_data[ i ].size() < size_t( image_info.x ) * size_t( image_info.y ) ) {
				instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create texture: Texture data too small for texture" );
				return false;
			}

			auto staging_buffer = memory_pool->CreateCompleteHostBufferResourceWithData(
				texture_data[ i ],
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			);

			if( staging_buffer != VK_SUCCESS ) {
				instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource staging buffer!" );
				return false;
			}
			staging_buffers.push_back( std::move( staging_buffer ) );
		}
	}

	image_layer_count		= uint32_t( staging_buffers.size() );
	if( !image_layer_count ) {
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load texture, nothing to do!" );
		return false;
	}

	// 3. Create image and image view Vulkan objects.
	auto mipmap_levels = GenerateMipSizes(
		glm::uvec2( image_info.x, image_info.y )
	);
	{
		VkImageCreateInfo image_create_info {};
		image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.pNext						= nullptr;
		image_create_info.flags						= 0;
		image_create_info.imageType					= VK_IMAGE_TYPE_2D;
		image_create_info.format					= VK_FORMAT_R8G8B8A8_UNORM;
		image_create_info.extent					= { image_info.x, image_info.y, 1 };
		image_create_info.mipLevels					= uint32_t( mipmap_levels.size() );
		image_create_info.arrayLayers				= image_layer_count;
		image_create_info.samples					= VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage						= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.queueFamilyIndexCount		= 0;
		image_create_info.pQueueFamilyIndices		= nullptr;
		image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext				= nullptr;
		image_view_create_info.flags				= 0;
		image_view_create_info.image				= VK_NULL_HANDLE;	// CreateCompleteImageResource() will replace this with proper image handle
		image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		image_view_create_info.format				= VK_FORMAT_R8G8B8A8_UNORM;
		image_view_create_info.components			= {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		image_view_create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel	= 0;
		image_view_create_info.subresourceRange.levelCount		= image_create_info.mipLevels;
		image_view_create_info.subresourceRange.baseArrayLayer	= 0;
		image_view_create_info.subresourceRange.layerCount		= image_layer_count;

		image = memory_pool->CreateCompleteImageResource(
			&image_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&image_view_create_info
		);
		if( image != VK_SUCCESS ) {
			instance.Report( image.result, "Internal error: Cannot create texture resource image!" );
			return false;
		}
	}

	// 4. Allocate a command buffer from thread resources
	{
		// Allocate render command buffer if needed.
		if( is_primary_render_needed ) {
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext					= nullptr;
			command_buffer_allocate_info.commandPool			= loader_thread_resource->GetPrimaryRenderCommandPool();
			command_buffer_allocate_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount		= 1;
			result = vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&vk_primary_render_command_buffer
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot allocate command buffers for texture data upload!" );
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			result = vkBeginCommandBuffer(
				vk_primary_render_command_buffer,
				&command_buffer_begin_info
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot record command buffers for texture data upload!" );
				return false;
			}
		} else {
			vk_primary_render_command_buffer	= VK_NULL_HANDLE;
		}

		// Allocate blit command buffer.
		{
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext					= nullptr;
			command_buffer_allocate_info.commandPool			= loader_thread_resource->GetSecondaryRenderCommandPool();
			command_buffer_allocate_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount		= 1;
			result = vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&vk_secondary_render_command_buffer
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot allocate blit command buffer for texture mip map processing!" );
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			result = vkBeginCommandBuffer(
				vk_secondary_render_command_buffer,
				&command_buffer_begin_info
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot begin command buffer for texture data upload!" );
				return false;
			}
		}

		// Allocate transfer command buffer.
		{
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext					= nullptr;
			command_buffer_allocate_info.commandPool			= loader_thread_resource->GetPrimaryTransferCommandPool();
			command_buffer_allocate_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount		= 1;
			result = vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&vk_primary_transfer_command_buffer
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot allocate command buffers for texture data upload!" );
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			result = vkBeginCommandBuffer(
				vk_primary_transfer_command_buffer,
				&command_buffer_begin_info
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot begin command buffers for texture data upload!" );
				return false;
			}
		}
	}

	// 5. Record commands to upload image into the GPU.
	// This gets very memory barrier heavy because of all the image layout transitions and
	// individually blitting mip levels from previously blitted mip level.
	{
		// 5.1 Record transfer commands.
		{
			// Transition image from undefined image layout to transfer dst optimal
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= 0;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
				image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_primary_transfer_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}

			// Copy to mip level 0, all layers
			for( uint32_t i = 0; i < image_layer_count; ++i ) {
				VkBufferImageCopy copy_region {};
				copy_region.bufferOffset					= 0;
				copy_region.bufferRowLength					= 0;
				copy_region.bufferImageHeight				= 0;
				copy_region.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				copy_region.imageSubresource.mipLevel		= 0;
				copy_region.imageSubresource.baseArrayLayer	= i;
				copy_region.imageSubresource.layerCount		= 1;
				copy_region.imageOffset						= { 0, 0, 0 };
				copy_region.imageExtent						= { image_info.x, image_info.y, 1 };
				vkCmdCopyBufferToImage(
					vk_primary_transfer_command_buffer,
					staging_buffers[ i ].buffer,
					image.image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy_region
				);
			}
		}

		// We might need to change image queue family ownership
		if( primary_transfer_queue.GetQueueFamilyIndex() != secondary_render_queue.GetQueueFamilyIndex() ) {
			// Ownership is transferred by writing the same pipeline barrier twice to
			// two different command buffers from two different families and setting
			// srcQueueFamilyIndex dstQueueFamilyIndex members to appropriate families.
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue.GetQueueFamilyIndex();
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue.GetQueueFamilyIndex();
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_primary_transfer_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue.GetQueueFamilyIndex();
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue.GetQueueFamilyIndex();
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_secondary_render_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}
		}

		// 6. Record commands to make mipmaps of the image in the GPU.
		{
			for( uint32_t current_mip_level = 1; current_mip_level < uint32_t( mipmap_levels.size() ); ++current_mip_level ) {
				auto src_mip_level							= current_mip_level - 1;
				auto dst_mip_level							= current_mip_level;
				auto src_mipmap_extent						= mipmap_levels[ src_mip_level ];
				auto dst_mipmap_extent						= mipmap_levels[ dst_mip_level ];

				// Transition current image layout for current mipmap level from transfer dst optimal to transfer src optimal
				{
					VkImageMemoryBarrier image_memory_barrier {};
					image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					image_memory_barrier.pNext								= nullptr;
					image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
					image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
					image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.image								= image.image;
					image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					image_memory_barrier.subresourceRange.baseMipLevel		= src_mip_level;
					image_memory_barrier.subresourceRange.levelCount		= 1;
					image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
					image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
					vkCmdPipelineBarrier(
						vk_secondary_render_command_buffer,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &image_memory_barrier
					);
				}

				// Blit here
				{
					VkImageBlit blit_region {};
					blit_region.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					blit_region.srcSubresource.mipLevel			= src_mip_level;
					blit_region.srcSubresource.baseArrayLayer	= 0;
					blit_region.srcSubresource.layerCount		= image_layer_count;
					blit_region.srcOffsets[ 0 ]					= { 0, 0, 0 };
					blit_region.srcOffsets[ 1 ]					= { int32_t( src_mipmap_extent.width ), int32_t( src_mipmap_extent.height ), 1 };
					blit_region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					blit_region.dstSubresource.mipLevel			= dst_mip_level;
					blit_region.dstSubresource.baseArrayLayer	= 0;
					blit_region.dstSubresource.layerCount		= image_layer_count;
					blit_region.dstOffsets[ 0 ]					= { 0, 0, 0 };
					blit_region.dstOffsets[ 1 ]					= { int32_t( dst_mipmap_extent.width ), int32_t( dst_mipmap_extent.height ), 1 };

					vkCmdBlitImage(
						vk_secondary_render_command_buffer,
						image.image,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						image.image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, &blit_region,
						VK_FILTER_LINEAR
					);
				}

				// 7. Make image available in a shader.
				// Transition current image layout for current mipmap level from transfer src optimal to shader read only optimal
				{
					VkImageMemoryBarrier image_memory_barrier {};
					image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					image_memory_barrier.pNext								= nullptr;
					image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
					image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
					image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.image								= image.image;
					image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					image_memory_barrier.subresourceRange.baseMipLevel		= src_mip_level;
					image_memory_barrier.subresourceRange.levelCount		= 1;
					image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
					image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
					vkCmdPipelineBarrier(
						vk_secondary_render_command_buffer,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &image_memory_barrier
					);
				}
			}

			// Lastly we'll transition current image layout for last mipmap level from transfer src optimal to shader read only optimal
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
				image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= uint32_t( mipmap_levels.size() - 1 );
				image_memory_barrier.subresourceRange.levelCount		= 1;
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_secondary_render_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}
		}

		// We might need to change image queue family ownership
		if( secondary_render_queue.GetQueueFamilyIndex() != primary_render_queue.GetQueueFamilyIndex() ) {
			// Ownership is transferred by writing the same pipeline barrier twice to
			// two different command buffers from two different families and setting
			// srcQueueFamilyIndex dstQueueFamilyIndex members to appropriate families.
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue.GetQueueFamilyIndex();
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue.GetQueueFamilyIndex();
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_secondary_render_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}
			{
				VkImageMemoryBarrier image_memory_barrier {};
				image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_memory_barrier.pNext								= nullptr;
				image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
				image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue.GetQueueFamilyIndex();
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue.GetQueueFamilyIndex();
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= image_layer_count;
				vkCmdPipelineBarrier(
					vk_primary_render_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}
		}
	}

	// Create synchronization primitives
	{
		VkSemaphoreCreateInfo semaphore_create_info {};
		semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext		= nullptr;
		semaphore_create_info.flags		= 0;

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType			= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext			= nullptr;
		fence_create_info.flags			= 0;

		result = vkCreateSemaphore(
			loader_thread_resource->GetVulkanDevice(),
			&semaphore_create_info,
			nullptr,
			&vk_transfer_semaphore
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create semaphore for texture data upload synchronization!" );
			return false;
		}

		if( is_primary_render_needed ) {
			auto result = vkCreateSemaphore(
				loader_thread_resource->GetVulkanDevice(),
				&semaphore_create_info,
				nullptr,
				&vk_blit_semaphore
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot create semaphore for texture data upload synchronization!" );
				return false;
			}
		}

		result = vkCreateFence(
			loader_thread_resource->GetVulkanDevice(),
			&fence_create_info,
			nullptr,
			&vk_texture_complete_fence
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create fence for texture data upload synchronization!" );
			return false;
		}
	}

	// 8. Submit command buffer to the GPU, get a fence handle to indicate when the image is ready to be used.
	{
		result = vkEndCommandBuffer(
			vk_primary_transfer_command_buffer
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot compile transfer command buffer for texture data upload!" );
			return false;
		}
		result = vkEndCommandBuffer(
			vk_secondary_render_command_buffer
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot compile secondary render queue command buffer for texture mipmap creation!" );
			return false;
		}
		if( is_primary_render_needed ) {
			auto result = vkEndCommandBuffer(
				vk_primary_render_command_buffer
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot compile primary render queue command buffer for texture queue family handover finalization!" );
				return false;
			}
		}

		// Submit transfer command buffer
		{
			VkSubmitInfo submit_info {};
			submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext					= nullptr;
			submit_info.waitSemaphoreCount		= 0;
			submit_info.pWaitSemaphores			= nullptr;
			submit_info.pWaitDstStageMask		= nullptr;
			submit_info.commandBufferCount		= 1;
			submit_info.pCommandBuffers			= &vk_primary_transfer_command_buffer;
			submit_info.signalSemaphoreCount	= 1;
			submit_info.pSignalSemaphores		= &vk_transfer_semaphore;

			auto result = primary_transfer_queue.Submit(
				submit_info,
				VK_NULL_HANDLE
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot submit texture upload command buffer!" );
				return false;
			}
		}

		// Submit blit command buffer
		{
			VkPipelineStageFlags wait_semaphore_dst	= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkSubmitInfo submit_info {};
			submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext					= nullptr;
			submit_info.waitSemaphoreCount		= 1;
			submit_info.pWaitSemaphores			= &vk_transfer_semaphore;
			submit_info.pWaitDstStageMask		= &wait_semaphore_dst;
			submit_info.commandBufferCount		= 1;
			submit_info.pCommandBuffers			= &vk_secondary_render_command_buffer;
			submit_info.signalSemaphoreCount	= is_primary_render_needed ? 1 : 0;
			submit_info.pSignalSemaphores		= is_primary_render_needed ? &vk_blit_semaphore : nullptr;
			auto result = secondary_render_queue.Submit(
				submit_info,
				is_primary_render_needed ? VK_NULL_HANDLE : vk_texture_complete_fence
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot submit texture mipmap generation command buffer!" );
				return false;
			}
		}

		// Submit primary render command buffer
		if( is_primary_render_needed ) {
			VkPipelineStageFlags wait_semaphore_dst	= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkSubmitInfo submit_info {};
			submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext					= nullptr;
			submit_info.waitSemaphoreCount		= 1;
			submit_info.pWaitSemaphores			= &vk_blit_semaphore;
			submit_info.pWaitDstStageMask		= &wait_semaphore_dst;
			submit_info.commandBufferCount		= 1;
			submit_info.pCommandBuffers			= &vk_primary_render_command_buffer;
			submit_info.signalSemaphoreCount	= 0;
			submit_info.pSignalSemaphores		= nullptr;
			auto result = primary_render_queue.Submit(
				submit_info,
				vk_texture_complete_fence
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot submit texture queue family handover command buffer!" );
				return false;
			}
		}
	}

	vk_image_layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return true;
}

void vk2d::vk2d_internal::TextureResourceImpl::MTUnload(
	ThreadPrivateResource	*	thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadLoaderResource*>( thread_resource );

	assert( loader_thread_resource );
	if( !loader_thread_resource ) return;

	auto memory_pool		= loader_thread_resource->GetDeviceMemoryPool();

	// Check if loaded successfully, no need to check for failure as this thread was
	// responsible for loading it, it's either loaded or failed to load but it'll
	// definitely be either or. MTUnload() does not ever get called before MTLoad().
	WaitUntilLoaded( std::chrono::nanoseconds::max() );

	vkDestroyFence(
		loader_thread_resource->GetVulkanDevice(),
		vk_texture_complete_fence,
		nullptr
	);

	vkDestroySemaphore(
		loader_thread_resource->GetVulkanDevice(),
		vk_transfer_semaphore,
		nullptr
	);
	vkDestroySemaphore(
		loader_thread_resource->GetVulkanDevice(),
		vk_blit_semaphore,
		nullptr
	);

	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetPrimaryRenderCommandPool(),
		1, &vk_primary_render_command_buffer
	);
	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetSecondaryRenderCommandPool(),
		1, &vk_secondary_render_command_buffer
	);
	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetPrimaryTransferCommandPool(),
		1, &vk_primary_transfer_command_buffer
	);

	memory_pool->FreeCompleteResource( image );
	for( auto & sb : staging_buffers ) {
		memory_pool->FreeCompleteResource( sb );
	}
	staging_buffers.clear();
}

vk2d::ResourceStatus vk2d::vk2d_internal::TextureResourceImpl::GetStatus()
{
	if( !is_good ) return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED ) {

		if( load_function_run_fence.IsSet() ) {

			// We can check the status of the fence in any thread,
			// it will not be removed until the resource is removed.
			assert( vk_texture_complete_fence );

			auto result = vkGetFenceStatus(
				resource_manager.GetVulkanDevice(),
				vk_texture_complete_fence
			);
			if( result == VK_SUCCESS ) {
				// Loaded, free some resources used to load
				status = local_status = ResourceStatus::LOADED;
				ScheduleTextureLoadResourceDestruction();
			} else if( result == VK_NOT_READY ) {
				return local_status;
			} else {
				status = local_status = ResourceStatus::FAILED_TO_LOAD;
			}
		}
	}

	return local_status;
}

vk2d::ResourceStatus vk2d::vk2d_internal::TextureResourceImpl::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	if( timeout == std::chrono::nanoseconds::max() ) {
		return WaitUntilLoaded( std::chrono::steady_clock::time_point::max() );
	}
	return WaitUntilLoaded( std::chrono::steady_clock::now() + timeout );
}

vk2d::ResourceStatus vk2d::vk2d_internal::TextureResourceImpl::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	// Make sure timeout is in the future.
	assert( timeout == std::chrono::steady_clock::time_point::max() ||
		timeout + std::chrono::seconds( 5 ) >= std::chrono::steady_clock::now() );

	if( !is_good ) return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED ) {

		if( load_function_run_fence.Wait( timeout ) ) {

			// We can check the status of the fence in any thread,
			// it will not be removed until the resource is removed.

			auto timeout_for_fences = ( timeout == std::chrono::steady_clock::time_point::max() ) ?
				UINT64_MAX :
				uint64_t( std::chrono::duration_cast<std::chrono::nanoseconds>( timeout - std::chrono::steady_clock::now() ).count() );

			assert( vk_texture_complete_fence );
			auto result = vkWaitForFences(
				resource_manager.GetVulkanDevice(),
				1, &vk_texture_complete_fence,
				VK_TRUE,
				timeout_for_fences
			);
			if( result == VK_SUCCESS ) {
				status = local_status = ResourceStatus::LOADED;
				ScheduleTextureLoadResourceDestruction();
			} else if( result == VK_TIMEOUT ) {
				return local_status;
			} else {
				status = local_status = ResourceStatus::FAILED_TO_LOAD;
				ScheduleTextureLoadResourceDestruction();
			}
		} // Else timeout and return local_status.
	}

	return local_status;
}

VkImage vk2d::vk2d_internal::TextureResourceImpl::GetVulkanImage() const
{
	return image.image;
}

VkImageView vk2d::vk2d_internal::TextureResourceImpl::GetVulkanImageView() const
{
	return image.view;
}

VkImageLayout vk2d::vk2d_internal::TextureResourceImpl::GetVulkanImageLayout() const
{
	return vk_image_layout;
}

glm::uvec2 vk2d::vk2d_internal::TextureResourceImpl::GetSize() const
{
	return glm::uvec2( extent.width, extent.height );
}

uint32_t vk2d::vk2d_internal::TextureResourceImpl::GetLayerCount() const
{
	return image_layer_count;
}

bool vk2d::vk2d_internal::TextureResourceImpl::IsTextureDataReady()
{
	return GetStatus() == ResourceStatus::LOADED;
}

bool vk2d::vk2d_internal::TextureResourceImpl::IsGood() const
{
	return is_good;
}



namespace vk2d {
namespace vk2d_internal {

// Handles the texture destruction
class DestroyTextureLoadResources :
	public Task
{
public:
	DestroyTextureLoadResources(
		TextureResourceImpl * texture
	) :
		texture( texture )
	{};

	void operator()(
		ThreadPrivateResource * thread_resource )
	{

		vkDestroyFence(
			texture->resource_manager.GetVulkanDevice(),
			texture->vk_texture_complete_fence,
			nullptr
		);

		vkDestroySemaphore(
			texture->resource_manager.GetVulkanDevice(),
			texture->vk_transfer_semaphore,
			nullptr
		);
		vkDestroySemaphore(
			texture->resource_manager.GetVulkanDevice(),
			texture->vk_blit_semaphore,
			nullptr
		);

		vkFreeCommandBuffers(
			texture->resource_manager.GetVulkanDevice(),
			texture->loader_thread_resource->GetPrimaryRenderCommandPool(),
			1, &texture->vk_primary_render_command_buffer
		);
		vkFreeCommandBuffers(
			texture->resource_manager.GetVulkanDevice(),
			texture->loader_thread_resource->GetSecondaryRenderCommandPool(),
			1, &texture->vk_secondary_render_command_buffer
		);
		vkFreeCommandBuffers(
			texture->resource_manager.GetVulkanDevice(),
			texture->loader_thread_resource->GetPrimaryTransferCommandPool(),
			1, &texture->vk_primary_transfer_command_buffer
		);

		for( auto & sb : texture->staging_buffers ) {
			texture->resource_manager.GetInstance().GetVulkanDevice().GetDeviceMemoryPool()->FreeCompleteResource(
				sb
			);
		}

		texture->vk_texture_complete_fence			= VK_NULL_HANDLE;
		texture->vk_transfer_semaphore				= VK_NULL_HANDLE;
		texture->vk_blit_semaphore					= VK_NULL_HANDLE;
		texture->vk_primary_render_command_buffer	= VK_NULL_HANDLE;
		texture->vk_secondary_render_command_buffer	= VK_NULL_HANDLE;
		texture->vk_primary_transfer_command_buffer	= VK_NULL_HANDLE;
		texture->staging_buffers.clear();
	}

private:
	TextureResourceImpl		*	texture;
};

} // vk2d_internal
} // vk2d

void vk2d::vk2d_internal::TextureResourceImpl::ScheduleTextureLoadResourceDestruction()
{
	resource_manager.GetThreadPool().ScheduleTask(
		std::make_unique<DestroyTextureLoadResources>( this ),
		{ GetLoaderThread() }
	);
}
