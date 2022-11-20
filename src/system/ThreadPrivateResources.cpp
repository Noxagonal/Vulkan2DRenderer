
#include <core/SourceCommon.hpp>

#include <system/ThreadPrivateResources.hpp>
#include <system/DescriptorSet.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>

#include <interface/InstanceImpl.hpp>



vk2d::vk2d_internal::ThreadLoaderResource::ThreadLoaderResource(
	InstanceImpl & instance
) :
	instance( instance ),
	device( instance.GetVulkanDevice() )
{}

vk2d::vk2d_internal::InstanceImpl & vk2d::vk2d_internal::ThreadLoaderResource::GetInstance()
{
	return instance;
}

VkDevice vk2d::vk2d_internal::ThreadLoaderResource::GetVulkanDevice() const
{
	return device;
}

vk2d::vk2d_internal::DeviceMemoryPool * vk2d::vk2d_internal::ThreadLoaderResource::GetDeviceMemoryPool()
{
	return device_memory_pool.get();
}

vk2d::vk2d_internal::DescriptorAutoPool * vk2d::vk2d_internal::ThreadLoaderResource::GetDescriptorAutoPool()
{
	return descriptor_auto_pool.get();
}

VkCommandPool vk2d::vk2d_internal::ThreadLoaderResource::GetPrimaryRenderCommandPool() const
{
	return primary_render_command_pool;
}

VkCommandPool vk2d::vk2d_internal::ThreadLoaderResource::GetSecondaryRenderCommandPool() const
{
	return secondary_render_command_pool;
}

VkCommandPool vk2d::vk2d_internal::ThreadLoaderResource::GetPrimaryTransferCommandPool() const
{
	return primary_transfer_command_pool;
}

FT_Library vk2d::vk2d_internal::ThreadLoaderResource::GetFreeTypeInstance() const
{
	return freetype_instance;
}

bool vk2d::vk2d_internal::ThreadLoaderResource::ThreadBegin()
{
	// Initialize Vulkan stuff here

	// Command buffers
	{
		auto primary_render_queue_family_index			= instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_RENDER ).GetQueueFamilyIndex();
		auto secondary_render_queue_family_index		= instance.GetVulkanDevice().GetQueue( VulkanQueueType::SECONDARY_RENDER ).GetQueueFamilyIndex();
		auto primary_transfer_queue_family_index		= instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_TRANSFER ).GetQueueFamilyIndex();

		VkCommandPoolCreateInfo command_pool_create_info {};
		command_pool_create_info.sType		= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext		= nullptr;
		command_pool_create_info.flags		= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		{
			command_pool_create_info.queueFamilyIndex	= primary_render_queue_family_index;
			auto result = vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&primary_render_command_pool
			);
			if( result != VK_SUCCESS ) {
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for primary render queue in thread: "
					<< std::this_thread::get_id();
				instance.Report( result, ss.str() );
				return false;
			}
		}
		{
			command_pool_create_info.queueFamilyIndex	= secondary_render_queue_family_index;
			auto result = vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&secondary_render_command_pool
			);
			if( result != VK_SUCCESS ) {
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for secondary render queue in thread: "
					<< std::this_thread::get_id();
				instance.Report( result, ss.str() );
				return false;
			}
		}
		{
			command_pool_create_info.queueFamilyIndex	= primary_transfer_queue_family_index;
			auto result = vkCreateCommandPool(
				device,
				&command_pool_create_info,
				nullptr,
				&primary_transfer_command_pool
			);
			if( result != VK_SUCCESS ) {
				std::stringstream ss;
				ss << "Internal error: Cannot create Vulkan command pool for primary transfer queue in thread: "
					<< std::this_thread::get_id();
				instance.Report( result, ss.str() );
				return false;
			}
		}
	}

	// Descriptor pool
	{
		descriptor_auto_pool	= CreateDescriptorAutoPool(
			&instance,
			device
		);
		if( !descriptor_auto_pool ) {
			std::stringstream ss;
			ss << "Internal error: Cannot create descriptor auto pool in thread: "
				<< std::this_thread::get_id();
			instance.Report( ReportSeverity::CRITICAL_ERROR, ss.str() );
			return false;
		}
	}

	// Device memory pool
	{
		device_memory_pool = MakeDeviceMemoryPool(
			instance.GetVulkanDevice().GetVulkanPhysicalDevice(),
			device
		);
		if( !device_memory_pool ) {
			std::stringstream ss;
			ss << "Internal error: Cannot create device memory pool in thread: "
				<< std::this_thread::get_id();
			instance.Report( ReportSeverity::CRITICAL_ERROR, ss.str() );
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
			instance.Report( ReportSeverity::CRITICAL_ERROR, ss.str() );
			return false;
		}
	}

	return true;
}

void vk2d::vk2d_internal::ThreadLoaderResource::ThreadEnd()
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
