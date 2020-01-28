
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/FontResource.h"
#include "../Header/Impl/FontResourceImpl.h"

#include "../Header/Impl/ResourceManagerImpl.h"

VK2D_API vk2d::FontResource::FontResource(
	vk2d::Resource							*	parent_resource,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager,
	uint32_t									loader_thread,
	std::filesystem::path						file_path
) : vk2d::Resource(
	loader_thread,
	resource_manager,
	parent_resource,
	file_path
)
{
	impl		= std::make_unique<vk2d::_internal::FontResourceImpl>(
		this,
		resource_manager
	);
	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		is_good		= false;
		impl		= nullptr;
		resource_manager->GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font resource implementation!" );
		return;
	}
}

VK2D_API vk2d::FontResource::~FontResource()
{}

VK2D_API bool VK2D_APIENTRY vk2d::FontResource::IsLoaded()
{
	return impl->IsLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::FontResource::WaitUntilLoaded()
{
	return impl->WaitUntilLoaded();
}

VK2D_API vk2d::TextureResource *VK2D_APIENTRY vk2d::FontResource::GetTextureResource()
{
	return impl->GetTextureResource();
}

VK2D_API bool VK2D_APIENTRY vk2d::FontResource::MTLoad( vk2d::_internal::ThreadPrivateResource * thread_resource )
{
	return impl->MTLoad(
		thread_resource
	);
}

VK2D_API void VK2D_APIENTRY vk2d::FontResource::MTUnload( vk2d::_internal::ThreadPrivateResource * thread_resource )
{
	impl->MTUnload(
		thread_resource
	);
}
