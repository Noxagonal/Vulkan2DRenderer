#pragma once

#include "../Header/SourceCommon.h"

#include <stdint.h>
#include <vector>

namespace vk2d {

namespace _internal {

class DeviceQueueResolver {
public:
	VK2D_API																						DeviceQueueResolver()		= delete;
	VK2D_API																						DeviceQueueResolver(
		VkInstance														instance,
		VkPhysicalDevice												physicalDevice,
		std::vector<std::pair<VkQueueFlags, float>>						queueTypes );

	VK2D_API																						~DeviceQueueResolver();

	const VK2D_API std::vector<VkDeviceQueueCreateInfo>				&	VK2D_APIENTRY				GetDeviceQueueCreateInfos();
	VK2D_API std::vector<vk2d::_internal::ResolvedQueue>				VK2D_APIENTRY				GetQueues( VkDevice device );

	bool																							IsGood();

private:
	struct QueueGetInfo {
		uint32_t							queueIndex;
		uint32_t							queueFamilyIndex;
		uint32_t							basedOn;
	};
	std::vector<VkDeviceQueueCreateInfo>	queueCreateInfos;
	std::vector<QueueGetInfo>				queueGetInfo;
	std::vector<std::vector<float>>			queuePriorities;

	VkInstance								refInstance					= VK_NULL_HANDLE;
	VkPhysicalDevice						refPhysicalDevice			= VK_NULL_HANDLE;

	bool									is_good						= {};
};

} // _internal

} // vk2d
