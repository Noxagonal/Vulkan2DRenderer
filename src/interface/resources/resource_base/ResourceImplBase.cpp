
#include <core/SourceCommon.hpp>

#include <interface/resources/resource_base/ResourceBase.hpp>
#include "ResourceImplBase.hpp"

#include <interface/instance/InstanceImpl.hpp>

#include <interface/resource_manager/ResourceManagerImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::vk2d_internal::ResourceImplBase::GetReferenceCount() const
{
	return reference_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::ResourceImplBase::CanBeDestroyedNow() const
{
	auto lock_guard = std::unique_lock( destroy_resource_mutex, std::defer_lock );
	if( lock_guard.try_lock() == false ) return false;

	// TODO: GetStatus() should be implemeted in ResourceImplBase instead of per resource. This requires GetStatus() to never do
	// any actual work in the resource. This means all loading needs to be moved into the MTLoad() (Which is also todo).
	//if( GetStatus() == ResourceStatus::UNDETERMINED ) return false;

	if( reference_count.load() != 0 ) return false;
	if( reference_decrementer_thread_count.load() != 0 ) return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::ResourceImplBase::IncrementReferenceCount()
{
	++reference_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::ResourceImplBase::DecrementReferenceCount()
{
	// This keeps track of how many threads are currently trying to decrement the reference count for this resource.
	++reference_decrementer_thread_count;

	assert( reference_count.load() > 0 );

	--reference_count;
	if( reference_count.load() == 0 )
	{
		// If multiple threads release the same resource at once, it is possible some of them might enter this section.
		// This lock guard will make sure only one thread can call DestroyResource().
		auto lock_guard = std::unique_lock( destroy_resource_mutex );

		if( !is_marked_for_destruction )
		{
			is_marked_for_destruction = true;

			lock_guard.unlock();

			// Inform resource manager that this resource should be destroyed, this is safe as the destruction is queued, and
			// before the resource is actually destroyed, CanBeDestroyedNow() is called to make sure no accesses are still made
			// to this resource.
			resource_manager.DestroyResource( &this->my_base_interface );
		}
	}
	--reference_decrementer_thread_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::ResourceImplBase::DestroySubresources_DEPRECATED()
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex_DEPRECATED );

	for( auto s : subresources_DEPRECATED ) {
		s->resource_impl->DestroySubresources_DEPRECATED();
		resource_manager.DestroyResource( s );
	}
	subresources_DEPRECATED.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::ResourceImplBase::AddSubresource_DEPRECATED(
	ResourceBase		&	subresource
)
{
	std::lock_guard<std::mutex> lock_guard( subresources_mutex_DEPRECATED );

	subresources_DEPRECATED.push_back( &subresource );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceBase * vk2d::vk2d_internal::ResourceImplBase::GetParentResource_DEPRECATED()
{
	return parent_resource;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t vk2d::vk2d_internal::ResourceImplBase::GetLoaderThread()
{
	return loader_thread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::ResourceImplBase::IsFromFile() const
{
	return is_from_file;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<std::filesystem::path> & vk2d::vk2d_internal::ResourceImplBase::GetFilePaths() const
{
	return file_paths;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::ResourceImplBase::IsSubResource() const
{
	return !!parent_resource;
}
