
#include <core/SourceCommon.hpp>

#include <interface/resources/resource_base/ResourceBase.hpp>
#include "ResourceImplBase.hpp"

#include <interface/instance/InstanceImpl.hpp>

#include <interface/resource_manager/ResourceManagerImpl.hpp>



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
		this->parent_resource->resource_impl->AddSubresource_DEPRECATED( my_base_interface );
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
		this->parent_resource->resource_impl->AddSubresource_DEPRECATED( my_base_interface );
	}
}

void vk2d::vk2d_internal::ResourceImplBase::DestroySubresources_DEPRECATED()
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex_DEPRECATED );

	for( auto s : subresources_DEPRECATED ) {
		s->resource_impl->DestroySubresources_DEPRECATED();
		resource_manager.DestroyResource( s );
	}
	subresources_DEPRECATED.clear();
}

void vk2d::vk2d_internal::ResourceImplBase::AddSubresource_DEPRECATED(
	ResourceBase		&	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex_DEPRECATED );

	subresources_DEPRECATED.push_back( &subresource );
}

vk2d::ResourceBase * vk2d::vk2d_internal::ResourceImplBase::GetParentResource_DEPRECATED()
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
