
#include "Queue.hpp"



VkResult vk2d::vulkan::Queue::Submit(
	const VkSubmitInfo		&	submit_info,
	VkFence						fence
)
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueueSubmit(
		vulkan_queue,
		1,
		&submit_info,
		fence
	);
}

VkResult vk2d::vulkan::Queue::Submit(
	const std::vector<VkSubmitInfo>		submit_infos,
	VkFence								fence )
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueueSubmit(
		vulkan_queue,
		uint32_t( submit_infos.size() ),
		submit_infos.data(),
		fence
	);
}

VkResult vk2d::vulkan::Queue::Present(
	const VkPresentInfoKHR		&	present_info
)
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueuePresentKHR(
		vulkan_queue,
		&present_info
	);
}

VkQueue vk2d::vulkan::Queue::GetVulkanQueue() const
{
	return vulkan_queue;
}

uint32_t vk2d::vulkan::Queue::GetQueueFamilyIndex() const
{
	return queue_family_index;
}

VkBool32 vk2d::vulkan::Queue::IsPresentationSupported() const
{
	return supports_presentation;
}

const VkQueueFamilyProperties & vk2d::vulkan::Queue::GetQueueFamilyProperties() const
{
	return queue_family_properties;
}

std::mutex * vk2d::vulkan::Queue::GetQueueMutex() const
{
	return &*queue_mutex;
}

uint32_t vk2d::vulkan::Queue::GetBasedOn() const
{
	return based_on;
}
