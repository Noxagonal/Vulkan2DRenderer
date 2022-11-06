
#include <core/SourceCommon.h>

#include <interface/resources/ResourceBase.h>
#include <interface/resources/ResourceImplBase.h>

#include <interface/InstanceImpl.h>

#include <interface/resources/ResourceManagerImpl.h>







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::GetStatus()
{
	assert( resource_impl );
	return resource_impl->GetStatus();
}

VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceBase * vk2d::ResourceBase::GetParentResource()
{
	assert( resource_impl );
	return resource_impl->GetParentResource();
}

VK2D_API bool vk2d::ResourceBase::IsFromFile() const
{
	assert( resource_impl );
	return resource_impl->IsFromFile();
}

VK2D_API const std::vector<std::filesystem::path> & vk2d::ResourceBase::GetFilePaths() const
{
	assert( resource_impl );
	return resource_impl->GetFilePaths();
}

VK2D_API bool vk2d::ResourceBase::IsGood() const
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







vk2d::vk2d_internal::ResourceImplBase::ResourceImplBase(
	ResourceBase			&	my_base_interface,
	uint32_t					loader_thread,
	ResourceManagerImpl		&	resource_manager,
	ResourceBase			*	parent_resource
) :
	my_base_interface( my_base_interface ),
	loader_thread( loader_thread ),
	resource_manager( resource_manager ),
	parent_resource( parent_resource ),
	file_paths( {} ),
	is_from_file( false )
{
	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_base_interface );
	}
}

vk2d::vk2d_internal::ResourceImplBase::ResourceImplBase(
	ResourceBase								&	my_base_interface,
	uint32_t										loader_thread,
	ResourceManagerImpl							&	resource_manager,
	ResourceBase								*	parent_resource,
	const std::vector<std::filesystem::path>	&	paths
) :
	my_base_interface( my_base_interface ),
	loader_thread( loader_thread ),
	resource_manager( resource_manager ),
	parent_resource( parent_resource ),
	file_paths( paths ),
	is_from_file( true )
{
	if( this->parent_resource ) {
		this->parent_resource->resource_impl->AddSubresource( my_base_interface );
	}
}

void vk2d::vk2d_internal::ResourceImplBase::DestroySubresources()
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex );

	for( auto s : subresources ) {
		s->resource_impl->DestroySubresources();
		resource_manager.DestroyResource( s );
	}
	subresources.clear();
}

void vk2d::vk2d_internal::ResourceImplBase::AddSubresource(
	ResourceBase		&	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex );

	subresources.push_back( &subresource );
}

vk2d::ResourceBase * vk2d::vk2d_internal::ResourceImplBase::GetParentResource()
{
	return parent_resource;
}

uint32_t vk2d::vk2d_internal::ResourceImplBase::GetLoaderThread()
{
	return loader_thread;
}

bool vk2d::vk2d_internal::ResourceImplBase::IsFromFile() const
{
	return is_from_file;
}

const std::vector<std::filesystem::path> & vk2d::vk2d_internal::ResourceImplBase::GetFilePaths() const
{
	return file_paths;
}

bool vk2d::vk2d_internal::ResourceImplBase::IsSubResource() const
{
	return !!parent_resource;
}
