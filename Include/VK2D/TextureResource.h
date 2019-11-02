#pragma once

#include "Common.h"
#include "Resource.h"

#include <memory>

namespace vk2d {

namespace _internal {
class RendererImpl;
class TextureResourceImpl;
}


class TextureResource :
	public Resource
{
	friend class _internal::RendererImpl;

	TextureResource(
		_internal::RendererImpl						*	renderer_parent );

public:
	~TextureResource();

	bool						IsGood();

private:
	std::unique_ptr<_internal::TextureResourceImpl>		impl	= {};

	bool						is_good							= {};
};



}
