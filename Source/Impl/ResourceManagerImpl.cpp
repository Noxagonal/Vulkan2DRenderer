
#include "../Header/Core/SourceCommon.h"

#include "../Header/Impl/ResourceManagerImpl.h"
#include "../Header/Core/ThreadPool.h"
#include "../../Include/Interface/TextureResource.h"
#include "../Header/Impl/TextureResourceImpl.h"



namespace vk2d {

namespace _internal {





// Load task works on the resource list through pointers
class LoadTask : public _internal::Task {
public:
	LoadTask(
		ResourceManagerImpl			*	resource_manager,
		Resource					*	resource
	) :
		resource_manager( resource_manager ),
		resource( resource )
	{};

	void operator()( _internal::ThreadPrivateResource * thread_resource )
	{
		if( !resource->MTLoad( thread_resource ) ) {
			resource->failed_to_load	= true;
		}
		resource->load_function_ran		= true;
	}

	ResourceManagerImpl				*	resource_manager		= {};
	Resource						*	resource				= {};
};

// Unload task takes ownership of the task
class UnloadTask : public _internal::Task {
public:
	UnloadTask(
		ResourceManagerImpl			*	resource_manager,
		std::unique_ptr<Resource>		resource
	) :
		resource_manager( resource_manager ),
		resource( std::move( resource ) )
	{};

	void operator()( _internal::ThreadPrivateResource * thread_resource )
	{
		resource->MTUnload( thread_resource );
	}

	ResourceManagerImpl				*	resource_manager		= {};
	std::unique_ptr<Resource>			resource				= {};
};





ResourceManagerImpl::ResourceManagerImpl( _internal::RendererImpl * parent_renderer )
{
	parent			= parent_renderer;
	assert( parent );

	device			= parent->GetVulkanDevice();
	assert( device );

	thread_pool		= parent->GetThreadPool();
	assert( thread_pool );

	loader_threads	= parent->GetLoaderThreads();

	is_good		= true;
}

ResourceManagerImpl::~ResourceManagerImpl()
{
	std::lock_guard<std::mutex> lock_guard( resources_mutex );

	auto it = resources.begin();
	while( it != resources.end() ) {
		( *it )->WaitUntilLoaded();
		thread_pool->ScheduleTask( std::make_unique<UnloadTask>( this, std::move( *it ) ), { ( *it )->GetLoaderThread() } );
		it = resources.erase( it );
	}

	thread_pool->WaitIdle();
}



TextureResource * ResourceManagerImpl::LoadTextureResource(
	const std::filesystem::path		&	file_path )
{
	std::lock_guard<std::mutex> lock_guard( resources_mutex );

	/*
	// RETURNING EXISTING RESOURCES IS NOT SUPPORTED ANYMORE.
	// User will keep track of resources, resource manager only makes
	// sure everything gets destroyed at the end of the application.

	// find resource if it exists
	for( auto & res : resources ) {
		auto texres = dynamic_cast<TextureResource*>( res.get() );
		if( res->IsFromFile() && res->GetFilePath() == file_path ) {
			return texres;
		}
	}
	*/

	auto resource		= std::unique_ptr<TextureResource>( new TextureResource( this ) );
	auto resource_ptr	= resource.get();
	if( !resource || !resource->IsGood() ) return nullptr; // Could not create resource.

	resource->path			= file_path;
	resource->is_from_file	= true;
	resource->loader_thread	= SelectLoaderThread();
	resources.push_back( std::move( resource ) );

	thread_pool->ScheduleTask( std::make_unique<LoadTask>( this, resource_ptr ), { resource_ptr->loader_thread } );
	return resource_ptr;
}

TextureResource * ResourceManagerImpl::CreateTextureResource(
	uint32_t							size_x,
	uint32_t							size_y,
	const std::vector<vk2d::Texel>	&	texture_data )
{
	auto resource		= std::unique_ptr<TextureResource>( new TextureResource( this ) );
	auto resource_ptr	= resource.get();
	if( !resource || !resource->IsGood() ) return nullptr; // Could not create resource.

	resource->impl->extent			= { size_x, size_y };
	resource->impl->texture_data	= texture_data;
	resource->is_from_file	= false;
	resource->loader_thread	= SelectLoaderThread();
	resources.push_back( std::move( resource ) );

	thread_pool->ScheduleTask( std::make_unique<LoadTask>( this, resource_ptr ), { resource_ptr->loader_thread } );
	return resource_ptr;
}

void ResourceManagerImpl::DestroyResource(
	Resource	*	resource
)
{
 	if( !resource ) return;

	// We'll have to wait until the resource is definitely loaded, or encountered an error.
	resource->WaitUntilLoaded();

	std::lock_guard<std::mutex> lock_guard( resources_mutex );

	// find resource if it exists
	auto it = resources.begin();
	while( it != resources.end() ) {
		if( it->get() == resource ) {
			// Found resource in the resources list
			thread_pool->ScheduleTask( std::make_unique<UnloadTask>( this, std::move( *it ) ), { resource->GetLoaderThread() } );
			it = resources.erase( it );
			return;
		} else {
			++it;
		}
	}
}

_internal::RendererImpl * ResourceManagerImpl::GetRenderer() const
{
	return parent;
}

_internal::ThreadPool * ResourceManagerImpl::GetThreadPool() const
{
	return thread_pool;
}

const std::vector<uint32_t> & ResourceManagerImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & ResourceManagerImpl::GetGeneralThreads() const
{
	return general_threads;
}

VkDevice ResourceManagerImpl::GetVulkanDevice() const
{
	return device;
}

bool ResourceManagerImpl::IsGood() const
{
	return is_good;
}

uint32_t ResourceManagerImpl::SelectLoaderThread()
{
	auto load_thread = loader_threads[ current_loader_thread_index++ ];
	if( current_loader_thread_index >= uint32_t( loader_threads.size() ) ) current_loader_thread_index = 0;

	return load_thread;
}

} // _internal

} // vk2d
