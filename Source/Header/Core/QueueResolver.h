#pragma once

#include "SourceCommon.h"

namespace vk2d {

namespace _internal {



class DeviceQueueResolver;



class ResolvedQueue {
	friend class vk2d::_internal::DeviceQueueResolver;

public:
	ResolvedQueue()							= default;
	~ResolvedQueue()						= default;

	VkResult								Submit(
		const VkSubmitInfo				&	submit_info,
		VkFence								fence );

	VkResult								Submit(
		const std::vector<VkSubmitInfo>		submit_infos,
		VkFence								fence );

	VkResult								Present(
		const VkPresentInfoKHR			&	present_info );

	VkQueue									GetQueue();
	uint32_t								GetQueueFamilyIndex();
	VkBool32								IsPresentationSupported();
	std::mutex							*	GetQueueMutex();
	uint32_t								GetBasedOn();

private:
	VkQueue									queue						= {};	// VkQueue handle.
	uint32_t								queueFamilyIndex			= {};	// Typical queueFamilyIndex.
	VkBool32								supportsPresentation		= {};	// VK_TRUE if you can present using this queue, VK_FALSE if you can not.
	std::shared_ptr<std::mutex>				queueMutex					= {};	// Mutex for queue submissions, only one thread must submit work at a time for single queue.
	uint32_t								basedOn						= {};	// Which other queue this one is based off.
};



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
