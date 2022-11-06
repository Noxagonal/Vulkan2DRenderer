#pragma once

#include <core/SourceCommon.h>



namespace vk2d {
namespace vk2d_internal {



class InstanceImpl;



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
		InstanceImpl							&	instance
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
	InstanceImpl								&	instance;

	std::vector<const char*>						instance_layers;
	std::vector<const char*>						instance_extensions;

	VkInstance										vk_instance								= {};

	VkDebugUtilsMessengerEXT						vk_debug_utils_messenger				= {};

	bool											is_good									= {};
};



} // vk2d_internal
} // vk2d
