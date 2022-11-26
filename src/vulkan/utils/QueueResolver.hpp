#pragma once

#include <core/SourceCommon.hpp>

#include <vulkan/Queue.hpp>



namespace vk2d {
namespace vulkan {



class DeviceQueueResolver {
public:
	DeviceQueueResolver()		= delete;
	DeviceQueueResolver(
		VkInstance											instance,
		VkPhysicalDevice									physicalDevice,
		std::vector<std::pair<VkQueueFlags, float>>			queueTypes );

	~DeviceQueueResolver();

	const std::vector<VkDeviceQueueCreateInfo>			&	GetDeviceQueueCreateInfos();
	std::vector<Queue>								GetQueues(
		VkDevice device
	) const;

	bool													IsGood();

private:
	struct QueueGetInfo {
		uint32_t											queueIndex;
		uint32_t											queueFamilyIndex;
		uint32_t											based_on;
	};
	std::vector<VkDeviceQueueCreateInfo>					queueCreateInfos;
	std::vector<QueueGetInfo>								queueGetInfo;
	std::vector<std::vector<float>>							queuePriorities;

	VkInstance												refInstance					= VK_NULL_HANDLE;
	VkPhysicalDevice										refPhysicalDevice			= VK_NULL_HANDLE;

	std::vector<VkQueueFamilyProperties>					family_properties;

	bool													is_good						= {};
};



} // vulkan
} // vk2d
