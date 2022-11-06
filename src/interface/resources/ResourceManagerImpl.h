#pragma once

#include "core/SourceCommon.h"

#include "system/ThreadPool.h"

#include "types/Color.hpp"

namespace vk2d {

class ResourceManager;
class ResourceBase;
class TextureResource;
class FontResource;

namespace vk2d_internal {

class InstanceImpl;
class ResourceManagerImpl;
class ThreadPool;
class ResourceImplBase;



// Load task works on the resource list through pointers
class ResourceThreadLoadTask : public Task
{
public:
	ResourceThreadLoadTask(
		ResourceManagerImpl				&	resource_manager,
		ResourceBase					*	resource );

	void operator()(
		ThreadPrivateResource			*	thread_resource );

private:
	ResourceManagerImpl					&	resource_manager;
	ResourceBase						*	resource;
};

// Unload task takes ownership of the task
class ResourceThreadUnloadTask : public Task
{
public:
	ResourceThreadUnloadTask(
		ResourceManagerImpl				&	resource_manager,
		std::unique_ptr<ResourceBase>		resource );

	void operator()(
		ThreadPrivateResource			*	thread_resource );

private:
	ResourceManagerImpl					&	resource_manager;
	std::unique_ptr<ResourceBase>			resource;
};



class ResourceManagerImpl {
public:
	ResourceManagerImpl(
		ResourceManager									&	my_interface,
		InstanceImpl									&	parent_instance
	);
	~ResourceManagerImpl();

	TextureResource										*	LoadTextureResource(
		const std::filesystem::path						&	file_path,
		ResourceBase									*	parent_resource
	);

	TextureResource										*	CreateTextureResource(
		glm::uvec2											size,
		const std::vector<Color8>						&	texture_data,
		ResourceBase									*	parent_resource
	);

	TextureResource										*	LoadArrayTextureResource(
		const std::vector<std::filesystem::path>		&	file_path_listings,
		ResourceBase									*	parent_resource
	);

	TextureResource										*	CreateArrayTextureResource(
		glm::uvec2											size,
		const std::vector<const std::vector<Color8>*>	&	texture_data_listings,
		ResourceBase									*	parent_resource
	);

	FontResource										*	LoadFontResource(
		const std::filesystem::path						&	file_path,
		ResourceBase									*	parent_resource,
		uint32_t											glyph_texel_size,
		bool												use_alpha,
		uint32_t											fallback_character,
		uint32_t											glyph_atlas_padding
	);

	void													DestroyResource(
		ResourceBase									*	resource
	);

	InstanceImpl										&	GetInstance() const;
	ThreadPool											&	GetThreadPool() const;
	const std::vector<uint32_t>							&	GetLoaderThreads() const;
	const std::vector<uint32_t>							&	GetGeneralThreads() const;
	VkDevice												GetVulkanDevice() const;

	bool													IsGood() const;

private:
	// CALL ONLY FROM "AttachResource()".
	// Schedules the resource to be loaded after it's attached.
	void													ScheduleResourceLoad(
		ResourceBase									*	resource_ptr );

	// Some resources will need to use the same thread where they were
	// originally created, for example if a resource uses a memory pool
	// from a thread, memory should be freed to the same thread memory
	// pool, idea of per thread resource scheme is to reduce mutex usage.
	// This is just to select a loader thread prior to resource loading.
	uint32_t												SelectLoaderThread();

	// Take ownership of the resource and put it into a load queue.
	// Returns raw pointer to the resource after it's been attached.
	template<typename T>
	T													*	AttachResource(
		std::unique_ptr<T>									resource )
	{
		static_assert( std::is_base_of_v<ResourceBase, T>, "<T> must be resource type." );

		auto resource_ptr = resource.get();
		std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );
		resources.push_back( std::move( resource ) );
		ScheduleResourceLoad( resource_ptr );
		return resource_ptr;
	}

	ResourceManager										&	my_interface;
	InstanceImpl										&	instance;
	VkDevice												vk_device							= {};

	ThreadPool											&	thread_pool;
	std::vector<uint32_t>									loader_threads						= {};
	std::vector<uint32_t>									general_threads						= {};

	// TODO: This only cycles through loader threads for every new load operation,
	// a more advanced load balancer could be more appropriate, for now it's no big deal.
	uint32_t												current_loader_thread_index			= {};

	std::recursive_mutex									resources_mutex						= {};
	std::list<std::unique_ptr<ResourceBase>>				resources							= {};

	bool													is_good								= {};
};



} // vk2d_internal



}

