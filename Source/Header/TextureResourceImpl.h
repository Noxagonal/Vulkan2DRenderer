#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/TextureResource.h"
#include "../Header/VulkanMemoryManagement.h"
#include "../Header/DescriptorSet.h"
#include "../Header/ThreadPrivateResources.h"


namespace vk2d {

namespace _internal {

class ResourceManagerImpl;
class DestroyTextureLoadResources;


class TextureResourceImpl
{
	friend class vk2d::TextureResource;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::DestroyTextureLoadResources;

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
	bool											WaitUntilLoaded();

	VkDescriptorSet									GetDescriptorSet() const;

	bool											IsGood() const;

private:
	void											ScheduleTextureLoadResourceDestruction();

	TextureResource								*	texture										= {};
	_internal::ResourceManagerImpl				*	resource_manager							= {};
	ThreadLoaderResource						*	loader_thread_resource						= {};

	CompleteBufferResource							staging_buffer								= {};
	CompleteImageResource							image										= {};

	PoolDescriptorSet								descriptor_set								= {};

	VkCommandBuffer									primary_render_command_buffer				= {};
	VkCommandBuffer									secondary_render_command_buffer				= {};
	VkCommandBuffer									primary_transfer_command_buffer				= {};

	VkSemaphore										transfer_semaphore							= {};
	VkSemaphore										blit_semaphore								= {};
	VkFence											texture_complete_fence						= {};

	std::mutex										is_loaded_mutex								= {};
	bool											is_loaded									= {};
	bool											is_good										= {};
};



} // _internal

} // vk2d
