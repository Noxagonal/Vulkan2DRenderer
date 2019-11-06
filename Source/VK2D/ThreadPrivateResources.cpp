
#include "../Header/SourceCommon.h"

#include "../Header/ThreadPrivateResources.h"
#include "../Header/RendererImpl.h"

vk2d::_internal::ThreadLoaderResource::ThreadLoaderResource( RendererImpl * parent )
{
	this->parent	= parent;
	device			= parent->GetVulkanDevice();
}

vk2d::_internal::RendererImpl * vk2d::_internal::ThreadLoaderResource::GetRenderer() const
{
	return parent;
}

VkDevice vk2d::_internal::ThreadLoaderResource::GetVulkanDevice() const
{
	return device;
}

VkCommandPool vk2d::_internal::ThreadLoaderResource::GetPrimaryRenderCommandPool() const
{
	return primary_render_command_pool;
}

VkCommandPool vk2d::_internal::ThreadLoaderResource::GetSecondaryRenderCommandPool() const
{
	return secondary_render_command_pool;
}

VkCommandPool vk2d::_internal::ThreadLoaderResource::GetPrimaryTransferCommandPool() const
{
	return primary_transfer_command_pool;
}

bool vk2d::_internal::ThreadLoaderResource::ThreadBegin()
{
	// Initialize Vulkan stuff here

	// Command buffers
	{
		auto primary_render_queue_family_index			= parent->GetPrimaryRenderQueue().queueFamilyIndex;
		auto secondary_render_queue_family_index		= parent->GetSecondaryRenderQueue().queueFamilyIndex;
		auto primary_transfer_queue_family_index		= parent->GetPrimaryTransferQueue().queueFamilyIndex;

		VkCommandPoolCreateInfo command_pool_create_info {};
		command_pool_create_info.sType		= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext		= nullptr;
		command_pool_create_info.flags		= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		{
			command_pool_create_info.queueFamilyIndex	= primary_render_queue_family_index;
			if( vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&primary_render_command_pool
			) != VK_SUCCESS ) {
				return false;
			}
		}
		{
			command_pool_create_info.queueFamilyIndex	= secondary_render_queue_family_index;
			if( vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&secondary_render_command_pool
			) != VK_SUCCESS ) {
				return false;
			}
		}
		{
			command_pool_create_info.queueFamilyIndex	= primary_transfer_queue_family_index;
			if( vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&primary_transfer_command_pool
			) != VK_SUCCESS ) {
				return false;
			}
		}
	}

	// Descriptor pool
	// TODO: descriptor pool

	return true;
}

void vk2d::_internal::ThreadLoaderResource::ThreadEnd()
{
	// De-initialize Vulkan stuff here
	{
		vkDestroyCommandPool(
			device,
			primary_render_command_pool,
			nullptr
		);
		vkDestroyCommandPool(
			device,
			secondary_render_command_pool,
			nullptr
		);
		vkDestroyCommandPool(
			device,
			primary_transfer_command_pool,
			nullptr
		);
	}
}
