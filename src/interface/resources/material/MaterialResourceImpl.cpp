
#include <core/SourceCommon.hpp>

#include <interface/resources/MaterialResource.hpp>
#include "MaterialResourceImpl.hpp"

#include <interface/resource_manager/ResourceManagerImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::MaterialResourceImpl::MaterialResourceImpl(
	MaterialResource			&	my_interface,
	ResourceManagerImpl			&	resource_manager,
	uint32_t						loader_thread,
	ResourceBase				*	parent_resource,
	const MaterialCreateInfo	&	create_info
) :
	ResourceImplBase(
		my_interface,
		loader_thread,
		resource_manager,
		parent_resource
	),
	my_interface( my_interface ),
	resource_manager( resource_manager ),
	create_info_copy( create_info )
{
	// TODO: Material resource should fully define how the drawing actually happens, what shaders and pipelines are made for it.
	// 
	// What is needed of material:
	// - Ability to be fully loaded on the background to prevent any kind of potential stutters in the main rendering thread.
	// 
	// - Custom vertices.
	// 
	// - Custon per-draw data. (Per draw instance data)
	// 
	// - Define pipeline fully in advance.
	//   - This means that the material will be responsible for determining if it can draw points, lines, polygons, filled or
	//     lined polygons.
	//   - See which pipeline states may be defined as dynamic, and even then, keep the amount of dynamic states low.
	// 
	// - Mesh will no longer determine if it will be drawn as points, lines or polygons.
	// 
	// - Material can determine the amount of textures and samplers.
	//   - One material with different textures and potentially different samplers may be used to keep the amount of materials
	//     down. Allowing only one material to define how the drawing actually happens and not necessarily what resources are
	//     tied to it.
	//

	is_good						= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::MaterialResourceImpl::~MaterialResourceImpl()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::MaterialResourceImpl::MTLoad(
	ThreadPrivateResource * thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadMaterialLoaderResource*>( thread_resource );
	assert( loader_thread_resource );
	if( !loader_thread_resource ) return false;

	auto memory_pool		= loader_thread_resource->GetDeviceMemoryPool();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::MaterialResourceImpl::MTUnload(
	ThreadPrivateResource * thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadMaterialLoaderResource*>( thread_resource );
	assert( loader_thread_resource );
	if( !loader_thread_resource ) return;

	auto memory_pool		= loader_thread_resource->GetDeviceMemoryPool();

	// TODO: Is this check necessary? resource should be loaded by this point, make sure and remove this call if possible.
	// Check if loaded successfully, no need to check for failure as this thread was
	// responsible for loading it, it's either loaded or failed to load but it'll
	// definitely be either or. MTUnload() does not ever get called before MTLoad().
	WaitUntilLoaded( std::chrono::nanoseconds::max() );

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::MaterialResourceImpl::GetStatus()
{
	if( !is_good ) return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED )
	{
		if( load_function_run_fence.IsSet() )
		{
			// TODO: Determine status if it is undetermined.

			//if( result == VK_SUCCESS )
			//{
			//	status = local_status = ResourceStatus::LOADED;
			//}
			//else if( result == VK_NOT_READY )
			//{
			//	return local_status;
			//}
			//else
			//{
			//	status = local_status = ResourceStatus::FAILED_TO_LOAD;
			//}
		}
	}

	return local_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::MaterialResourceImpl::WaitUntilLoaded(
	std::chrono::nanoseconds timeout
)
{
	if( timeout == std::chrono::nanoseconds::max() )
	{
		return WaitUntilLoaded( std::chrono::steady_clock::time_point::max() );
	}
	return WaitUntilLoaded( std::chrono::steady_clock::now() + timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::MaterialResourceImpl::WaitUntilLoaded(
	std::chrono::steady_clock::time_point timeout
)
{
	// Make sure timeout is in the future.
	assert(
		timeout == std::chrono::steady_clock::time_point::max() ||
		timeout + std::chrono::seconds( 5 ) >= std::chrono::steady_clock::now()
	);

	if( !is_good ) return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED )
	{
		if( load_function_run_fence.Wait( timeout ) )
		{
			// We can check the status of the fence in any thread,
			// it will not be removed until the resource is removed.

			// TODO: Determine status if it is undetermined.

			//auto timeout_for_fences = ( timeout == std::chrono::steady_clock::time_point::max() ) ?
			//	UINT64_MAX :
			//	uint64_t( std::chrono::duration_cast<std::chrono::nanoseconds>( timeout - std::chrono::steady_clock::now() ).count() );
			//
			//if( result == VK_SUCCESS ) {
			//	status = local_status = ResourceStatus::LOADED;
			//	SchedulePostLoadCleanup();
			//}
			//else if( result == VK_TIMEOUT ) {
			//	return local_status;
			//}
			//else {
			//	status = local_status = ResourceStatus::FAILED_TO_LOAD;
			//	SchedulePostLoadCleanup();
			//}
		}
	}

	return local_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::MaterialResourceImpl::IsGood() const
{
	return is_good;
}



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handles the texture destruction
class DestroyTextureLoadResources :
	public Task
{
public:
	DestroyTextureLoadResources(
		MaterialResourceImpl	*	material
	) :
		material( material )
	{};

	void operator()(
		ThreadPrivateResource	*	thread_resource
	)
	{}

private:
	MaterialResourceImpl		*	material;
};



} // vk2d_internal
} // vk2d



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::MaterialResourceImpl::SchedulePostLoadCleanup()
{
	resource_manager.GetThreadPool().ScheduleTask(
		std::make_unique<DestroyTextureLoadResources>( this ),
		{ GetLoaderThread() }
	);
}
