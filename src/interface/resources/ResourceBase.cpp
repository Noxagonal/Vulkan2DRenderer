
#include "core/SourceCommon.h"

#include "interface/resources/ResourceBase.h"
#include "interface/resources/ResourceImplBase.h"

#include "interface/InstanceImpl.h"

#include "interface/resources/ResourceManagerImpl.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::ResourceStatus VK2D_APIENTRY vk2d::ResourceBase::GetStatus()
{
	assert( resource_impl );
	return resource_impl->GetStatus();
}

VK2D_API vk2d::ResourceStatus VK2D_APIENTRY vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceStatus VK2D_APIENTRY vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceBase * VK2D_APIENTRY vk2d::ResourceBase::GetParentResource()
{
	assert( resource_impl );
	return resource_impl->GetParentResource();
}

VK2D_API bool VK2D_APIENTRY vk2d::ResourceBase::IsFromFile() const
{
	assert( resource_impl );
	return resource_impl->IsFromFile();
}

VK2D_API const std::vector<std::filesystem::path>&VK2D_APIENTRY vk2d::ResourceBase::GetFilePaths() const
{
	assert( resource_impl );
	return resource_impl->GetFilePaths();
}

VK2D_API bool VK2D_APIENTRY vk2d::ResourceBase::IsGood() const
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







vk2d::_internal::ResourceImplBase::ResourceImplBase(
	vk2d::ResourceBase							*	my_interface,
	uint32_t										loader_thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::ResourceBase							*	parent_resource
)
{
	this->my_interface				= my_interface;
	this->loader_thread				= loader_thread;
	this->resource_manager	= resource_manager;
	this->parent_resource			= parent_resource;
	this->file_paths				= {};
	this->is_from_file				= false;

	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_interface );
	}
}

vk2d::_internal::ResourceImplBase::ResourceImplBase(
	vk2d::ResourceBase							*	my_interface,
	uint32_t										loader_thread,
	vk2d::_internal::ResourceManagerImpl		*	resource_manager,
	vk2d::ResourceBase							*	parent_resource,
	const std::vector<std::filesystem::path>	&	paths
)
{
	this->my_interface		= my_interface;
	this->loader_thread		= loader_thread;
	this->resource_manager	= resource_manager;
	this->parent_resource	= parent_resource;
	this->file_paths		= paths;
	this->is_from_file		= true;

	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_interface );
	}
}

void vk2d::_internal::ResourceImplBase::DestroySubresources()
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex );

	for( auto s : subresources ) {
		s->resource_impl->DestroySubresources();
		resource_manager->DestroyResource( s );
	}
	subresources.clear();
}

void vk2d::_internal::ResourceImplBase::AddSubresource(
	vk2d::ResourceBase		*	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex );

	subresources.push_back( subresource );
}

vk2d::ResourceBase * VK2D_APIENTRY vk2d::_internal::ResourceImplBase::GetParentResource()
{
	return parent_resource;
}

uint32_t vk2d::_internal::ResourceImplBase::GetLoaderThread()
{
	return loader_thread;
}

bool vk2d::_internal::ResourceImplBase::IsFromFile() const
{
	return is_from_file;
}

const std::vector<std::filesystem::path> & vk2d::_internal::ResourceImplBase::GetFilePaths() const
{
	return file_paths;
}

bool vk2d::_internal::ResourceImplBase::IsSubResource() const
{
	return !!parent_resource;
}
