#pragma once

#include "../Header/Core/SourceCommon.h"
#include "../../../Include/Interface/Renderer.h"

#include "../../Header/Core/VulkanMemoryManagement.h"

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



enum class SamplerType : uint32_t {
	DEFAULT,
};



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
	const DescriptorSetLayout					&	GetSamplerDescriptorSetLayout() const;
	const DescriptorSetLayout					&	GetTextureDescriptorSetLayout() const;

	VkDescriptorSet									GetDefaultTextureDescriptorSet() const;
	VkDescriptorSet									GetSamplerDescriptorSet(
		SamplerType									sampler_type ) const;
	VkSampler										GetSampler(
		SamplerType									sampler_type ) const;

	DeviceMemoryPool							*	GetDeviceMemoryPool() const;

	bool											IsGood() const;

private:
	bool											CreateInstance();
	bool											PickPhysicalDevice();
	bool											CreateDeviceAndQueues();
	bool											CreateSamplers();
	bool											CreatePipelineCache();
	bool											CreateShaderModules();
	bool											CreateDescriptorSetLayouts();
	bool											CreatePipelineLayout();
	bool											CreateDeviceMemoryPool();
	bool											CreateThreadPool();
	bool											CreateResourceManager();
	bool											CreateDefaultTexture();

	void											DestroyInstance();
	void											DestroyDevice();
	void											DestroySamplers();
	void											DestroyPipelineCache();
	void											DestroyShaderModules();
	void											DestroyDescriptorSetLayouts();
	void											DestroyPipelineLayout();
	void											DestroyDeviceMemoryPool();
	void											DestroyThreadPool();
	void											DestroyResourceManager();
	void											DestroyDefaultTexture();

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

	VkDescriptorPool								sampler_descriptor_pool				= {};
	struct Sampler {
		VkSampler									sampler								= {};
		VkDescriptorSet								descriptor_set				= {};
	};
	std::array<Sampler, 1>							samplers							= {};

	std::unique_ptr<DescriptorSetLayout>			sampler_descriptor_set_layout		= {};
	std::unique_ptr<DescriptorSetLayout>			texture_descriptor_set_layout		= {};

	ResolvedQueue									primary_render_queue				= {};
	ResolvedQueue									secondary_render_queue				= {};
	ResolvedQueue									primary_compute_queue				= {};
	ResolvedQueue									primary_transfer_queue				= {};

	std::unique_ptr<DeviceMemoryPool>				device_memory_pool					= {};

	vk2d::_internal::CompleteImageResource			default_texture						= {};
	VkDescriptorPool								default_texture_descriptor_pool		= {};
	VkDescriptorSet									default_texture_descriptor_set		= {};

	std::list<std::unique_ptr<Window>>				windows								= {};

	bool											is_good								= {};
};



} // _internal



} // vk2d
