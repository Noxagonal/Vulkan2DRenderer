#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/Renderer.h"

#include "../Header/VulkanMemoryManagement.h"

#include <list>
#include <vector>
#include <array>
#include <memory>



namespace vk2d {

class Window;
class Renderer;

namespace _internal {



class RendererImpl {
	friend class Renderer;

public:
	RendererImpl(
		const RendererCreateInfo				&	renderer_create_info );

	~RendererImpl();

	Window										*	CreateWindowOutput(
		WindowCreateInfo						&	window_create_info );
	void											CloseWindowOutput(
		Window									*	window );

	PFN_VK2D_ReportFunction							GetReportFunction();

	VkInstance										GetVulkanInstance();
	VkPhysicalDevice								GetVulkanPhysicalDevice();
	VkDevice										GetVulkanDevice();

	ResolvedQueue									GetPrimaryRenderQueue();
	ResolvedQueue									GetSecondaryRenderQueue();
	ResolvedQueue									GetPrimaryComputeQueue();
	ResolvedQueue									GetPrimaryTransferQueue();

	const VkPhysicalDeviceProperties			&	GetPhysicalDeviceProperties();
	const VkPhysicalDeviceMemoryProperties		&	GetPhysicalDeviceMemoryProperties();
	const VkPhysicalDeviceFeatures				&	GetPhysicalDeviceFeatures();

	VkShaderModule									GetVertexShaderModule();
	VkShaderModule									GetFragmentShaderModule();

	VkPipelineCache									GetPipelineCache();
	VkPipelineLayout								GetPipelineLayout();
	VkDescriptorSetLayout							GetDescriptorSetLayout();

	DeviceMemoryPool							*	GetDeviceMemoryPool();

private:
	bool											CreateInstance();
	bool											PickPhysicalDevice();
	bool											CreateDeviceAndQueues();
	bool											CreateSampler();
	bool											CreatePipelineCache();
	bool											CreateShaderModules();
	bool											CreateDescriptorSetLayouts();
	bool											CreatePipelineLayout();
	std::vector<VkPhysicalDevice>					EnumeratePhysicalDevices();
	VkPhysicalDevice								PickBestPhysicalDevice();



	static uint64_t									renderer_count;		// used to keep track of Renderer instances

	RendererCreateInfo								create_info_copy					= {};

	std::vector<const char*>						instance_layers						= {};
	std::vector<const char*>						instance_extensions					= {};
	std::vector<const char*>						device_extensions					= {};

	PFN_VK2D_ReportFunction							report_function						= {};

	VkDebugUtilsMessengerEXT						debug_utils_messenger				= {};

	VkInstance										instance							= {};
	VkPhysicalDevice								physical_device						= {};
	VkDevice										device								= {};

	VkPhysicalDeviceProperties						physical_device_properties			= {};
	VkPhysicalDeviceMemoryProperties				physical_device_memory_properties	= {};
	VkPhysicalDeviceFeatures						physical_device_features			= {};

	VkShaderModule									vertex_shader_module				= {};
	VkShaderModule									fragment_shader_module				= {};
	VkPipelineCache									pipeline_cache						= {};
	VkPipelineLayout								pipeline_layout						= {};

	VkSampler										sampler								= {};

	VkDescriptorSetLayout							descriptor_set_layout				= {};

	ResolvedQueue									primary_render_queue				= {};
	ResolvedQueue									secondary_render_queue				= {};
	ResolvedQueue									primary_compute_queue				= {};
	ResolvedQueue									primary_transfer_queue				= {};

	std::unique_ptr<DeviceMemoryPool>				device_memory_pool					= {};

	std::list<std::unique_ptr<Window>>				windows								= {};

	bool											is_good								= {};
};



} // _internal



} // vk2d
