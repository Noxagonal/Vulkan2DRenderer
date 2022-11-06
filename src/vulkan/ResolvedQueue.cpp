
#include "ResolvedQueue.hpp"



VkResult vk2d::vk2d_internal::ResolvedQueue::Submit(
	const VkSubmitInfo		&	submit_info,
	VkFence						fence
)
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueueSubmit(
		queue,
		1,
		&submit_info,
		fence
	);
}

VkResult vk2d::vk2d_internal::ResolvedQueue::Submit(
	const std::vector<VkSubmitInfo>		submit_infos,
	VkFence								fence )
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueueSubmit(
		queue,
		uint32_t( submit_infos.size() ),
		submit_infos.data(),
		fence
	);
}

VkResult vk2d::vk2d_internal::ResolvedQueue::Present(
	const VkPresentInfoKHR		&	present_info
)
{
	std::lock_guard<std::mutex> lock_guard( *queue_mutex );

	return vkQueuePresentKHR(
		queue,
		&present_info
	);
}

VkQueue vk2d::vk2d_internal::ResolvedQueue::GetQueue() const
{
	return queue;
}

uint32_t vk2d::vk2d_internal::ResolvedQueue::GetQueueFamilyIndex() const
{
	return queue_family_index;
}

VkBool32 vk2d::vk2d_internal::ResolvedQueue::IsPresentationSupported() const
{
	return supports_presentation;
}

const VkQueueFamilyProperties & vk2d::vk2d_internal::ResolvedQueue::GetQueueFamilyProperties() const
{
	return queue_family_properties;
}

std::mutex * vk2d::vk2d_internal::ResolvedQueue::GetQueueMutex() const
{
	return &*queue_mutex;
}

uint32_t vk2d::vk2d_internal::ResolvedQueue::GetBasedOn() const
{
	return based_on;
}
