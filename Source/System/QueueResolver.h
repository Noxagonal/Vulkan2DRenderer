#pragma once

#include "../Core/SourceCommon.h"

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
		VkFence								fence						= VK_NULL_HANDLE );

	VkResult								Submit(
		const std::vector<VkSubmitInfo>		submit_infos,
		VkFence								fence						= VK_NULL_HANDLE );

	VkResult								Present(
		const VkPresentInfoKHR			&	present_info );

	VkQueue									GetQueue() const;
	uint32_t								GetQueueFamilyIndex() const;
	VkBool32								IsPresentationSupported() const;
	const VkQueueFamilyProperties		&	GetQueueFamilyProperties() const;
	std::mutex							*	GetQueueMutex() const;
	uint32_t								GetBasedOn() const;

private:
	VkQueue									queue						= {};	// VkQueue handle.
	uint32_t								queue_family_index			= {};	// Index of the queue family.
	VkBool32								supports_presentation		= {};	// VK_TRUE if you can present using this queue, VK_FALSE if you can not.
	VkQueueFamilyProperties					queue_family_properties		= {};	// Typical VkQueueFamilyProperties.
	std::shared_ptr<std::mutex>				queue_mutex					= {};	// Mutex for queue submissions, only one thread must submit work at a time for single queue.
	uint32_t								based_on					= {};	// Which other queue this one is based off.
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
		uint32_t							based_on;
	};
	std::vector<VkDeviceQueueCreateInfo>	queueCreateInfos;
	std::vector<QueueGetInfo>				queueGetInfo;
	std::vector<std::vector<float>>			queuePriorities;

	VkInstance								refInstance					= VK_NULL_HANDLE;
	VkPhysicalDevice						refPhysicalDevice			= VK_NULL_HANDLE;

	std::vector<VkQueueFamilyProperties>	family_properties;

	bool									is_good						= {};
};



} // _internal

} // vk2d
