#pragma once

#include <core/SourceCommon.h>

#include "ResolvedQueue.hpp"
#include "VulkanQueueTypes.hpp"



namespace vk2d {
namespace vk2d_internal {



class InstanceImpl;
class DeviceMemoryPool;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class VulkanDevice {
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VulkanDevice(
		const VulkanDevice						&	other
	) = delete;
	VulkanDevice(
		VulkanDevice							&&	other
	) = default;

	VulkanDevice(
		InstanceImpl							&	instance,
		VkPhysicalDevice							physical_device
	);
	~VulkanDevice();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VulkanDevice								&	operator=(
		const VulkanDevice						&	other
	) = delete;

	VulkanDevice								&	operator=(
		VulkanDevice							&&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get command buffer queue.
	///
	///				Primary render queue is used to do high throughput rendering, all window and render target textures are rendered
	///				in this queue.
	///
	///				Secondary render queue is used to finalize data already in the GPU, for example generating mip maps for
	///				textures.
	///
	///				Compute queue is used to do compute tasks. Highly repeatable tasks should be done in this queue. For example,
	///				image analysis.
	///
	///				Transfer queue is used to upload data to the GPU, this task may run asynchronously to the other GPU tasks,
	///				allowing more asynchronous operations.
	///
	/// @see		VulkanQueueType
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	queue_type
	///				Queue type we wish to get.
	///
	/// @return		Resolved queue object.
	ResolvedQueue								&	GetQueue(
		VulkanQueueType								queue_type
	);

	VkDevice										GetVulkanDevice();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan physical device handle.
	VkPhysicalDevice								GetVulkanPhysicalDevice() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device properties.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device properties.
	const VkPhysicalDeviceProperties			&	GetVulkanPhysicalDeviceProperties() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device memory properties.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device memory properties.
	const VkPhysicalDeviceMemoryProperties		&	GetVulkanPhysicalDeviceMemoryProperties() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device features.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device features.
	const VkPhysicalDeviceFeatures				&	GetVulkanPhysicalDeviceFeatures() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get device memory pool.
	///
	///				Vulkan objects require memory backing and that memory must be managed manually as only a limited number of
	///				allocations from the GPU are permitted. Alignment requirements and memory types complicate things further.
	///				Device memory pool takes care of these details.
	///
	/// @see		DeviceMemoryPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Pointer to device memory pool.
	DeviceMemoryPool							*	GetDeviceMemoryPool();

	bool											IsGood();

	operator VkDevice();

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											PopulatePhysicalDeviceStructs();

	bool											CreateDeviceMemoryPool();

	void											DestroyDeviceMemoryPool();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	InstanceImpl								&	instance;

	std::vector<const char*>						device_extensions;

	VkPhysicalDevice								vk_physical_device					= {};
	VkDevice										vk_device							= {};

	VkPhysicalDeviceProperties						physical_device_properties			= {};
	VkPhysicalDeviceMemoryProperties				physical_device_memory_properties	= {};
	VkPhysicalDeviceFeatures						physical_device_features			= {};

	std::vector<ResolvedQueue>						resolved_queues;

	std::unique_ptr<DeviceMemoryPool>				device_memory_pool;

	bool											is_good								= {};
};



} // vk2d_internal
} // vk2d