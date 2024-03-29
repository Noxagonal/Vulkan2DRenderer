
#include "core/SourceCommon.h"

#include "system/ThreadPool.h"

#include "interface/InstanceImpl.h"

#include "interface/resources/ResourceManager.h"
#include "interface/resources/ResourceManagerImpl.h"

#include "interface/resources/ResourceBase.h"
#include "interface/resources/ResourceImplBase.h"

#include "interface/resources/TextureResource.h"

#include "interface/resources/FontResource.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::ResourceManager::ResourceManager(
	vk2d_internal::InstanceImpl		*	parent_instance
)
{
	impl = std::make_unique<vk2d_internal::ResourceManagerImpl>(
		this,
		parent_instance
	);
	if( !impl || !impl->IsGood() ) {
		impl	= nullptr;
		parent_instance->Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create resource manager implementation!" );
		return;
	}
}

VK2D_API vk2d::ResourceManager::~ResourceManager()
{}

VK2D_API vk2d::TextureResource * vk2d::ResourceManager::CreateTextureResource(
	glm::uvec2						size,
	const std::vector<Color8>	&	texels
)
{
	return impl->CreateTextureResource(
		size,
		texels,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * vk2d::ResourceManager::LoadTextureResource(
	const std::filesystem::path		&	file_path
)
{
	return impl->LoadTextureResource(
		file_path,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * vk2d::ResourceManager::CreateArrayTextureResource(
	glm::uvec2											size,
	const std::vector<const std::vector<Color8>*>	&	texels_listing
)
{
	return impl->CreateArrayTextureResource(
		size,
		texels_listing,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * vk2d::ResourceManager::LoadArrayTextureResource(
	const std::vector<std::filesystem::path>		&	file_path_listing
)
{
	return impl->LoadArrayTextureResource(
		file_path_listing,
		nullptr
	);
}

VK2D_API vk2d::FontResource * vk2d::ResourceManager::LoadFontResource(
	const std::filesystem::path		&	file_path,
	uint32_t							glyph_texel_size,
	bool								use_alpha,
	uint32_t							fallback_character,
	uint32_t							glyph_atlas_padding
)
{
	return impl->LoadFontResource(
		file_path,
		nullptr,
		glyph_texel_size,
		use_alpha,
		fallback_character,
		glyph_atlas_padding
	);
}

VK2D_API void vk2d::ResourceManager::DestroyResource(
	ResourceBase		*	resource
)
{
	impl->DestroyResource( resource );
}

VK2D_API bool vk2d::ResourceManager::IsGood() const
{
	return !!impl;
}







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







vk2d::vk2d_internal::ResourceThreadLoadTask::ResourceThreadLoadTask(
	ResourceManagerImpl			*	resource_manager,
	ResourceBase				*	resource
) :
	resource_manager( resource_manager ),
	resource( resource )
{};

void vk2d::vk2d_internal::ResourceThreadLoadTask::operator()(
	ThreadPrivateResource	*	thread_resource
)
{
	// Because Vulkan often needs to do more processing afterwards resource status
	// is not set to "LOADED" here, it'll be determined by the resource itself.
	// However we can set resource status to "FAILED_TO_LOAD" at any time.

	if( !resource->resource_impl->MTLoad( thread_resource ) ) {
		resource->resource_impl->status = ResourceStatus::FAILED_TO_LOAD;
		resource_manager->GetInstance()->Report( ReportSeverity::WARNING, "Resource loading failed!" );
	}
	resource->resource_impl->load_function_run_fence.Set();
}



vk2d::vk2d_internal::ResourceThreadUnloadTask::ResourceThreadUnloadTask(
	ResourceManagerImpl	*	resource_manager,
	std::unique_ptr<ResourceBase>			resource
) :
	resource_manager( resource_manager ),
	resource( std::move( resource ) )
{};

void vk2d::vk2d_internal::ResourceThreadUnloadTask::operator()(
	ThreadPrivateResource	*	thread_resource
	)
{
	resource->resource_impl->MTUnload( thread_resource );
}



vk2d::vk2d_internal::ResourceManagerImpl::ResourceManagerImpl(
	ResourceManager			*	my_interface,
	InstanceImpl	*	parent_instance
)
{
	this->my_interface		= my_interface;
	this->instance			= parent_instance;
	this->vk_device			= instance->GetVulkanDevice();
	this->thread_pool		= instance->GetThreadPool();
	this->loader_threads	= instance->GetLoaderThreads();

	assert( this->my_interface );
	assert( this->instance );
	assert( this->vk_device );
	assert( this->thread_pool );
	assert( std::size( this->loader_threads ) );

	is_good		= true;
}

vk2d::vk2d_internal::ResourceManagerImpl::~ResourceManagerImpl()
{
	// Wait for all resources to finish loading, giving time to finish.
	while( true ) {
		bool all_resources_status_determined = true;
		{
			std::unique_lock<std::recursive_mutex> unique_lock( resources_mutex );

			auto it = resources.begin();
			while( it != resources.end() ) {
				if( ( *it )->GetStatus() == ResourceStatus::UNDETERMINED ) {
					// Resource status is still undetermined
					all_resources_status_determined = false;
				}
				// Okay to continue
				++it;
			}
			if( all_resources_status_determined ) break;
		}
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
	}

	// Everythign should be up to date now and we can start scheduling resource unloading.
	{
		std::unique_lock<std::recursive_mutex> unique_lock( resources_mutex );

		auto it = resources.begin();
		while( it != resources.end() ) {
			( *it )->resource_impl->WaitUntilLoaded();
			thread_pool->ScheduleTask(
				std::make_unique<ResourceThreadUnloadTask>(
					this, std::move( *it )
					),
				{ ( *it )->resource_impl->GetLoaderThread() }
			);
			it = resources.erase( it );
		}
	}

	thread_pool->WaitIdle();
}



vk2d::TextureResource * vk2d::vk2d_internal::ResourceManagerImpl::LoadTextureResource(
	const std::filesystem::path			&	file_path,
	ResourceBase					*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<TextureResource>(
		new TextureResource(
			this,
			SelectLoaderThread(),
			parent_resource,
			{ file_path } )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::vk2d_internal::ResourceManagerImpl::CreateTextureResource(
	glm::uvec2								size,
	const std::vector<Color8>		&	texture_data,
	ResourceBase					*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<TextureResource>(
		new TextureResource(
			this,
			SelectLoaderThread(),
			parent_resource,
			size,
			{ &texture_data }
		)
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::vk2d_internal::ResourceManagerImpl::LoadArrayTextureResource(
	const std::vector<std::filesystem::path>		&	file_path_listing,
	ResourceBase								*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		= std::unique_ptr<TextureResource>(
		new TextureResource(
			this,
			SelectLoaderThread(),
			parent_resource,
			file_path_listing )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::TextureResource * vk2d::vk2d_internal::ResourceManagerImpl::CreateArrayTextureResource(
	glm::uvec2											size,
	const std::vector<const std::vector<Color8>*>	&	texture_data_listings,
	ResourceBase									*	parent_resource )
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		=
		std::unique_ptr<TextureResource>(
			new TextureResource(
				this,
				SelectLoaderThread(),
				parent_resource,
				size,
				texture_data_listings
			)
			);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::FontResource * vk2d::vk2d_internal::ResourceManagerImpl::LoadFontResource(
	const std::filesystem::path			&	file_path,
	ResourceBase					*	parent_resource,
	uint32_t								glyph_texel_size,
	bool									use_alpha,
	uint32_t								fallback_character,
	uint32_t								glyph_atlas_padding
)
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		=
		std::unique_ptr<FontResource>(
			new FontResource(
				this,
				SelectLoaderThread(),
				parent_resource,
				file_path,
				glyph_texel_size,
				use_alpha,
				fallback_character,
				glyph_atlas_padding
			)
			);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

void vk2d::vk2d_internal::ResourceManagerImpl::DestroyResource(
	ResourceBase	*	resource
)
{
	if( !resource ) return;

	// We'll have to wait until the resource is definitely loaded, or encountered an error.
	resource->resource_impl->WaitUntilLoaded();
	resource->resource_impl->DestroySubresources();

	std::lock_guard<std::recursive_mutex> lock_guard( resources_mutex );

	// find resource if it exists
	auto it = resources.begin();
	while( it != resources.end() ) {
		if( it->get() == resource ) {
			// Found resource in the resources list
			thread_pool->ScheduleTask( std::make_unique<ResourceThreadUnloadTask>( this, std::move( *it ) ), { resource->resource_impl->GetLoaderThread() } );
			it = resources.erase( it );
			return;
		} else {
			++it;
		}
	}
}

vk2d::vk2d_internal::InstanceImpl * vk2d::vk2d_internal::ResourceManagerImpl::GetInstance() const
{
	return instance;
}

vk2d::vk2d_internal::ThreadPool * vk2d::vk2d_internal::ResourceManagerImpl::GetThreadPool() const
{
	return thread_pool;
}

const std::vector<uint32_t> & vk2d::vk2d_internal::ResourceManagerImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & vk2d::vk2d_internal::ResourceManagerImpl::GetGeneralThreads() const
{
	return general_threads;
}

VkDevice vk2d::vk2d_internal::ResourceManagerImpl::GetVulkanDevice() const
{
	return vk_device;
}

bool vk2d::vk2d_internal::ResourceManagerImpl::IsGood() const
{
	return is_good;
}

void vk2d::vk2d_internal::ResourceManagerImpl::ScheduleResourceLoad(
	ResourceBase * resource_ptr
)
{
	thread_pool->ScheduleTask(
		std::make_unique<ResourceThreadLoadTask>(
			this,
			resource_ptr
			),
		{ resource_ptr->resource_impl->loader_thread }
	);
}

uint32_t vk2d::vk2d_internal::ResourceManagerImpl::SelectLoaderThread()
{
	auto load_thread = loader_threads[ current_loader_thread_index++ ];
	if( current_loader_thread_index >= uint32_t( loader_threads.size() ) ) current_loader_thread_index = 0;

	return load_thread;
}
