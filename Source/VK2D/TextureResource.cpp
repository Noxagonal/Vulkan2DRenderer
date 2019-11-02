
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/TextureResource.h"
#include "../Header/TextureResourceImpl.h"



namespace vk2d {



TextureResource::TextureResource(
	_internal::RendererImpl			*	renderer_parent
)
{
	impl		= std::make_unique<_internal::TextureResourceImpl>( renderer_parent );
	if( !impl )				return;
	if( !impl->IsGood() )	return;

	is_good		= true;
}

TextureResource::~TextureResource()
{}

bool TextureResource::IsGood()
{
	return is_good;
}



}
