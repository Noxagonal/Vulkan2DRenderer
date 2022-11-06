
#include "VulkanDevice.hpp"

#include "utils/QueueResolver.hpp"
#include <interface/InstanceImpl.h>

#include "utils/VulkanMemoryManagement.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<std::pair<VkQueueFlags, float>> queue_requests {
	{ VK_QUEUE_GRAPHICS_BIT, 1.0f },
	{ VK_QUEUE_GRAPHICS_BIT, 0.2f },
	{ VK_QUEUE_COMPUTE_BIT, 0.9f },
	{ VK_QUEUE_TRANSFER_BIT, 0.5f }
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanDevice::VulkanDevice(
	InstanceImpl		&	instance,
	VkPhysicalDevice		physical_device
) :
	instance( instance )
{
	// Device extensions.
	device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	device_extensions.push_back( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );

	#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	device_extensions.push_back( VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME );
	#endif

	const VkInstance vk_instance	= instance.GetVulkanInstance();
	vk_physical_device				= physical_device;

	auto queue_resolver = DeviceQueueResolver(
		vk_instance,
		physical_device,
		queue_requests
	);
	if( !queue_resolver.IsGood() ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create queue resolver!" );
		return;
	}
	auto queue_create_infos = queue_resolver.GetDeviceQueueCreateInfos();

	VkPhysicalDeviceFeatures features {};
	features.samplerAnisotropy						= VK_TRUE;
	features.fillModeNonSolid						= VK_TRUE;
	features.wideLines								= VK_TRUE;
	features.geometryShader							= VK_TRUE;
//	features.shaderStorageImageWriteWithoutFormat	= VK_TRUE;
//	features.fragmentStoresAndAtomics				= VK_TRUE;

	VkPhysicalDeviceVulkan12Features features_1_2 {};
	features_1_2.sType								= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features_1_2.pNext								= nullptr;
	features_1_2.samplerMirrorClampToEdge			= VK_TRUE;
	features_1_2.timelineSemaphore					= VK_TRUE;

	VkDeviceCreateInfo device_create_info {};
	device_create_info.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext						= &features_1_2;
	device_create_info.flags						= 0;
	device_create_info.queueCreateInfoCount			= uint32_t( queue_create_infos.size() );
	device_create_info.pQueueCreateInfos			= queue_create_infos.data();
	device_create_info.enabledLayerCount			= 0;
	device_create_info.ppEnabledLayerNames			= nullptr;
	device_create_info.enabledExtensionCount		= uint32_t( device_extensions.size() );
	device_create_info.ppEnabledExtensionNames		= device_extensions.data();
	device_create_info.pEnabledFeatures				= &features;

	auto result = vkCreateDevice(
		vk_physical_device,
		&device_create_info,
		nullptr,
		&vk_device
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan device!" );
		return;
	}

	resolved_queues = queue_resolver.GetQueues( vk_device );
	if( resolved_queues.size() != queue_requests.size() ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot get correct amount of queues from queue resolver!" );
		return;
	}

	PopulatePhysicalDeviceStructs();
	if( !CreateDeviceMemoryPool() ) return;

	is_good = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanDevice::~VulkanDevice()
{
	DestroyDeviceMemoryPool();

	vkDestroyDevice(
		vk_device,
		nullptr
	);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::ResolvedQueue & vk2d::vk2d_internal::VulkanDevice::GetQueue(
	VulkanQueueType queue_type
)
{
	return resolved_queues[ std::underlying_type_t<VulkanQueueType>( queue_type ) ];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkDevice vk2d::vk2d_internal::VulkanDevice::GetVulkanDevice()
{
	return vk_device;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPhysicalDevice vk2d::vk2d_internal::VulkanDevice::GetVulkanPhysicalDevice() const
{
	return vk_physical_device;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::DeviceMemoryPool * vk2d::vk2d_internal::VulkanDevice::GetDeviceMemoryPool()
{
	return device_memory_pool.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::VulkanDevice::IsGood()
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanDevice::operator VkDevice()
{
	return GetVulkanDevice();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::VulkanDevice::PopulatePhysicalDeviceStructs()
{
	vkGetPhysicalDeviceProperties(
		vk_physical_device,
		&physical_device_properties
	);
	vkGetPhysicalDeviceMemoryProperties(
		vk_physical_device,
		&physical_device_memory_properties
	);
	vkGetPhysicalDeviceFeatures(
		vk_physical_device,
		&physical_device_features
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::VulkanDevice::CreateDeviceMemoryPool()
{
	device_memory_pool = MakeDeviceMemoryPool(
		vk_physical_device,
		vk_device
	);
	if( !device_memory_pool ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create memory pool!" );
		return false;
	}
	return true;
}

void vk2d::vk2d_internal::VulkanDevice::DestroyDeviceMemoryPool()
{
	device_memory_pool = nullptr;
}
