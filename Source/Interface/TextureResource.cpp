
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/TextureResource.h"
#include "../Header/Impl/TextureResourceImpl.h"



namespace vk2d {



VK2D_API TextureResource::TextureResource(
	_internal::ResourceManagerImpl		*	resource_manager_parent
)
{
	impl		= std::make_unique<_internal::TextureResourceImpl>( this, resource_manager_parent );
	if( !impl )	return;
	if( !impl->IsGood() ) {
		impl	= nullptr;
		return;
	}
	is_good		= true;
}

VK2D_API TextureResource::~TextureResource()
{}

VK2D_API bool VK2D_APIENTRY TextureResource::IsLoaded()
{
	if( impl ) return impl->IsLoaded();
	return false;
}

VK2D_API bool VK2D_APIENTRY TextureResource::WaitUntilLoaded()
{
	if( impl ) return impl->WaitUntilLoaded();
	return false;
}

VK2D_API bool VK2D_APIENTRY TextureResource::IsGood()
{
	return is_good;
}

VK2D_API bool VK2D_APIENTRY TextureResource::MTLoad(
	_internal::ThreadPrivateResource	*	thread_resource
)
{
	if( impl ) return impl->MTLoad( thread_resource );
	return false;
}

VK2D_API void VK2D_APIENTRY TextureResource::MTUnload(
	_internal::ThreadPrivateResource	*	thread_resource
)
{
	if( impl ) impl->MTUnload( thread_resource );
}



}
