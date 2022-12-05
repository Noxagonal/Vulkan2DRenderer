
#include <core/SourceCommon.hpp>

#include <interface/resources/material/MaterialResource.hpp>
#include "MaterialResourceImpl.hpp"

#include <interface/resource_manager/ResourceManagerImpl.hpp>
#include <interface/instance/InstanceImpl.hpp>

#include <vulkan/Device.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::MaterialResourceImpl::MaterialResourceImpl(
	MaterialResource					&	my_interface,
	ResourceManagerImpl					&	resource_manager,
	uint32_t								loader_thread,
	ResourceBase						*	parent_resource,
	std::span<ShaderMemberInfo>				vertex_members,
	const MaterialCreateInfo			&	create_info
) :
	ResourceImplBase(
		my_interface,
		loader_thread,
		resource_manager,
		parent_resource
	),
	my_interface( my_interface ),
	resource_manager( resource_manager ),
	vertex_members( vertex_members.begin(), vertex_members.end() ),
	create_info_copy( create_info )
{
	// TODO: Material resource should fully define how the drawing actually happens, what shaders and pipelines are made for it.
	//
	// What is needed of material:
	// - Ability to be fully loaded on the background to prevent any kind of potential stutters in the main rendering thread.
	//
	// - Shaders and vulkan pipelines may be reused. This means that they must be reference counted. Add reference counting on
	//   these managers instead of plain create and destroy functionalities.
	//
	// - Custom vertices. Need to convey types and names to the shader somehow, otherwise this is done.
	// 
	// - Custon per-draw data. (Per draw instance data) need a custom data struct and reflection similar to vertex for this.
	//
	// - Mesh will no longer determine if it will be drawn as points, lines or polygons.
	//
	// - Is material compatiblity with mesh checked in runtime or compile time? Can it be checked in compile time?
	//   Most of the type info is erased when the material is actually created, because it travels accross shared library
	//   boundaries. I think it will have to be runtime checked always. However I can definitely implement some compile time
	//   checking whenever possible.
	//
	// - How to verify a vertex and its member types are compatible with the material, or other structs sent to the GPU?
	//   - Resource manager could return a resource handle that automatically manages the resource lifetime. This resource
	//     handle could be a template class.
	//   - Look into std::is_same, Vertex::Base::IsVertex... For compile time checking whenever possible.
	//   - Some runtime checking is necessary, might need to check all vertex members match expected for each and every call,
	//     which is going to be a pretty heavy task. Maybe the vertex member types could be hashed and just the hash checked?
	//   - This is going to be a user development issue rather than an issue resulted by having a different system. This could
	//     probably be a check made only on debug builds, unless the final app decides to somehow mix different types of
	//     meshes with different types of materials on the fly. (This remainds me, look into throwing errors instead of
	//     returning error values, also look into release builds with debug features.)
	//
	// - Define pipeline fully in advance.
	//   - This means that the material will be responsible for determining if it can draw points, lines, polygons, filled or
	//     lined polygons.
	//   - See which pipeline states may be defined as dynamic, and even then, keep the amount of dynamic states low.
	//     - Vulkan 1.3 pretty much opened the entire pipeline to be dynamic if needed. This may have a lot of performance
	//       costs.
	//
	// - Material can determine the amount of textures and samplers.
	//   - One material with different textures and potentially different samplers may be used to keep the amount of materials
	//     down. Allowing only one material to define how the drawing actually happens and not necessarily what resources are
	//     tied to it.
	//
	// - Sampler info is pretty much the detail for the material instead of a separate object. (Currently regular sampler is
	//   depricated.)
	//   - It would further help keep the number of materials down by allowing samplers to be their own entities and reused.
	//     This shouldn't be a performance consideration either, samplers may be switched between draw calls.
	//   - It would be easier for the user to define the sampler inside the material.
	//   - Consider an array of mostly similar samplers defined for a material.
	//   - How often are different samplers actually defined for the material? Would it be easier to just use a specific one,
	//     or create a similar one for each material.
	//
	// - GLSL to Spir-V compilation must happen on a single thread (glslang limitation). Consider separating shader and
	//   pipeline building to 2 different resources, Eg. Create Shader class which holds a compiled shader.
	//   - Pros:
	//     - This would allow building shaders in advance and stored for later use, even saved on disk.
	//     - Enables creating pipelines on multiple threads, or at the very least, allows staggering compiling shaders and
	//       creating pipelines on 2 different threads.
	//   - Cons:
	//     - Fragments the implementation of a material into 2 parts that really should be one operation.
	//     - Runtime check is needed to validate a material compatibility with a shader, cannot check during compile time.
	//   - Possible fixes:
	//     - It may not be worth separating shader compilation from material creation as a whole thread is reserved just to
	//       material creation so it is always ready when needed.
	//     - Using Vulkan pipeline cache requires synchronization, so only one pipeline can be created at a time anyhow,
	//       unless each thread uses a separate pipeline cache, then it would be possible to create multiple pipelines at a
	//       time.
	//     - Keep CreateMaterialResource() working on a single thread for simplicity, but allow LoadMaterialResource() to use
	//       any thread. This would allow storing already compiled Spir-V into a file. (Material saving is still a little out
	//       in the open on how to implement it. It may reaquire a whole interface just for resource saving.)
	//       This would also allow saving the hash of the Spir-V, which would make it faster to search for already loaded
	//       shaders and simply increment their reference count.
	//       (I personally prefer this option, it's somewhat demanding, but easy, to create a material but performance can be
	//       mitigated by loading other already created materials from file faster.)
	//

	is_good						= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::MaterialResourceImpl::~MaterialResourceImpl()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::ResourceMTLoadResult vk2d::vk2d_internal::MaterialResourceImpl::MTLoad(
	LocalThreadData * thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadMaterialLoaderResource*>( thread_resource );
	assert( loader_thread_resource );
	if( !loader_thread_resource ) return ResourceMTLoadResult::FAILED;

	auto & instance = loader_thread_resource->GetInstance();
	auto memory_pool = loader_thread_resource->GetThreadLocalDeviceMemoryPool();

	auto AssignShaders = [ this, &instance ]() -> bool
	{
		auto & shader_manager = loader_thread_resource->GetVulkanDevice().GetShaderManager();
		for( auto & shader_create_info : create_info_copy.shader_create_infos )
		{
			// If more shader types are added, eg. Geometry shaders and tessellation shaders... They need to be added here.
			switch( shader_create_info.GetStage() )
			{
			case ShaderStage::VERTEX:
			{
				if( vertex_shader )
				{
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						"Introduced multiple vertex shaders to material, must be exactly 1"
					);
				}
				vertex_shader = shader_manager.GetShader( shader_create_info );

				break;
			}

			case ShaderStage::FRAGMENT:
			{
				if( fragment_shader )
				{
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						"Introduced multiple vertex shaders to material, must be exactly 1"
					);
				}
				fragment_shader = shader_manager.GetShader( shader_create_info );

				break;
			}

			default:
			{
				// TODO: Throw here.
				instance.Report(
					ReportSeverity::NON_CRITICAL_ERROR,
					"Inroduced unsupported or unknown shader type to material, currently supported shader types are VERTEX and FRAGMENT."
				);
				return false;
			}
			}
		}

		// TODO: Assign default shaders to all shaders that have not been assigned above.

		return true;
	};

	if( !AssignShaders() ) return ResourceMTLoadResult::FAILED;

	return ResourceMTLoadResult::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::ResourceMTUnloadResult vk2d::vk2d_internal::MaterialResourceImpl::MTUnload(
	LocalThreadData * thread_resource
)
{
	loader_thread_resource	= dynamic_cast<ThreadMaterialLoaderResource*>( thread_resource );
	assert( loader_thread_resource );
	if( !loader_thread_resource ) return ResourceMTUnloadResult::SUCCESS;

	auto memory_pool = loader_thread_resource->GetThreadLocalDeviceMemoryPool();

	// TODO: Remove once CanBeDestroyedNow() works fully.
	if( GetStatus() == ResourceStatus::UNDETERMINED ) return ResourceMTUnloadResult::POSTPONED;

	auto DestroyShaders = [ this ]()
	{
		vertex_shader = {};
		fragment_shader = {};
	};

	DestroyShaders();

	return ResourceMTUnloadResult::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::MaterialResourceImpl::GetStatus()
{
	if( !is_good ) return ResourceStatus::FAILED;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED )
	{
		// !!! DEBUGGING !!!
		status = local_status = ResourceStatus::AVAILABLE;

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

	if( !is_good ) return ResourceStatus::FAILED;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED )
	{
		// !!! DEBUGGING !!!
		status = local_status = ResourceStatus::AVAILABLE;

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

	TaskInvokeResult				operator()(
		LocalThreadData			*	thread_resource
	)
	{
		return TaskInvokeResult::SUCCESS;
	}

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
