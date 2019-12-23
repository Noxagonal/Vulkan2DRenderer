#pragma once

#include "../Header/Core/SourceCommon.h"
#include "RendererImpl.h"
#include "../../../Include/Interface/TextureResource.h"

#include <list>
#include <filesystem>
#include <mutex>

namespace vk2d {

class Resource;

namespace _internal {

class ThreadPool;
class ResourceImpl;



class ResourceManagerImpl {
public:
	ResourceManagerImpl(
		vk2d::_internal::RendererImpl					*	parent_renderer
	);
	~ResourceManagerImpl();

	vk2d::TextureResource						*	LoadTextureResource(
		const std::filesystem::path				&	file_path );

	vk2d::TextureResource						*	CreateTextureResource(
		vk2d::Vector2u								size,
		const std::vector<vk2d::Color8>			&	texture_data );

	vk2d::TextureResource						*	CreateTextureResource(
		vk2d::Vector2u								size,
		const std::vector<vk2d::Color16>		&	texture_data );

	void											DestroyResource(
		vk2d::Resource							*	resource );

	vk2d::_internal::RendererImpl				*	GetRenderer() const;
	vk2d::_internal::ThreadPool					*	GetThreadPool() const;
	const std::vector<uint32_t>					&	GetLoaderThreads() const;
	const std::vector<uint32_t>					&	GetGeneralThreads() const;
	VkDevice										GetVulkanDevice() const;

	bool											IsGood() const;

private:
	// Some resources will need to use the same thread where they were
	// originally created, for example if a resource uses a memory pool
	// from a thread, memory should be freed to the same thread memory
	// pool, idea of per thread resource scheme is to reduce mutex usage.
	// This is just to select a loader thread prior to resource loading.
	uint32_t										SelectLoaderThread();

	vk2d::_internal::RendererImpl				*	renderer_parent						= {};
	VkDevice										device								= {};

	vk2d::_internal::ThreadPool					*	thread_pool							= {};
	std::vector<uint32_t>							loader_threads						= {};
	std::vector<uint32_t>							general_threads						= {};

	// TODO: This only cycles through loader threads for every new load operation,
	// a more advanced load balancer could be more appropriate, for now it's no big deal.
	uint32_t										current_loader_thread_index			= {};

	std::mutex										resources_mutex						= {};
	std::list<std::unique_ptr<vk2d::Resource>>		resources							= {};

	bool											is_good								= {};
};



} // _internal



}

