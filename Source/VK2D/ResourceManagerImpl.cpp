
#include "../Header/SourceCommon.h"

#include "../Header/ResourceManagerImpl.h"
#include "../Header/ThreadPool.h"
#include "../../Include/VK2D/TextureResource.h"



namespace vk2d {

namespace _internal {





ResourceManagerImpl::ResourceManagerImpl( _internal::RendererImpl * parent_renderer )
{
	parent			= parent_renderer;
	assert( parent );

	thread_pool		= parent->GetThreadPool();
	assert( thread_pool );

	loader_threads	= parent->GetLoaderThreads();

	is_good		= true;
}

ResourceManagerImpl::~ResourceManagerImpl()
{

}



class LoadTask : public _internal::Task {
public:
	LoadTask(
		ResourceManagerImpl		*	resource_manager,
		Resource				*	resource
	) :
		resource_manager( resource_manager ),
		resource( resource )
	{};

	void operator()( _internal::ThreadPrivateResource * thread_resource )
	{
		if( !resource->MTLoad() ) resource->failed_to_load = true;
	}

	ResourceManagerImpl		*	resource_manager		= {};
	Resource				*	resource				= {};
};

class UnloadTask : public _internal::Task {
public:
	UnloadTask(
		ResourceManagerImpl		*	resource_manager,
		Resource				*	resource
	) :
		resource_manager( resource_manager ),
		resource( resource )
	{};

	void operator()( _internal::ThreadPrivateResource * thread_resource )
	{
		resource->MTUnload();
	}

	ResourceManagerImpl		*	resource_manager		= {};
	Resource				*	resource				= {};
};



TextureResource * ResourceManagerImpl::LoadTextureResource(
	std::filesystem::path				file_path )
{
	std::lock_guard<std::mutex> lock_guard( resources_mutex );
	// find resource if it exists
	for( auto & res : resources ) {
		auto texres = dynamic_cast<TextureResource*>( res.get() );
		if( res->IsFromFile() && res->GetFilePath() == file_path ) {
			return texres;
		}
	}

	// Not found in resources, create it
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
	const std::vector<uint8_t>		&	texture_data )
{
	return nullptr;
}

bool ResourceManagerImpl::IsGood()
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
