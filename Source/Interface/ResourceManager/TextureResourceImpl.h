#pragma once

#include "../../Core/SourceCommon.h"

#include "../../../Include/Types/Vector2.hpp"
#include "../../../Include/Types/Color.hpp"

#include "../../System/VulkanMemoryManagement.h"

#include "ResourceImpl.h"
#include "../TextureImpl.h"


namespace vk2d {

class TextureResource;

namespace _internal {

class ResourceManagerImpl;
class DestroyTextureLoadResources;
class ThreadLoaderResource;
class ThreadPrivateResource;



class TextureResourceImpl :
	public vk2d::_internal::ResourceImpl,
	public vk2d::_internal::TextureImpl
{
	friend class vk2d::TextureResource;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::DestroyTextureLoadResources;

public:
															TextureResourceImpl(
		vk2d::TextureResource							*	my_interface,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread,
		vk2d::Resource									*	parent_resource,
		const std::vector<std::filesystem::path>		&	file_paths_listing );

															TextureResourceImpl(
		vk2d::TextureResource							*	my_interface,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread,
		vk2d::Resource									*	parent_resource,
		vk2d::Vector2u										size,
		const std::vector<vk2d::Color8>					&	texels );

															TextureResourceImpl(
		vk2d::TextureResource							*	my_interface,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread,
		vk2d::Resource									*	parent_resource,
		vk2d::Vector2u										size,
		const std::vector<std::vector<vk2d::Color8>*>	&	texels );

	bool													MTLoad(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	void													MTUnload(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	vk2d::ResourceStatus									GetStatus();

	vk2d::ResourceStatus									WaitUntilLoaded(
		std::chrono::nanoseconds							timeout );

	vk2d::ResourceStatus									WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	VkImage													GetVulkanImage() const;
	VkImageView												GetVulkanImageView() const;
	VkImageLayout											GetVulkanImageLayout() const;

	vk2d::Vector2u											GetSize() const;
	uint32_t												GetLayerCount() const;

	bool													IsTextureDataReady();

	bool													IsGood() const;

private:
	void													ScheduleTextureLoadResourceDestruction();

	vk2d::TextureResource								*	my_interface								= {};
	vk2d::_internal::ResourceManagerImpl				*	resource_manager							= {};
	vk2d::_internal::ThreadLoaderResource				*	loader_thread_resource						= {};

	std::vector<vk2d::_internal::CompleteBufferResource>	staging_buffers								= {};
	vk2d::_internal::CompleteImageResource					image										= {};
	VkImageLayout											vk_image_layout								= {};

	uint32_t												image_layer_count							= {};
	VkExtent2D												extent										= {};
	std::vector<std::vector<vk2d::Color8>>					texture_data								= {};

	VkCommandBuffer											vk_primary_render_command_buffer			= {};
	VkCommandBuffer											vk_secondary_render_command_buffer			= {};
	VkCommandBuffer											vk_primary_transfer_command_buffer			= {};

	VkSemaphore												vk_transfer_semaphore						= {};
	VkSemaphore												vk_blit_semaphore							= {};
	VkFence													vk_texture_complete_fence					= {};

	bool													is_good										= {};
};



} // _internal

} // vk2d
