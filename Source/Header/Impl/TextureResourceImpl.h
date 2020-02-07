#pragma once

#include "../../Header/Core/SourceCommon.h"
#include "../../../Include/Interface/TextureResource.h"
#include "../../Header/Core/VulkanMemoryManagement.h"
#include "../../Header/Core/DescriptorSet.h"
#include "../../Header/Core/ThreadPrivateResources.h"
#include "../../../Include/Interface/RenderPrimitives.h"


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
		vk2d::TextureResource							*	texture_resource_parent,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager );

															TextureResourceImpl(
		vk2d::TextureResource							*	texture_resource_parent,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::Vector2u										size,
		const std::vector<vk2d::Color8>					&	texels );

															TextureResourceImpl(
		vk2d::TextureResource							*	texture_resource_parent,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::Vector2u										size,
		const std::vector<std::vector<vk2d::Color8>*>	&	texels );

															~TextureResourceImpl();

	bool													MTLoad(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	void													MTUnload(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	bool													IsLoaded();
	bool													WaitUntilLoaded();

//	VkDescriptorSet											GetDescriptorSet() const;
	VkImage													GetVulkanImage() const;
	VkImageView												GetVulkanImageView() const;
	VkImageLayout											GetVulkanImageLayout() const;
	uint32_t												GetLayerCount() const;

	bool													IsGood() const;

private:
	void													ScheduleTextureLoadResourceDestruction();

	vk2d::TextureResource								*	texture_parent								= {};
	_internal::ResourceManagerImpl						*	resource_manager							= {};
	vk2d::_internal::ThreadLoaderResource				*	loader_thread_resource						= {};

	std::vector<vk2d::_internal::CompleteBufferResource>	staging_buffers								= {};
	vk2d::_internal::CompleteImageResource					image										= {};
	VkImageLayout											vk_image_layout								= {};

	uint32_t												image_layer_count							= {};
	VkExtent2D												extent										= {};
	std::vector<std::vector<vk2d::Color8>>					texture_data								= {};

//	vk2d::_internal::PoolDescriptorSet						descriptor_set								= {};

	VkCommandBuffer											vk_primary_render_command_buffer			= {};
	VkCommandBuffer											vk_secondary_render_command_buffer			= {};
	VkCommandBuffer											vk_primary_transfer_command_buffer			= {};

	VkSemaphore												vk_transfer_semaphore						= {};
	VkSemaphore												vk_blit_semaphore							= {};
	VkFence													vk_texture_complete_fence					= {};

	std::mutex												is_loaded_mutex								= {};
	bool													is_loaded									= {};
	bool													is_good										= {};
};



} // _internal

} // vk2d
