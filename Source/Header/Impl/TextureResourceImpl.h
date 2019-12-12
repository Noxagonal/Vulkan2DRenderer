#pragma once

#include "../Header/Core/SourceCommon.h"
#include "../../../Include/Interface/TextureResource.h"
#include "../../Header/Core/VulkanMemoryManagement.h"
#include "../../Header/Core/DescriptorSet.h"
#include "../../Header/Core/ThreadPrivateResources.h"


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

	VkExtent2D										extent										= {};
	std::vector<vk2d::Texel>						texture_data								= {};

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
