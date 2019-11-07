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
class ResourceManager;

namespace _internal {

class ThreadPool;
class DescriptorSetLayout;



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

	PFN_VK2D_ReportFunction							GetReportFunction() const;

	ThreadPool									*	GetThreadPool() const;
	const std::vector<uint32_t>					&	GetLoaderThreads() const;
	const std::vector<uint32_t>					&	GetGeneralThreads() const;
	ResourceManager								*	GetResourceManager() const;

	VkInstance										GetVulkanInstance() const;
	VkPhysicalDevice								GetVulkanPhysicalDevice() const;
	VkDevice										GetVulkanDevice() const;

	ResolvedQueue									GetPrimaryRenderQueue() const;
	ResolvedQueue									GetSecondaryRenderQueue() const;
	ResolvedQueue									GetPrimaryComputeQueue() const;
	ResolvedQueue									GetPrimaryTransferQueue() const;

	const VkPhysicalDeviceProperties			&	GetPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties		&	GetPhysicalDeviceMemoryProperties() const;
	const VkPhysicalDeviceFeatures				&	GetPhysicalDeviceFeatures() const;

	VkShaderModule									GetVertexShaderModule() const;
	VkShaderModule									GetFragmentShaderModule() const;

	VkPipelineCache									GetPipelineCache() const;
	VkPipelineLayout								GetPipelineLayout() const;
	const DescriptorSetLayout					&	GetDescriptorSetLayout() const;

	DeviceMemoryPool							*	GetDeviceMemoryPool() const;

	bool											IsGood() const;

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

	std::unique_ptr<ResourceManager>				resource_manager					= {};
	std::unique_ptr<ThreadPool>						thread_pool							= {};
	std::vector<uint32_t>							loader_threads						= {};
	std::vector<uint32_t>							general_threads						= {};

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

	std::unique_ptr<DescriptorSetLayout>			descriptor_set_layout				= {};

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
