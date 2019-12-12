
#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/ThreadPrivateResources.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Core/DescriptorSet.h"
#include "../Header/Core/VulkanMemoryManagement.h"

vk2d::_internal::ThreadLoaderResource::ThreadLoaderResource( RendererImpl * parent )
{
	this->renderer	= parent;
	device			= parent->GetVulkanDevice();
}

vk2d::_internal::RendererImpl * vk2d::_internal::ThreadLoaderResource::GetRenderer() const
{
	return renderer;
}

VkDevice vk2d::_internal::ThreadLoaderResource::GetVulkanDevice() const
{
	return device;
}

vk2d::_internal::DeviceMemoryPool * vk2d::_internal::ThreadLoaderResource::GetDeviceMemoryPool() const
{
	return device_memory_pool.get();
}

vk2d::_internal::DescriptorAutoPool * vk2d::_internal::ThreadLoaderResource::GetDescriptorAutoPool() const
{
	return descriptor_auto_pool.get();
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
		auto primary_render_queue_family_index			= renderer->GetPrimaryRenderQueue().GetQueueFamilyIndex();
		auto secondary_render_queue_family_index		= renderer->GetSecondaryRenderQueue().GetQueueFamilyIndex();
		auto primary_transfer_queue_family_index		= renderer->GetPrimaryTransferQueue().GetQueueFamilyIndex();

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
	{
		descriptor_auto_pool	= vk2d::_internal::CreateDescriptorAutoPool(
			device
		);
		if( !descriptor_auto_pool ) {
			return false;
		}
	}

	// Device memory pool
	device_memory_pool			= vk2d::_internal::MakeDeviceMemoryPool(
		renderer->GetVulkanPhysicalDevice(),
		device
	);
	if( !device_memory_pool ) {
		return false;
	}

	return true;
}

void vk2d::_internal::ThreadLoaderResource::ThreadEnd()
{
	// De-initialize Vulkan stuff here
	device_memory_pool		= nullptr;
	descriptor_auto_pool	= nullptr;

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
