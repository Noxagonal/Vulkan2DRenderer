
#include "../Header/SourceCommon.h"

#include "../Header/TextureResourceImpl.h"

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
	return false;
}

void TextureResourceImpl::MTUnload()
{
	// TODO
}

bool TextureResourceImpl::IsGood()
{
	return is_good;
}

} // _internal

} // vk2d
