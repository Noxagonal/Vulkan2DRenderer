#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vk2d_internal {
class InstanceImpl;
} // vk2d_internal



namespace vulkan {



class VulkanInstance {
public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VulkanInstance(
		const VulkanInstance					&	other
	) = delete;
	VulkanInstance(
		VulkanInstance							&&	other
	) = default;

	VulkanInstance(
		vk2d_internal::InstanceImpl				&	instance
	);
	~VulkanInstance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VulkanInstance								&	operator=(
		const VulkanInstance					&	other
	) = delete;

	VulkanInstance								&	operator=(
		VulkanInstance							&&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkInstance										GetVulkanInstance();

	std::vector<VkPhysicalDevice>					EnumeratePhysicalDevices();
	VkPhysicalDevice								PickBestVulkanPhysicalDevice();

	bool											IsGood();

	operator VkInstance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl					&	instance;

	std::vector<const char*>						instance_layers;
	std::vector<const char*>						instance_extensions;

	VkInstance										vk_instance								= {};

	VkDebugUtilsMessengerEXT						vk_debug_utils_messenger				= {};

	bool											is_good									= {};
};



} // vulkan
} // vk2d
