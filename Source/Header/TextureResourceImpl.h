#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/TextureResource.h"


namespace vk2d {

namespace _internal {

class ResourceManagerImpl;



class TextureResourceImpl
{
	friend class TextureResource;
	friend class _internal::ResourceManagerImpl;

public:
	TextureResourceImpl(
		TextureResource							*	texture_resource_parent,
		_internal::ResourceManagerImpl			*	resource_manager );

	~TextureResourceImpl();

	bool											MTLoad();
	bool											MTUnload();

	bool											IsGood();

private:
	TextureResource								*	parent						= {};
	_internal::ResourceManagerImpl				*	resource_manager			= {};

	bool											is_good						= {};
};



} // _internal

} // vk2d
