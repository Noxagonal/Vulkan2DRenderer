
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/TextureResource.h"
#include "../Header/Impl/TextureResourceImpl.h"
#include "../Header/Impl/ResourceManagerImpl.h"



VK2D_API vk2d::TextureResource::TextureResource(
	vk2d::_internal::ResourceManagerImpl		*	resource_manager_parent
)
{
	impl		= std::make_unique<vk2d::_internal::TextureResourceImpl>( this, resource_manager_parent );
	if( !impl )	return;
	if( !impl->IsGood() ) {
		impl	= nullptr;
		resource_manager_parent->GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}
	is_good		= true;
}

VK2D_API vk2d::TextureResource::~TextureResource()
{}

VK2D_API bool VK2D_APIENTRY vk2d::TextureResource::IsLoaded()
{
	return impl->IsLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::TextureResource::WaitUntilLoaded()
{
	return impl->WaitUntilLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::TextureResource::IsGood()
{
	return is_good;
}

VK2D_API bool VK2D_APIENTRY vk2d::TextureResource::MTLoad(
	vk2d::_internal::ThreadPrivateResource	*	thread_resource
)
{
	return impl->MTLoad( thread_resource );
}

VK2D_API void VK2D_APIENTRY vk2d::TextureResource::MTUnload(
	vk2d::_internal::ThreadPrivateResource	*	thread_resource
)
{
	impl->MTUnload( thread_resource );
}
