
#include "../Header/SourceCommon.h"

#include "../Header/TextureResourceImpl.h"
#include "../Header/ResourceManagerImpl.h"

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
	this->parent				= texture_resource_parent;
	this->resource_manager		= resource_manager;
	assert( this->parent );
	assert( this->resource_manager );

	is_good						= true;
}

TextureResourceImpl::~TextureResourceImpl()
{
	// TODO
}

bool TextureResourceImpl::MTLoad()
{
	// 1. Load and process image from file.
	// 2. Create staging buffer, we'll also need memory pool for this.
	// 3. Create image and image view Vulkan objects.
	// 4. Allocate a command buffer from thread resources
	// 5. Record commands to upload image into the GPU.
	// 6. Record commands to make mipmaps of the image in the GPU.
	// 7. Make image available in a shader.
	// 8. Allocate descriptor set that points to the image.
	// 9. Submit command buffer to the GPU, get a fence handle to indicate when the image is ready to be used.

	auto memory_pool = resource_manager->GetRenderer()->GetDeviceMemoryPool();

	// Get data into a staging buffer, and create staging buffer
	struct {
		uint32_t	x;
		uint32_t	y;
		uint32_t	channels;
	} image_info;

	{
		// 1. Load and process image from file.

		if( parent->IsFromFile() ) {
			// Create texture from a file

			auto mypath = std::filesystem::current_path() / parent->GetFilePath();

			int image_size_x			= 0;
			int image_size_y			= 0;
			int image_channel_count		= 0;

			auto image_data = stbi_load(
				parent->GetFilePath().string().c_str(),
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
		image_create_info.usage						= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
	// 5. Record commands to upload image into the GPU.
	// 6. Record commands to make mipmaps of the image in the GPU.
	// 7. Make image available in a shader.
	// 8. Allocate descriptor set that points to the image.
	// 9. Submit command buffer to the GPU, get a fence handle to indicate when the image is ready to be used.

	return true;
}

void TextureResourceImpl::MTUnload()
{
	auto memory_pool = resource_manager->GetRenderer()->GetDeviceMemoryPool();

	memory_pool->FreeCompleteResource( image );
	memory_pool->FreeCompleteResource( staging_buffer );
}

bool TextureResourceImpl::IsGood()
{
	return is_good;
}

} // _internal

} // vk2d
