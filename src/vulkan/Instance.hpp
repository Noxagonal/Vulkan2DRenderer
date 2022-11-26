#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vk2d_internal {
class InstanceImpl;
} // vk2d_internal



namespace vulkan {



class Instance {
public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Instance(
		const Instance							&	other
	) = delete;
	Instance(
		Instance								&&	other
	) = default;

	Instance(
		vk2d_internal::InstanceImpl				&	instance
	);
	~Instance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Instance									&	operator=(
		const Instance							&	other
	) = delete;

	Instance									&	operator=(
		Instance								&&	other
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

	VkInstance										vulkan_instance							= {};

	VkDebugUtilsMessengerEXT						vk_debug_utils_messenger				= {};

	bool											is_good									= {};
};



} // vulkan
} // vk2d
