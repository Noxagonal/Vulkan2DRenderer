#pragma once

#include <core/SourceCommon.hpp>

#include <interface/resource_manager/ResourceManager.hpp>

#include <system/ThreadPool.hpp>

#include <containers/Color.hpp>



namespace vk2d {

class ResourceManager;
class ResourceBase;
class TextureResource;
class FontResource;
class MaterialResource;

namespace vk2d_internal {

class InstanceImpl;
class ResourceManagerImpl;
class ThreadPool;
class ResourceImplBase;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load task works on the resource list through pointers
class ResourceThreadLoadTask : public Task
{
public:
	ResourceThreadLoadTask(
		ResourceManagerImpl				&	resource_manager,
		ResourceBase					*	resource
	);

	TaskInvokeResult						operator()(
		LocalThreadData					*	thread_resource
	);

private:
	ResourceManagerImpl					&	resource_manager;
	ResourceBase						*	resource;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unload task takes ownership of the task
class ResourceThreadUnloadTask : public Task
{
public:
	ResourceThreadUnloadTask(
		ResourceManagerImpl				&	resource_manager,
		std::unique_ptr<ResourceBase>		resource
	);

	TaskInvokeResult						operator()(
		LocalThreadData					*	thread_resource
	);

private:
	ResourceManagerImpl					&	resource_manager;
	std::unique_ptr<ResourceBase>			resource;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ResourceManagerImpl {
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceManagerImpl(
		ResourceManager									&	my_interface,
		InstanceImpl									&	instance
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~ResourceManagerImpl();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceManager										&	GetInterface();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TextureResource										*	DoLoadTextureResource(
		const std::filesystem::path						&	file_path,
		ResourceBase									*	parent_resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TextureResource										*	DoCreateTextureResource(
		glm::uvec2											size,
		const std::vector<Color8>						&	texture_data,
		ResourceBase									*	parent_resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TextureResource										*	DoLoadArrayTextureResource(
		const std::vector<std::filesystem::path>		&	file_path_listings,
		ResourceBase									*	parent_resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TextureResource										*	DoCreateArrayTextureResource(
		glm::uvec2											size,
		const std::vector<const std::vector<Color8>*>	&	texture_data_listings,
		ResourceBase									*	parent_resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FontResource										*	DoLoadFontResource(
		const std::filesystem::path						&	file_path,
		ResourceBase									*	parent_resource,
		uint32_t											glyph_texel_size,
		bool												use_alpha,
		uint32_t											fallback_character,
		uint32_t											glyph_atlas_padding
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MaterialResource									*	DoCreateMaterialResource(
		ResourceBase									*	parent_resource,
		std::span<ShaderMemberInfo>							vertex_members,
		const MaterialCreateInfo						&	create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void													DestroyResource(
		ResourceBase									*	resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	InstanceImpl										&	GetInstance() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ThreadPool											&	GetThreadPool() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const std::vector<uint32_t>							&	GetLoaderThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const std::vector<uint32_t>							&	GetGeneralThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkDevice												GetVulkanDevice() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool													IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CALL ONLY FROM "AttachResource()".
	// Schedules the resource to be loaded after it's attached.
	void													ScheduleResourceLoad(
		ResourceBase									*	resource_ptr
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Some resources will need to use the same thread where they were originally created, for example if a resource uses a memory
	// pool from a thread, memory should be freed to the same thread memory pool, idea of per thread resource scheme is to reduce
	// mutex usage. This is just to select a loader thread prior to resource loading.
	uint32_t												SelectLoaderThread();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Return the loader thread for materials, materials must be single threaded so they only get one thread.
	uint32_t												SelectMaterialLoaderThread();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Same as SelectLoaderThread() but for general threads that do not involve GPU or file system loading.
	uint32_t												SelectGeneralThread();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Take ownership of the resource and put it into a load queue.
	// Returns raw pointer to the resource after it's been attached.
	template<typename T>
	T													*	AttachResource(
		std::unique_ptr<T>									resource
	)
	{
		static_assert( std::is_base_of_v<ResourceBase, T>, "<T> must be resource type." );

		auto resource_ptr = resource.get();
		std::lock_guard<std::recursive_mutex>		resources_lock( resources_mutex );
		resources.push_back( std::move( resource ) );
		ScheduleResourceLoad( resource_ptr );
		return resource_ptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceManager										&	my_interface;
	InstanceImpl										&	instance;
	VkDevice												vk_device							= {};

	ThreadPool											&	thread_pool;
	std::vector<uint32_t>									loader_threads						= {};
	uint32_t												material_loader_thread				= {};
	std::vector<uint32_t>									general_threads						= {};

	// TODO: This only cycles through loader threads for every new load operation,
	// a more advanced load balancer could be more appropriate, for now it's no big deal.
	uint32_t												current_loader_thread_index			= {};
	uint32_t												current_general_thread_index		= {};

	std::recursive_mutex									resources_mutex						= {};
	std::list<std::unique_ptr<ResourceBase>>				resources							= {};

	bool													is_good								= {};
};



} // vk2d_internal



}
