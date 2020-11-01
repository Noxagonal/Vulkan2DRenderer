
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/TextureResource.h"
#include "../Header/Impl/TextureResourceImpl.h"
#include "../Header/Impl/ResourceManagerImpl.h"



VK2D_API vk2d::TextureResource::TextureResource(
	vk2d::Resource							*	parent_resource,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager_parent,
	uint32_t									loader_thread,
	std::filesystem::path						file_path
) :
vk2d::Resource(
	loader_thread,
	resource_manager_parent,
	parent_resource,
	file_path
)
{
	impl		= std::make_unique<vk2d::_internal::TextureResourceImpl>(
		this,
		resource_manager_parent
	);
	if( impl && impl->IsGood() ) {
		texture_impl	= &*impl;
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		resource_manager_parent->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
	}
}

VK2D_API vk2d::TextureResource::TextureResource(
	vk2d::Resource							*	parent_resource,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager_parent,
	uint32_t									loader_thread,
	std::vector<std::filesystem::path>			file_paths_listing
) :
vk2d::Resource(
	loader_thread,
	resource_manager_parent,
	parent_resource,
	file_paths_listing
)
{
	impl		= std::make_unique<vk2d::_internal::TextureResourceImpl>(
		this,
		resource_manager_parent
		);
	if( impl && impl->IsGood() ) {
		texture_impl	= &*impl;
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		resource_manager_parent->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}
}

VK2D_API vk2d::TextureResource::TextureResource(
	vk2d::Resource							*	parent_resource,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager_parent,
	uint32_t									loader_thread,
	vk2d::Vector2u								size,
	const std::vector<vk2d::Color8>			&	texels
) :
vk2d::Resource(
	loader_thread,
	resource_manager_parent,
	parent_resource
)
{
	impl		= std::make_unique<vk2d::_internal::TextureResourceImpl>(
		this,
		resource_manager_parent,
		size,
		texels
	);
	if( impl && impl->IsGood() ) {
		texture_impl	= &*impl;
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		resource_manager_parent->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}
}

VK2D_API vk2d::TextureResource::TextureResource(
	vk2d::Resource									*	parent_resource,
	vk2d::_internal::ResourceManagerImpl			*	resource_manager_parent,
	uint32_t											loader_thread,
	vk2d::Vector2u										size,
	const std::vector<std::vector<vk2d::Color8>*>	&	texels_listing
) :
vk2d::Resource(
	loader_thread,
	resource_manager_parent,
	parent_resource
)
{
	impl		= std::make_unique<vk2d::_internal::TextureResourceImpl>(
		this,
		resource_manager_parent,
		size,
		texels_listing
		);
	if( impl && impl->IsGood() ) {
		texture_impl	= &*impl;
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		resource_manager_parent->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}
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

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::TextureResource::GetSize() const
{
	return impl->GetSize();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::TextureResource::GetLayerCount() const
{
	return impl->GetLayerCount();
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

VK2D_API bool VK2D_APIENTRY vk2d::TextureResource::IsGood() const
{
	return is_good;
}
