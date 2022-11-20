#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vk2d_internal {



class DeviceQueueResolver;



class ResolvedQueue {
	friend class DeviceQueueResolver;

public:
	ResolvedQueue() = default;
	~ResolvedQueue() = default;

	VkResult								Submit(
		const VkSubmitInfo				&	submit_info,
		VkFence								fence = VK_NULL_HANDLE );

	VkResult								Submit(
		const std::vector<VkSubmitInfo>		submit_infos,
		VkFence								fence = VK_NULL_HANDLE );

	VkResult								Present(
		const VkPresentInfoKHR			&	present_info );

	VkQueue									GetQueue() const;
	uint32_t								GetQueueFamilyIndex() const;
	VkBool32								IsPresentationSupported() const;
	const VkQueueFamilyProperties		&	GetQueueFamilyProperties() const;
	std::mutex							*	GetQueueMutex() const;
	uint32_t								GetBasedOn() const;

private:
	VkQueue									queue = {};	// VkQueue handle.
	uint32_t								queue_family_index = {};	// Index of the queue family.
	VkBool32								supports_presentation = {};	// VK_TRUE if you can present using this queue, VK_FALSE if you can not.
	VkQueueFamilyProperties					queue_family_properties = {};	// Typical VkQueueFamilyProperties.
	std::shared_ptr<std::mutex>				queue_mutex = {};	// Mutex for queue submissions, only one thread must submit work at a time for single queue.
	uint32_t								based_on = {};	// Which other queue this one is based off.
};



} // vk2d_internal
} // vk2d
