
#include "../../Core/SourceCommon.h"

#include "../../../Include/Interface/ResourceManager/Resource.h"
#include "ResourceImpl.h"

#include "ResourceManagerImpl.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsLoaded()
{
	assert( resource_impl );
	return resource_impl->IsLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::WaitUntilLoaded()
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded();
}

VK2D_API vk2d::Resource *VK2D_APIENTRY vk2d::Resource::GetParentResource()
{
	assert( resource_impl );
	return resource_impl->GetParentResource();
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::FailedToLoad() const
{
	assert( resource_impl );
	return resource_impl->FailedToLoad();
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsFromFile() const
{
	assert( resource_impl );
	return resource_impl->IsFromFile();
}

VK2D_API const std::vector<std::filesystem::path>&VK2D_APIENTRY vk2d::Resource::GetFilePaths() const
{
	assert( resource_impl );
	return resource_impl->GetFilePaths();
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsGood() const
{
	if( !resource_impl ) return false;
	return resource_impl->IsGood();
}







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







vk2d::_internal::ResourceImpl::ResourceImpl(
	vk2d::Resource								*	my_interface,
	uint32_t										loader_thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::Resource								*	parent_resource
)
{
	this->my_interface				= my_interface;
	this->loader_thread				= loader_thread;
	this->resource_manager_parent	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths				= {};
	this->is_from_file				= false;

	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_interface );
	}
}

vk2d::_internal::ResourceImpl::ResourceImpl(
	vk2d::Resource								*	my_interface,
	uint32_t										loader_thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::Resource								*	parent_resource,
	const std::vector<std::filesystem::path>	&	paths
)
{
	this->my_interface				= my_interface;
	this->loader_thread				= loader_thread;
	this->resource_manager_parent	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths				= paths;
	this->is_from_file				= true;

	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_interface );
	}
}

void vk2d::_internal::ResourceImpl::DestroySubresources()
{
	std::lock_guard<std::mutex> lock_guard( resource_mutex );
	for( auto s : subresources ) {
		s->resource_impl->DestroySubresources();
		resource_manager_parent->DestroyResource( s );
	}
	subresources.clear();
}

void vk2d::_internal::ResourceImpl::AddSubresource(
	vk2d::Resource			*	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( resource_mutex );
	subresources.push_back( subresource );
}

vk2d::Resource * VK2D_APIENTRY vk2d::_internal::ResourceImpl::GetParentResource()
{
	return parent_resource;
}

bool vk2d::_internal::ResourceImpl::FailedToLoad() const
{
	return failed_to_load.load();
}

uint32_t vk2d::_internal::ResourceImpl::GetLoaderThread()
{
	return loader_thread;
}

bool vk2d::_internal::ResourceImpl::IsFromFile() const
{
	return is_from_file;
}

const std::vector<std::filesystem::path> & vk2d::_internal::ResourceImpl::GetFilePaths() const
{
	return file_paths;
}

bool vk2d::_internal::ResourceImpl::IsSubResource() const
{
	return !!parent_resource;
}
