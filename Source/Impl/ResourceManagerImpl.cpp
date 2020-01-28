
#include "../Header/Core/SourceCommon.h"

#include "../Header/Impl/ResourceManagerImpl.h"
#include "../Header/Core/ThreadPool.h"
#include "../../Include/Interface/TextureResource.h"
#include "../../Include/Interface/FontResource.h"



namespace vk2d {
namespace _internal {

// Load task works on the resource list through pointers
class LoadTask : public vk2d::_internal::Task {
public:
	LoadTask(
		vk2d::_internal::ResourceManagerImpl	*	resource_manager,
		vk2d::Resource							*	resource
	) :
		resource_manager( resource_manager ),
		resource( resource )
	{};

	void operator()( vk2d::_internal::ThreadPrivateResource * thread_resource )
	{
		if( !resource->MTLoad( thread_resource ) ) {
			resource->failed_to_load	= true;
			resource_manager->GetRenderer()->Report( vk2d::ReportSeverity::WARNING, "Resource loading failed!" );
		}
		resource->load_function_ran		= true;
	}

	vk2d::_internal::ResourceManagerImpl	*	resource_manager		= {};
	vk2d::Resource							*	resource				= {};
};

// Unload task takes ownership of the task
class UnloadTask : public vk2d::_internal::Task {
public:
	UnloadTask(
		vk2d::_internal::ResourceManagerImpl	*	resource_manager,
		std::unique_ptr<vk2d::Resource>				resource
	) :
		resource_manager( resource_manager ),
		resource( std::move( resource ) )
	{};

	void operator()( vk2d::_internal::ThreadPrivateResource * thread_resource )
	{
		resource->MTUnload( thread_resource );
	}

	vk2d::_internal::ResourceManagerImpl	*	resource_manager		= {};
	std::unique_ptr<vk2d::Resource>				resource				= {};
};

} // _internal
} // vk2d



vk2d::_internal::ResourceManagerImpl::ResourceManagerImpl(
	vk2d::_internal::RendererImpl	*	parent_renderer
)
{
	renderer_parent			= parent_renderer;
	assert( renderer_parent );

	device			= renderer_parent->GetVulkanDevice();
	assert( device );

	thread_pool		= renderer_parent->GetThreadPool();
	assert( thread_pool );

	loader_threads	= renderer_parent->GetLoaderThreads();

	is_good		= true;
}

vk2d::_internal::ResourceManagerImpl::~ResourceManagerImpl()
{
	// Wait for all resources to finish loading, giving time to finish.
	while( true ) {
		bool loaded = true;
		{
			std::unique_lock<std::recursive_mutex> unique_lock( resources_mutex );

			auto it = resources.begin();
			while( it != resources.end() ) {
				if( !( *it )->IsLoaded() ) {
					if( !( *it )->FailedToLoad() ) {
						// Not completely loaded yet
						loaded = false;
					}
				}
				// Okay to continue
				++it;
			}
			if( loaded ) break;
		}
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
	}

	// Everythign should be up to date now and we can start scheduling resource unloading.
	{
		std::unique_lock<std::recursive_mutex> unique_lock( resources_mutex );

		auto it = resources.begin();
		while( it != resources.end() ) {
			( *it )->WaitUntilLoaded();
			thread_pool->ScheduleTask(
				std::make_unique<vk2d::_internal::UnloadTask>(
					this, std::move( *it )
					),
				{ ( *it )->GetLoaderThread() }
			);
			it = resources.erase( it );
		}
	}

	thread_pool->WaitIdle();
}



vk2d::TextureResource * vk2d::_internal::ResourceManagerImpl::LoadTextureResource(
	const std::filesystem::path			&	file_path,
	vk2d::Resource						*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<vk2d::TextureResource>(
		new vk2d::TextureResource(
			parent_resource,
			this,
			SelectLoaderThread(),
			file_path )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::_internal::ResourceManagerImpl::CreateTextureResource(
	vk2d::Vector2u							size,
	const std::vector<vk2d::Color8>		&	texture_data,
	vk2d::Resource						*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<vk2d::TextureResource>(
		new vk2d::TextureResource(
			parent_resource,
			this,
			SelectLoaderThread(),
			size,
			texture_data
		)
	);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::_internal::ResourceManagerImpl::LoadArrayTextureResource(
	const std::vector<std::filesystem::path>		&	file_path_listing,
	vk2d::Resource									*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<vk2d::TextureResource>(
		new vk2d::TextureResource(
			parent_resource,
			this,
			SelectLoaderThread(),
			file_path_listing )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::_internal::ResourceManagerImpl::CreateArrayTextureResource(
	vk2d::Vector2u										size,
	const std::vector<std::vector<vk2d::Color8>*>	&	texture_data_listings,
	vk2d::Resource									*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		=
		std::unique_ptr<vk2d::TextureResource>(
			new vk2d::TextureResource(
				parent_resource,
				this,
				SelectLoaderThread(),
				size,
				texture_data_listings
			)
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::FontResource * vk2d::_internal::ResourceManagerImpl::LoadFontResource(
	const std::filesystem::path			&	file_path,
	vk2d::Resource						*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		=
		std::unique_ptr<vk2d::FontResource>(
			new vk2d::FontResource(
				parent_resource,
				this,
				SelectLoaderThread(),
				file_path
			)
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

void vk2d::_internal::ResourceManagerImpl::DestroyResource(
	vk2d::Resource		*	resource
)
{
 	if( !resource ) return;

	// We'll have to wait until the resource is definitely loaded, or encountered an error.
	resource->WaitUntilLoaded();
	resource->DestroySubresources();

	std::lock_guard<std::recursive_mutex> lock_guard( resources_mutex );

	// find resource if it exists
	auto it = resources.begin();
	while( it != resources.end() ) {
		if( it->get() == resource ) {
			// Found resource in the resources list
			thread_pool->ScheduleTask( std::make_unique<vk2d::_internal::UnloadTask>( this, std::move( *it ) ), { resource->GetLoaderThread() } );
			it = resources.erase( it );
			return;
		} else {
			++it;
		}
	}
}

vk2d::_internal::RendererImpl * vk2d::_internal::ResourceManagerImpl::GetRenderer() const
{
	return renderer_parent;
}

vk2d::_internal::ThreadPool * vk2d::_internal::ResourceManagerImpl::GetThreadPool() const
{
	return thread_pool;
}

const std::vector<uint32_t> & vk2d::_internal::ResourceManagerImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & vk2d::_internal::ResourceManagerImpl::GetGeneralThreads() const
{
	return general_threads;
}

VkDevice vk2d::_internal::ResourceManagerImpl::GetVulkanDevice() const
{
	return device;
}

bool vk2d::_internal::ResourceManagerImpl::IsGood() const
{
	return is_good;
}

void vk2d::_internal::ResourceManagerImpl::ScheduleResourceLoad( vk2d::Resource * resource_ptr )
{
	thread_pool->ScheduleTask(
		std::make_unique<vk2d::_internal::LoadTask>(
			this,
			resource_ptr
			),
		{ resource_ptr->loader_thread }
	);
}

uint32_t vk2d::_internal::ResourceManagerImpl::SelectLoaderThread()
{
	auto load_thread = loader_threads[ current_loader_thread_index++ ];
	if( current_loader_thread_index >= uint32_t( loader_threads.size() ) ) current_loader_thread_index = 0;

	return load_thread;
}
