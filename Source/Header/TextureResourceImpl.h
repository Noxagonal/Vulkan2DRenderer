#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/TextureResource.h"
#include "../Header/VulkanMemoryManagement.h"
#include "../Header/DescriptorSet.h"
#include "../Header/ThreadPrivateResources.h"


namespace vk2d {

namespace _internal {

class ResourceManagerImpl;



class TextureResourceImpl
{
	friend class TextureResource;
	friend class _internal::ResourceManagerImpl;

public:
	TextureResourceImpl(
		TextureResource							*	texture_resource_parent,
		_internal::ResourceManagerImpl			*	resource_manager );

	~TextureResourceImpl();

	bool											MTLoad(
		_internal::ThreadPrivateResource		*	thread_resource );

	void											MTUnload(
		_internal::ThreadPrivateResource		*	thread_resource );

	bool											IsLoaded();

	bool											IsGood();

private:
	TextureResource								*	parent								= {};
	_internal::ResourceManagerImpl				*	resource_manager					= {};
	ThreadLoaderResource						*	loader_thread_resource				= {};

	CompleteBufferResource							staging_buffer						= {};
	CompleteImageResource							image								= {};

	PoolDescriptorSet								descriptor_set						= {};

	VkCommandBuffer									primary_render_command_buffer		= {};
	VkCommandBuffer									secondary_render_command_buffer		= {};
	VkCommandBuffer									primary_transfer_command_buffer		= {};

	VkSemaphore										transfer_semaphore					= {};
	VkSemaphore										blit_semaphore						= {};
	VkFence											texture_complete_fence				= {};

	bool											is_loaded							= {};
	bool											is_good								= {};
};



} // _internal

} // vk2d
