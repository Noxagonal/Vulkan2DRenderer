
#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/ThreadPrivateResources.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Core/DescriptorSet.h"
#include "../Header/Core/VulkanMemoryManagement.h"

#include <sstream>



vk2d::_internal::ThreadLoaderResource::ThreadLoaderResource( RendererImpl * renderer )
{
	renderer_parent		= renderer;
	device				= renderer->GetVulkanDevice();
}

vk2d::_internal::RendererImpl * vk2d::_internal::ThreadLoaderResource::GetRenderer() const
{
	return renderer_parent;
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

FT_Library vk2d::_internal::ThreadLoaderResource::GetFreeTypeInstance() const
{
	return freetype_instance;
}

bool vk2d::_internal::ThreadLoaderResource::ThreadBegin()
{
	// Initialize Vulkan stuff here

	// Command buffers
	{
		auto primary_render_queue_family_index			= renderer_parent->GetPrimaryRenderQueue().GetQueueFamilyIndex();
		auto secondary_render_queue_family_index		= renderer_parent->GetSecondaryRenderQueue().GetQueueFamilyIndex();
		auto primary_transfer_queue_family_index		= renderer_parent->GetPrimaryTransferQueue().GetQueueFamilyIndex();

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
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for primary render queue in thread: "
					<< std::this_thread::get_id();
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
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
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for secondary render queue in thread: "
					<< std::this_thread::get_id();
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
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
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for primary transfer queue in thread: "
					<< std::this_thread::get_id();
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
				return false;
			}
		}
	}

	// Descriptor pool
	{
		descriptor_auto_pool	= vk2d::_internal::CreateDescriptorAutoPool(
			renderer_parent,
			device
		);
		if( !descriptor_auto_pool ) {
			std::stringstream ss;
			ss << "Internal error: Cannot create descriptor auto pool in thread: "
				<< std::this_thread::get_id();
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
			return false;
		}
	}

	// Device memory pool
	{
		device_memory_pool			= vk2d::_internal::MakeDeviceMemoryPool(
			renderer_parent->GetVulkanPhysicalDevice(),
			device
		);
		if( !device_memory_pool ) {
			std::stringstream ss;
			ss << "Internal error: Cannot create device memory pool in thread: "
				<< std::this_thread::get_id();
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
			return false;
		}
	}

	// FreeType
	{
		auto ft_error = FT_Init_FreeType( &freetype_instance );
		if( ft_error ) {
			std::stringstream ss;
			ss << "Internal error: Cannot create FreeType instance in thread: "
				<< std::this_thread::get_id();
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, ss.str() );
			return false;
		}
	}

	return true;
}

void vk2d::_internal::ThreadLoaderResource::ThreadEnd()
{
	// FreeType
	FT_Done_FreeType( freetype_instance );
	freetype_instance		= nullptr;

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