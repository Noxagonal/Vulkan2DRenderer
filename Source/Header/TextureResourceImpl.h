#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/TextureResource.h"
#include "ResourceImpl.h"


namespace vk2d {

namespace _internal {



class TextureResourceImpl :
	public _internal::ResourceImpl
{

	friend class TextureResource;

public:
	TextureResourceImpl(
		_internal::RendererImpl						*	renderer_parent );

	~TextureResourceImpl();

	bool								MTLoad();
	bool								MTUnload();

	bool								IsGood();

private:
	_internal::RendererImpl			*	parent						= {};

	bool								is_good						= {};
};



} // _internal

} // vk2d
