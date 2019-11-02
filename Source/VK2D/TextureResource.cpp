
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/TextureResource.h"
#include "../Header/TextureResourceImpl.h"



namespace vk2d {



VK2D_API TextureResource::TextureResource(
	_internal::ResourceManagerImpl		*	resource_manager_parent
)
{
	impl		= std::make_unique<_internal::TextureResourceImpl>( this, resource_manager_parent );
	if( !impl )				return;
	if( !impl->IsGood() )	return;

	is_good		= true;
}

VK2D_API TextureResource::~TextureResource()
{}

VK2D_API bool VK2D_APIENTRY TextureResource::IsGood()
{
	return is_good;
}

VK2D_API bool VK2D_APIENTRY TextureResource::MTLoad()
{
	if( impl && impl->IsGood() ) return impl->MTLoad();
	return false;
}

VK2D_API bool VK2D_APIENTRY TextureResource::MTUnload()
{
	if( impl && impl->IsGood() ) return impl->MTUnload();
	return false;
}



}
