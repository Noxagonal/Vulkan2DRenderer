#pragma once

#include "SourceCommon.h"
#include "RendererImpl.h"
#include "../../Include/VK2D/TextureResource.h"

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
		_internal::RendererImpl					*	parent_renderer
	);
	~ResourceManagerImpl();

	TextureResource								*	LoadTextureResource(
		std::filesystem::path						file_path );

	TextureResource								*	CreateTextureResource(
		uint32_t									size_x,
		uint32_t									size_y,
		const std::vector<uint8_t>				&	texture_data );

	void											DestroyResource(
		Resource								*	resource );

	_internal::RendererImpl						*	GetRenderer() const;
	_internal::ThreadPool						*	GetThreadPool() const;
	const std::vector<uint32_t>					&	GetLoaderThreads() const;
	const std::vector<uint32_t>					&	GetGeneralThreads() const;
	VkDevice										GetVulkanDevice() const;

	bool											IsGood() const;

private:
	uint32_t										SelectLoaderThread();

	_internal::RendererImpl						*	parent								= {};
	VkDevice										device								= {};

	ThreadPool									*	thread_pool							= {};
	std::vector<uint32_t>							loader_threads						= {};
	std::vector<uint32_t>							general_threads						= {};

	// TODO: This only cycles through loader threads for every new load operation,
	// a more advanced load balancer could be more appropriate, for now it's no big deal.
	uint32_t										current_loader_thread_index			= {};

	std::mutex										resources_mutex						= {};
	std::list<std::unique_ptr<Resource>>			resources							= {};

	bool											is_good								= {};
};



} // _internal



}

