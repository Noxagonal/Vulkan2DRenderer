
#include "../Header/SourceCommon.h"

#include "../Header/TextureResourceImpl.h"

#include <assert.h>


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
	// TODO
	return false;
}

bool TextureResourceImpl::MTUnload()
{
	// TODO
	return false;
}

bool TextureResourceImpl::IsGood()
{
	return is_good;
}

} // _internal

} // vk2d
