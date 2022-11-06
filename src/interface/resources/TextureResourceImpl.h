#pragma once

#include <core/SourceCommon.h>

#include <types/Color.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>

#include <interface/resources/TextureResource.h>
#include "ResourceImplBase.h"
#include <interface/TextureImpl.h>


namespace vk2d {

class TextureResource;

namespace vk2d_internal {

class ResourceManagerImpl;
class DestroyTextureLoadResources;
class ThreadLoaderResource;
class ThreadPrivateResource;



class TextureResourceImpl :
	public ResourceImplBase,
	public TextureImpl
{
	friend class TextureResource;
	friend class ResourceManagerImpl;
	friend class DestroyTextureLoadResources;

public:
															TextureResourceImpl(
		TextureResource									&	my_interface,
		ResourceManagerImpl								&	resource_manager,
		uint32_t											loader_thread,
		ResourceBase									*	parent_resource,
		const std::vector<std::filesystem::path>		&	file_paths_listing );

															TextureResourceImpl(
		TextureResource									&	my_interface,
		ResourceManagerImpl								&	resource_manager,
		uint32_t											loader_thread,
		ResourceBase									*	parent_resource,
		glm::uvec2											size,
		const std::vector<const std::vector<Color8>*>	&	texels );

	bool													MTLoad(
		ThreadPrivateResource							*	thread_resource );

	void													MTUnload(
		ThreadPrivateResource							*	thread_resource );

	ResourceStatus											GetStatus();

	ResourceStatus											WaitUntilLoaded(
		std::chrono::nanoseconds							timeout );

	ResourceStatus											WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	VkImage													GetVulkanImage() const;
	VkImageView												GetVulkanImageView() const;
	VkImageLayout											GetVulkanImageLayout() const;

	glm::uvec2												GetSize() const;
	uint32_t												GetLayerCount() const;

	bool													IsTextureDataReady();

	bool													IsGood() const;

private:
	void													ScheduleTextureLoadResourceDestruction();

	TextureResource										&	my_interface;
	ResourceManagerImpl									&	resource_manager;
	ThreadLoaderResource								*	loader_thread_resource						= {};

	std::vector<CompleteBufferResource>						staging_buffers								= {};
	CompleteImageResource									image										= {};
	VkImageLayout											vk_image_layout								= {};

	uint32_t												image_layer_count							= {};
	VkExtent2D												extent										= {};
	std::vector<std::vector<Color8>>						texture_data								= {};

	VkCommandBuffer											vk_primary_render_command_buffer			= {};
	VkCommandBuffer											vk_secondary_render_command_buffer			= {};
	VkCommandBuffer											vk_primary_transfer_command_buffer			= {};

	VkSemaphore												vk_transfer_semaphore						= {};
	VkSemaphore												vk_blit_semaphore							= {};
	VkFence													vk_texture_complete_fence					= {};

	bool													is_good										= {};
};



} // vk2d_internal

} // vk2d
