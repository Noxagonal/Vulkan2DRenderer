
#include "../../Core/SourceCommon.h"

#include "../../System/ThreadPool.h"

#include "../../Interface/InstanceImpl.h"

#include "../../../Include/Interface/ResourceManager/ResourceManager.h"
#include "ResourceManagerImpl.h"

#include "../../../Include/Interface/ResourceManager/Resource.h"
#include "ResourceImpl.h"

#include "../../../Include/Interface/ResourceManager/TextureResource.h"

#include "../../../Include/Interface/ResourceManager/FontResource.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::ResourceManager::ResourceManager(
	vk2d::_internal::InstanceImpl		*	parent_instance
)
{
	impl = std::make_unique<vk2d::_internal::ResourceManagerImpl>( parent_instance );
	if( !impl || !impl->IsGood() ) {
		impl	= nullptr;
		parent_instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create resource manager implementation!" );
		return;
	}
}

VK2D_API vk2d::ResourceManager::~ResourceManager()
{}

VK2D_API vk2d::TextureResource * VK2D_APIENTRY vk2d::ResourceManager::CreateTextureResource(
	vk2d::Vector2u						size,
	const std::vector<vk2d::Color8>	&	texels
)
{
	return impl->CreateTextureResource(
		size,
		texels,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * VK2D_APIENTRY vk2d::ResourceManager::LoadTextureResource(
	const std::filesystem::path		&	file_path
)
{
	return impl->LoadTextureResource(
		file_path,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * VK2D_APIENTRY vk2d::ResourceManager::CreateArrayTextureResource(
	vk2d::Vector2u										size,
	const std::vector<std::vector<vk2d::Color8>*>	&	texels_listing
)
{
	return impl->CreateArrayTextureResource(
		size,
		texels_listing,
		nullptr
	);
}

VK2D_API vk2d::TextureResource * VK2D_APIENTRY vk2d::ResourceManager::LoadArrayTextureResource(
	const std::vector<std::filesystem::path>		&	file_path_listing
)
{
	return impl->LoadArrayTextureResource(
		file_path_listing,
		nullptr
	);
}

VK2D_API vk2d::FontResource * VK2D_APIENTRY vk2d::ResourceManager::LoadFontResource(
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

VK2D_API void VK2D_APIENTRY vk2d::ResourceManager::DestroyResource(
	vk2d::Resource		*	resource
)
{
	impl->DestroyResource( resource );
}

VK2D_API bool VK2D_APIENTRY vk2d::ResourceManager::IsGood() const
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







vk2d::_internal::ResourceThreadLoadTask::ResourceThreadLoadTask(
	vk2d::_internal::ResourceManagerImpl	*	resource_manager,
	vk2d::Resource							*	resource
) :
	resource_manager( resource_manager ),
	resource( resource )
{};

void vk2d::_internal::ResourceThreadLoadTask::operator()(
	vk2d::_internal::ThreadPrivateResource	*	thread_resource
)
{
	if( !resource->resource_impl->MTLoad( thread_resource ) ) {
		resource->resource_impl->failed_to_load	= true;
		resource_manager->GetInstance()->Report( vk2d::ReportSeverity::WARNING, "Resource loading failed!" );
	}
	resource->resource_impl->load_function_ran		= true;
}



vk2d::_internal::ResourceThreadUnloadTask::ResourceThreadUnloadTask(
	vk2d::_internal::ResourceManagerImpl	*	resource_manager,
	std::unique_ptr<vk2d::Resource>				resource
) :
	resource_manager( resource_manager ),
	resource( std::move( resource ) )
{};

void vk2d::_internal::ResourceThreadUnloadTask::operator()(
	vk2d::_internal::ThreadPrivateResource	*	thread_resource
	)
{
	resource->resource_impl->MTUnload( thread_resource );
}



vk2d::_internal::ResourceManagerImpl::ResourceManagerImpl(
	vk2d::_internal::InstanceImpl	*	parent_instance
)
{
	instance			= parent_instance;
	assert( instance );

	vk_device		= instance->GetVulkanDevice();
	assert( vk_device );

	thread_pool		= instance->GetThreadPool();
	assert( thread_pool );

	loader_threads	= instance->GetLoaderThreads();

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
			( *it )->resource_impl->WaitUntilLoaded();
			thread_pool->ScheduleTask(
				std::make_unique<vk2d::_internal::ResourceThreadUnloadTask>(
					this, std::move( *it )
					),
				{ ( *it )->resource_impl->GetLoaderThread() }
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
			this,
			SelectLoaderThread(),
			parent_resource,
			{ file_path } )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
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
			this,
			SelectLoaderThread(),
			parent_resource,
			size,
			texture_data
		)
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
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
			this,
			SelectLoaderThread(),
			parent_resource,
			file_path_listing )
		);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
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
				this,
				SelectLoaderThread(),
				parent_resource,
				size,
				texture_data_listings
			)
			);
	if( !resource || !resource->IsGood() ) {
		// Could not create resource.
		GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource handle!" );
		return nullptr;
	}

	return AttachResource( std::move( resource ) );
}

vk2d::FontResource * vk2d::_internal::ResourceManagerImpl::LoadFontResource(
	const std::filesystem::path			&	file_path,
	vk2d::Resource						*	parent_resource,
	uint32_t								glyph_texel_size,
	bool									use_alpha,
	uint32_t								fallback_character,
	uint32_t								glyph_atlas_padding
)
{
	std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );

	auto resource		=
		std::unique_ptr<vk2d::FontResource>(
			new vk2d::FontResource(
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
		GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font resource handle!" );
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
	resource->resource_impl->WaitUntilLoaded();
	resource->resource_impl->DestroySubresources();

	std::lock_guard<std::recursive_mutex> lock_guard( resources_mutex );

	// find resource if it exists
	auto it = resources.begin();
	while( it != resources.end() ) {
		if( it->get() == resource ) {
			// Found resource in the resources list
			thread_pool->ScheduleTask( std::make_unique<vk2d::_internal::ResourceThreadUnloadTask>( this, std::move( *it ) ), { resource->resource_impl->GetLoaderThread() } );
			it = resources.erase( it );
			return;
		} else {
			++it;
		}
	}
}

vk2d::_internal::InstanceImpl * vk2d::_internal::ResourceManagerImpl::GetInstance() const
{
	return instance;
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
	return vk_device;
}

bool vk2d::_internal::ResourceManagerImpl::IsGood() const
{
	return is_good;
}

void vk2d::_internal::ResourceManagerImpl::ScheduleResourceLoad( vk2d::Resource * resource_ptr )
{
	thread_pool->ScheduleTask(
		std::make_unique<vk2d::_internal::ResourceThreadLoadTask>(
			this,
			resource_ptr
			),
		{ resource_ptr->resource_impl->loader_thread }
	);
}

uint32_t vk2d::_internal::ResourceManagerImpl::SelectLoaderThread()
{
	auto load_thread = loader_threads[ current_loader_thread_index++ ];
	if( current_loader_thread_index >= uint32_t( loader_threads.size() ) ) current_loader_thread_index = 0;

	return load_thread;
}
