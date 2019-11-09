
#include "../Header/SourceCommon.h"

#include "../Header/TextureResourceImpl.h"
#include "../Header/ResourceManagerImpl.h"
#include "../Header/ThreadPrivateResources.h"
#include "../Header/DescriptorSet.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assert.h>
#include <thread>



namespace vk2d {

namespace _internal {





TextureResourceImpl::TextureResourceImpl(
	TextureResource					*	texture_resource_parent,
	_internal::ResourceManagerImpl	*	resource_manager
)
{
	this->texture				= texture_resource_parent;
	this->resource_manager		= resource_manager;
	assert( this->texture );
	assert( this->resource_manager );

	is_good						= true;
}

TextureResourceImpl::~TextureResourceImpl()
{
}

bool TextureResourceImpl::MTLoad(
	_internal::ThreadPrivateResource	*	thread_resource
)
{
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
	auto memory_pool		= resource_manager->GetRenderer()->GetDeviceMemoryPool();

	assert( loader_thread_resource );
	if( !loader_thread_resource ) return false;

	// Get data into a staging buffer, and create staging buffer
	struct {
		uint32_t	x;
		uint32_t	y;
		uint32_t	channels;
	} image_info;

	auto primary_render_queue_family_index		= resource_manager->GetRenderer()->GetPrimaryRenderQueue().queueFamilyIndex;
	auto secondary_render_queue_family_index	= resource_manager->GetRenderer()->GetSecondaryRenderQueue().queueFamilyIndex;
	auto primary_transfer_queue_family_index	= resource_manager->GetRenderer()->GetPrimaryTransferQueue().queueFamilyIndex;

	bool is_primary_render_needed				= secondary_render_queue_family_index != primary_render_queue_family_index;

	{
		// 1. Load and process image from file.

		if( texture->IsFromFile() ) {
			// Create texture from a file

			auto mypath = std::filesystem::current_path() / texture->GetFilePath();

			int image_size_x			= 0;
			int image_size_y			= 0;
			int image_channel_count		= 0;

			auto image_data = stbi_load(
				texture->GetFilePath().string().c_str(),
				&image_size_x,
				&image_size_y,
				&image_channel_count,
				4 );
			if( !image_data ) return false;
			assert( image_channel_count == 4 );

			// 2. Create staging buffer, we'll also need memory pool for this.

			staging_buffer = memory_pool->CreateCompleteHostBufferResourceWithData(
				image_data,
				VkDeviceSize( image_size_x * image_size_x * image_channel_count ),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			);

			stbi_image_free( image_data );

			if( staging_buffer != VK_SUCCESS ) {
				return false;
			}

			image_info.x		= uint32_t( image_size_x );
			image_info.y		= uint32_t( image_size_y );
			image_info.channels	= uint32_t( image_channel_count );

		} else {
			// Create texture from data
			// TODO
			assert( 0 );
		}
	}

	// 3. Create image and image view Vulkan objects.
	std::vector<VkExtent2D> mipmap_levels;
	{
		// generate mipmap levels
		mipmap_levels.reserve( 32 );
		{
			VkExtent2D last { image_info.x, image_info.y };
			mipmap_levels.push_back( last );

			while( last.width != 1 && last.height != 1 ) {
				VkExtent2D current_dim_size	= { last.width / 2, last.height / 2 };
				if( current_dim_size.width < 1 )	current_dim_size.width = 1;
				if( current_dim_size.height < 1 )	current_dim_size.height = 1;
				last	= current_dim_size;
				mipmap_levels.push_back( current_dim_size );
			}
		}

		VkImageCreateInfo image_create_info {};
		image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.pNext						= nullptr;
		image_create_info.flags						= 0;
		image_create_info.imageType					= VK_IMAGE_TYPE_2D;
		image_create_info.format					= VK_FORMAT_R8G8B8A8_UNORM;
		image_create_info.extent					= { image_info.x, image_info.y, 1 };
		image_create_info.mipLevels					= uint32_t( mipmap_levels.size() );
		image_create_info.arrayLayers				= 1;
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
		image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
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
		image_view_create_info.subresourceRange.layerCount		= 1;

		image = memory_pool->CreateCompleteImageResource(
			&image_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&image_view_create_info
		);
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
			if( vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&primary_render_command_buffer
			) != VK_SUCCESS ) {
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			if( vkBeginCommandBuffer(
				primary_render_command_buffer,
				&command_buffer_begin_info
			) != VK_SUCCESS ) {
				return false;
			}
		} else {
			primary_render_command_buffer	= VK_NULL_HANDLE;
		}

		// Allocate blit command buffer.
		{
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext					= nullptr;
			command_buffer_allocate_info.commandPool			= loader_thread_resource->GetSecondaryRenderCommandPool();
			command_buffer_allocate_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount		= 1;
			if( vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&secondary_render_command_buffer
			) != VK_SUCCESS ) {
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			if( vkBeginCommandBuffer(
				secondary_render_command_buffer,
				&command_buffer_begin_info
			) != VK_SUCCESS ) {
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
			if( vkAllocateCommandBuffers(
				loader_thread_resource->GetVulkanDevice(),
				&command_buffer_allocate_info,
				&primary_transfer_command_buffer
			) != VK_SUCCESS ) {
				return false;
			}

			VkCommandBufferBeginInfo command_buffer_begin_info {};
			command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext				= nullptr;
			command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pInheritanceInfo	= nullptr;
			if( vkBeginCommandBuffer(
				primary_transfer_command_buffer,
				&command_buffer_begin_info
			) != VK_SUCCESS ) {
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
				image_memory_barrier.subresourceRange.layerCount		= 1;
				vkCmdPipelineBarrier(
					primary_transfer_command_buffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &image_memory_barrier
				);
			}

			// Copy to mip level 0
			VkBufferImageCopy copy_region {};
			copy_region.bufferOffset					= 0;
			copy_region.bufferRowLength					= 0;
			copy_region.bufferImageHeight				= 0;
			copy_region.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.imageSubresource.mipLevel		= 0;
			copy_region.imageSubresource.baseArrayLayer	= 0;
			copy_region.imageSubresource.layerCount		= 1;
			copy_region.imageOffset						= { 0, 0, 0 };
			copy_region.imageExtent						= { image_info.x, image_info.y, 1 };
			vkCmdCopyBufferToImage(
				primary_transfer_command_buffer,
				staging_buffer.buffer,
				image.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copy_region
			);
		}

		// We might need to change image queue family ownership
		if( primary_transfer_queue_family_index != secondary_render_queue_family_index ) {
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
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue_family_index;
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue_family_index;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= 1;
				vkCmdPipelineBarrier(
					primary_transfer_command_buffer,
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
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue_family_index;
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue_family_index;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= 1;
				vkCmdPipelineBarrier(
					secondary_render_command_buffer,
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
					image_memory_barrier.subresourceRange.layerCount		= 1;
					vkCmdPipelineBarrier(
						secondary_render_command_buffer,
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
					blit_region.srcSubresource.layerCount		= 1;
					blit_region.srcOffsets[ 0 ]					= { 0, 0, 0 };
					blit_region.srcOffsets[ 1 ]					= { int32_t( src_mipmap_extent.width ), int32_t(  src_mipmap_extent.height ), 1 };
					blit_region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					blit_region.dstSubresource.mipLevel			= dst_mip_level;
					blit_region.dstSubresource.baseArrayLayer	= 0;
					blit_region.dstSubresource.layerCount		= 1;
					blit_region.dstOffsets[ 0 ]					= { 0, 0, 0 };
					blit_region.dstOffsets[ 1 ]					= { int32_t( dst_mipmap_extent.width ), int32_t( dst_mipmap_extent.height ), 1 };

					vkCmdBlitImage(
						secondary_render_command_buffer,
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
					image_memory_barrier.subresourceRange.layerCount		= 1;
					vkCmdPipelineBarrier(
						secondary_render_command_buffer,
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
			image_memory_barrier.subresourceRange.layerCount		= 1;
			vkCmdPipelineBarrier(
				secondary_render_command_buffer,
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
		if( secondary_render_queue_family_index != primary_render_queue_family_index ) {
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
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue_family_index;
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue_family_index;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= 1;
				vkCmdPipelineBarrier(
					secondary_render_command_buffer,
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
				image_memory_barrier.srcQueueFamilyIndex				= primary_transfer_queue_family_index;
				image_memory_barrier.dstQueueFamilyIndex				= secondary_render_queue_family_index;
				image_memory_barrier.image								= image.image;
				image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				image_memory_barrier.subresourceRange.baseMipLevel		= 0;
				image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
				image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
				image_memory_barrier.subresourceRange.layerCount		= 1;
				vkCmdPipelineBarrier(
					primary_render_command_buffer,
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

		if( vkCreateSemaphore(
			loader_thread_resource->GetVulkanDevice(),
			&semaphore_create_info,
			nullptr,
			&transfer_semaphore
		) != VK_SUCCESS ) {
			return false;
		}

		if( is_primary_render_needed ) {
			if( vkCreateSemaphore(
				loader_thread_resource->GetVulkanDevice(),
				&semaphore_create_info,
				nullptr,
				&blit_semaphore
			) != VK_SUCCESS ) {
				return false;
			}
		}

		if( vkCreateFence(
			loader_thread_resource->GetVulkanDevice(),
			&fence_create_info,
			nullptr,
			&texture_complete_fence
		) != VK_SUCCESS ) {
			return false;
		}
	}

	// 8. Submit command buffer to the GPU, get a fence handle to indicate when the image is ready to be used.
	{
		if( vkEndCommandBuffer( primary_transfer_command_buffer ) != VK_SUCCESS ) {
			return false;
		}
		if( vkEndCommandBuffer( secondary_render_command_buffer ) != VK_SUCCESS ) {
			return false;
		}
		if( is_primary_render_needed ) {
			if( vkEndCommandBuffer( primary_render_command_buffer ) != VK_SUCCESS ) {
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
			submit_info.pCommandBuffers			= &primary_transfer_command_buffer;
			submit_info.signalSemaphoreCount	= 1;
			submit_info.pSignalSemaphores		= &transfer_semaphore;
			if( vkQueueSubmit(
				resource_manager->GetRenderer()->GetPrimaryTransferQueue().queue,
				1, &submit_info,
				VK_NULL_HANDLE
			) != VK_SUCCESS ) {
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
			submit_info.pWaitSemaphores			= &transfer_semaphore;
			submit_info.pWaitDstStageMask		= &wait_semaphore_dst;
			submit_info.commandBufferCount		= 1;
			submit_info.pCommandBuffers			= &secondary_render_command_buffer;
			submit_info.signalSemaphoreCount	= is_primary_render_needed ? 1 : 0;
			submit_info.pSignalSemaphores		= is_primary_render_needed ? &blit_semaphore : nullptr;
			if( vkQueueSubmit(
				resource_manager->GetRenderer()->GetSecondaryRenderQueue().queue,
				1, &submit_info,
				is_primary_render_needed ? VK_NULL_HANDLE : texture_complete_fence
			) != VK_SUCCESS ) {
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
			submit_info.pWaitSemaphores			= &blit_semaphore;
			submit_info.pWaitDstStageMask		= &wait_semaphore_dst;
			submit_info.commandBufferCount		= 1;
			submit_info.pCommandBuffers			= &primary_render_command_buffer;
			submit_info.signalSemaphoreCount	= 0;
			submit_info.pSignalSemaphores		= nullptr;
			if( vkQueueSubmit(
				resource_manager->GetRenderer()->GetPrimaryRenderQueue().queue,
				1, &submit_info,
				texture_complete_fence
			) != VK_SUCCESS ) {
				return false;
			}
		}
	}

	// 9. Allocate descriptor set that points to the image.
	{
		descriptor_set = loader_thread_resource->GetDescriptorAutoPool()->AllocateDescriptorSet(
			resource_manager->GetRenderer()->GetDescriptorSetLayout()
		);
		if( descriptor_set != VK_SUCCESS ) {
			return false;
		}
	}

	return true;
}

void TextureResourceImpl::MTUnload(
	_internal::ThreadPrivateResource	*	thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadLoaderResource*>( thread_resource );

	assert( loader_thread_resource );
	if( !loader_thread_resource ) return;

	auto memory_pool		= resource_manager->GetRenderer()->GetDeviceMemoryPool();

	// Check if loaded successfully, no need to check for failure as this thread was
	// responsible for loading it, it's either loaded or failed to load but it'll
	// definitely be either or. MTUnload() does not ever get called before MTLoad().
	texture->WaitUntilLoaded();

	loader_thread_resource->GetDescriptorAutoPool()->FreeDescriptorSet(
		descriptor_set
	);

	vkDestroyFence(
		loader_thread_resource->GetVulkanDevice(),
		texture_complete_fence,
		nullptr
	);

	vkDestroySemaphore(
		loader_thread_resource->GetVulkanDevice(),
		transfer_semaphore,
		nullptr
	);
	vkDestroySemaphore(
		loader_thread_resource->GetVulkanDevice(),
		blit_semaphore,
		nullptr
	);

	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetPrimaryRenderCommandPool(),
		1, &primary_render_command_buffer
	);
	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetSecondaryRenderCommandPool(),
		1, &secondary_render_command_buffer
	);
	vkFreeCommandBuffers(
		loader_thread_resource->GetVulkanDevice(),
		loader_thread_resource->GetPrimaryTransferCommandPool(),
		1, &primary_transfer_command_buffer
	);

	memory_pool->FreeCompleteResource( image );
	memory_pool->FreeCompleteResource( staging_buffer );
}

bool TextureResourceImpl::IsLoaded()
{
	std::unique_lock<std::mutex>		is_loaded_lock( is_loaded_mutex, std::defer_lock );
	if( !is_loaded_lock.try_lock() ) {
		return false;
	}

	if( is_loaded )						return true;
	if( !is_good )						return false;
	if( !texture->load_function_ran )	return false;
	if( texture->FailedToLoad() )		return false;

	// We can check the status of the fence in any thread,
	// it will not be removed until the resource is removed.
	assert( texture_complete_fence );
	auto result = vkGetFenceStatus(
		resource_manager->GetVulkanDevice(),
		texture_complete_fence
	);
	if( result == VK_SUCCESS ) {
		// Loaded, free some resources used to load
		is_loaded							= true;
		ScheduleTextureLoadResourceDestruction();
		return true;

	} else if( result == VK_NOT_READY ) {
		return false;

	} else {
		texture->failed_to_load	= true;
		return false;
	};

	return false;
}

bool TextureResourceImpl::WaitUntilLoaded()
{
	std::lock_guard<std::mutex> is_loaded_lock( is_loaded_mutex );

	if( is_loaded )						return true;
	if( !is_good )						return false;
	while( !texture->load_function_ran ) {
		// Semi busy loop for now.
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
	}
	if( texture->FailedToLoad() )		return false;

	// We can check the status of the fence in any thread,
	// it will not be removed until the resource is removed.
	assert( texture_complete_fence );
	auto result = vkWaitForFences(
		resource_manager->GetVulkanDevice(),
		1, &texture_complete_fence,
		VK_TRUE,
		UINT64_MAX
	);
	if( result == VK_SUCCESS ) {
		// Loaded, free some resources used to load
		is_loaded							= true;
		ScheduleTextureLoadResourceDestruction();
		return true;

	} else {
		texture->failed_to_load	= true;
		return false;
	};

	return false;
}

bool TextureResourceImpl::IsGood()
{
	return is_good;
}



// Handles the texture destruction
class DestroyTextureLoadResources :
	public vk2d::_internal::Task {
public:
	DestroyTextureLoadResources(
		vk2d::_internal::TextureResourceImpl * texture
	) :
		texture( texture )
	{};

	void operator()(
		vk2d::_internal::ThreadPrivateResource * thread_resource )
	{

		vkDestroyFence(
			texture->resource_manager->GetVulkanDevice(),
			texture->texture_complete_fence,
			nullptr
		);

		vkDestroySemaphore(
			texture->resource_manager->GetVulkanDevice(),
			texture->transfer_semaphore,
			nullptr
		);
		vkDestroySemaphore(
			texture->resource_manager->GetVulkanDevice(),
			texture->blit_semaphore,
			nullptr
		);

		vkFreeCommandBuffers(
			texture->resource_manager->GetVulkanDevice(),
			texture->loader_thread_resource->GetPrimaryRenderCommandPool(),
			1, &texture->primary_render_command_buffer
		);
		vkFreeCommandBuffers(
			texture->resource_manager->GetVulkanDevice(),
			texture->loader_thread_resource->GetSecondaryRenderCommandPool(),
			1, &texture->secondary_render_command_buffer
		);
		vkFreeCommandBuffers(
			texture->resource_manager->GetVulkanDevice(),
			texture->loader_thread_resource->GetPrimaryTransferCommandPool(),
			1, &texture->primary_transfer_command_buffer
		);

		texture->resource_manager->GetRenderer()->GetDeviceMemoryPool()->FreeCompleteResource(
			texture->staging_buffer
		);

		texture->texture_complete_fence				= VK_NULL_HANDLE;
		texture->transfer_semaphore					= VK_NULL_HANDLE;
		texture->blit_semaphore						= VK_NULL_HANDLE;
		texture->primary_render_command_buffer		= VK_NULL_HANDLE;
		texture->secondary_render_command_buffer	= VK_NULL_HANDLE;
		texture->primary_transfer_command_buffer	= VK_NULL_HANDLE;
		texture->staging_buffer						= {};
	}

private:
	vk2d::_internal::TextureResourceImpl		*	texture;
};

void vk2d::_internal::TextureResourceImpl::ScheduleTextureLoadResourceDestruction()
{
	resource_manager->GetThreadPool()->ScheduleTask(
		std::make_unique<DestroyTextureLoadResources>( this ),
		{ texture->GetLoaderThread() }
	);
}

} // _internal

} // vk2d
