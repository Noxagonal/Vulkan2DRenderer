
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Resource.h"
#include "../Header/Impl/ResourceManagerImpl.h"

#include <thread>
#include <assert.h>



VK2D_API vk2d::Resource::Resource(
	uint32_t										thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::Resource								*	parent_resource
)
{
	this->loader_thread				= thread;
	this->resource_manager_parent	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths				= {};
	this->is_from_file				= false;

	if( this->parent_resource ) {
		this->parent_resource->AddSubresource( this );
	}
}

VK2D_API vk2d::Resource::Resource(
	uint32_t										thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::Resource								*	parent_resource,
	std::filesystem::path							path
)
{
	this->loader_thread				= thread;
	this->resource_manager_parent	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths.resize( 1 );
	this->file_paths[ 0 ]			= path;
	this->is_from_file				= true;

	if( this->parent_resource ) {
		this->parent_resource->AddSubresource( this );
	}
}

VK2D_API vk2d::Resource::Resource(
	uint32_t										thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::Resource								*	parent_resource,
	const std::vector<std::filesystem::path>	&	paths
)
{
	this->loader_thread				= thread;
	this->resource_manager_parent	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths				= paths;
	this->is_from_file				= true;

	if( this->parent_resource ) {
		this->parent_resource->AddSubresource( this );
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Resource::DestroySubresources()
{
	std::lock_guard<std::mutex> lock_guard( resource_mutex );
	for( auto s : subresources ) {
		s->DestroySubresources();
		resource_manager_parent->DestroyResource( s );
	}
	subresources.clear();
}

VK2D_API void VK2D_APIENTRY vk2d::Resource::AddSubresource(
	vk2d::Resource			*	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( resource_mutex );
	subresources.push_back( subresource );
}

VK2D_API vk2d::Resource * VK2D_APIENTRY vk2d::Resource::GetParentResource()
{
	return parent_resource;
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::FailedToLoad() const
{
	return failed_to_load.load();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::Resource::GetLoaderThread()
{
	return loader_thread;
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsFromFile() const
{
	return is_from_file;
}

VK2D_API const std::vector<std::filesystem::path> & VK2D_APIENTRY vk2d::Resource::GetFilePaths() const
{
	return file_paths;
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsGood() const
{
	return is_good;
}

VK2D_API bool VK2D_APIENTRY vk2d::Resource::IsSubResource() const
{
	return !!parent_resource;
}
