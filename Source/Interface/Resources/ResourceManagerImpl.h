#pragma once

#include "Core/SourceCommon.h"

#include "System/ThreadPool.h"

#include "Types/Color.hpp"

namespace vk2d {

class ResourceManager;
class ResourceBase;
class TextureResource;
class FontResource;

namespace _internal {

class InstanceImpl;
class ResourceManagerImpl;
class ThreadPool;
class ResourceImplBase;



// Load task
// Load task works on the resource list through pointers
class ResourceThreadLoadTask : public vk2d::_internal::Task
{
public:
	ResourceThreadLoadTask(
		vk2d::_internal::ResourceManagerImpl	*	resource_manager,
		vk2d::ResourceBase						*	resource );

	void operator()( vk2d::_internal::ThreadPrivateResource * thread_resource );

private:
	vk2d::_internal::ResourceManagerImpl		*	resource_manager		= {};
	vk2d::ResourceBase							*	resource				= {};
};

// Load more task
// Load task works on the resource list through pointers
class ResourceThreadLoadMoreTask : public vk2d::_internal::Task
{
public:
	ResourceThreadLoadMoreTask(
		vk2d::_internal::ResourceManagerImpl	*	resource_manager,
		vk2d::ResourceBase						*	resource );

	void operator()( vk2d::_internal::ThreadPrivateResource * thread_resource );

private:
	vk2d::_internal::ResourceManagerImpl		*	resource_manager		= {};
	vk2d::ResourceBase							*	resource				= {};
};

// Unload task
// Unload task takes ownership of the task
class ResourceThreadUnloadTask : public vk2d::_internal::Task
{
public:
	ResourceThreadUnloadTask(
		vk2d::_internal::ResourceManagerImpl	*	resource_manager,
		std::unique_ptr<vk2d::ResourceBase>			resource );

	void operator()( vk2d::_internal::ThreadPrivateResource * thread_resource );

private:
	vk2d::_internal::ResourceManagerImpl		*	resource_manager		= {};
	std::unique_ptr<vk2d::ResourceBase>				resource				= {};
};



class ResourceManagerImpl {
public:
	ResourceManagerImpl(
		vk2d::ResourceManager								*	my_interface,
		vk2d::_internal::InstanceImpl						*	parent_instance
	);
	~ResourceManagerImpl();

	vk2d::TextureResource									*	LoadTextureResource(
		const std::filesystem::path							&	file_path,
		vk2d::ResourceBase									*	parent_resource );

	vk2d::TextureResource									*	CreateTextureResource(
		glm::uvec2												size,
		const std::vector<vk2d::Color8>						&	texture_data,
		vk2d::ResourceBase									*	parent_resource );

	vk2d::TextureResource									*	LoadArrayTextureResource(
		const std::vector<std::filesystem::path>			&	file_path_listings,
		vk2d::ResourceBase									*	parent_resource );

	vk2d::TextureResource									*	CreateArrayTextureResource(
		glm::uvec2												size,
		const std::vector<const std::vector<vk2d::Color8>*>	&	texture_data_listings,
		vk2d::ResourceBase									*	parent_resource );

	vk2d::FontResource										*	LoadFontResource(
		const std::filesystem::path							&	file_path,
		vk2d::ResourceBase									*	parent_resource,
		uint32_t												glyph_texel_size,
		bool													use_alpha,
		uint32_t												fallback_character,
		uint32_t												glyph_atlas_padding );

	void														DestroyResource(
		vk2d::ResourceBase									*	resource );

	vk2d::_internal::InstanceImpl							*	GetInstance() const;
	vk2d::_internal::ThreadPool								*	GetThreadPool() const;
	const std::vector<uint32_t>								&	GetLoaderThreads() const;
	const std::vector<uint32_t>								&	GetGeneralThreads() const;
	VkDevice													GetVulkanDevice() const;

	bool														IsGood() const;

	/// @brief		Called by the resource when the resource needs further processing on
	///				a separate thread.
	/// @note		Multithreading: Any thread.
	/// @param[in]	resource_ptr
	///				Pointer to resource that should be processed further.
	void														RequestLoadMore(
		vk2d::ResourceBase									*	resource_ptr );

private:
	/// @brief		Schedules the resource to be loaded after it's attached.
	/// @note		CALL ONLY FROM "AttachResource()".
	/// @param resource_ptr 
	void														ScheduleResourceLoad(
		vk2d::ResourceBase									*	resource_ptr );

	/// @brief		Schedule resource to load more.
	/// @note		CALL ONLY FROM "RequestLoadMore()".
	/// @param		resource_ptr
	///				Pointer to resource to process further.
	void														ScheduleResourceLoadMore(
		vk2d::ResourceBase									*	resource_ptr );

	/// @brief		Some resources will need to use the same thread where they were
	///				originally created, for example if a resource uses a memory pool
	///				from a thread, memory should be freed to the same thread memory
	///				pool, idea of per thread resource scheme is to reduce mutex usage.
	///				This is just to select a loader thread prior to resource loading.
	/// @return		Loader thread allocated for the resource.
	uint32_t													SelectLoaderThread();

	/// @brief		Take ownership of the resource and put it into a load queue.
	/// @note		Multithreading: Any thread.
	/// @tparam		T
	///				Resource type. Must be derived from vk2d::ResourceBase.
	/// @param[in]	resource
	///				Resource to take ownership of.
	/// @return		Raw pointer to the resource after it's been attached.
	template<typename T>
	T														*	AttachResource(
		std::unique_ptr<T>										resource )
	{
		static_assert( std::is_base_of_v<vk2d::ResourceBase, T>, "<T> must be resource type." );

		auto resource_ptr = resource.get();
		std::lock_guard<std::recursive_mutex> resources_lock( resources_mutex );
		resources.push_back( std::move( resource ) );
		ScheduleResourceLoad( resource_ptr );
		return resource_ptr;
	}

	vk2d::ResourceManager									*	my_interface						= {};
	vk2d::_internal::InstanceImpl							*	instance							= {};
	VkDevice													vk_device							= {};

	vk2d::_internal::ThreadPool								*	thread_pool							= {};
	std::vector<uint32_t>										loader_threads						= {};
	std::vector<uint32_t>										general_threads						= {};

	// TODO: This only cycles through loader threads for every new load operation,
	// a more advanced load balancer could be more appropriate, for now it's no big deal.
	uint32_t													current_loader_thread_index			= {};

	std::recursive_mutex										resources_mutex						= {};
	std::list<std::unique_ptr<vk2d::ResourceBase>>				resources							= {};

	bool														is_good								= {};
};



} // _internal



}

