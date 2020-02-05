#pragma once

#include "../Core/SourceCommon.h"
#include "../../../Include/Interface/Instance.h"

#include "../../Header/Core/VulkanMemoryManagement.h"
#include "../Core/QueueResolver.h"

#include "../Core/DescriptorSet.h"
#include "../Core/ShaderInterface.h"

#include "WindowImpl.h"

#include <list>
#include <vector>
#include <array>
#include <memory>
#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>




namespace vk2d {

class Window;
class Monitor;
class Cursor;
class Instance;
class ResourceManager;
class TextureResource;

namespace _internal {

class ThreadPool;
class DescriptorSetLayout;
class WindowImpl;
class MonitorImpl;
class CursorImpl;





class InstanceImpl {
	friend class vk2d::Instance;

public:
	InstanceImpl(
		const vk2d::InstanceCreateInfo					&	instance_create_info );

	~InstanceImpl();

	std::vector<vk2d::Monitor*>								GetMonitors();

	vk2d::Monitor										*	GetPrimaryMonitor();

	void													SetMonitorUpdateCallback(
		vk2d::MonitorUpdateCallbackFun						monitor_update_callback_funtion );

	vk2d::Cursor										*	CreateCursor(
		const std::filesystem::path						&	image_path,
		vk2d::Vector2i										hot_spot );

	vk2d::Cursor										*	CreateCursor(
		vk2d::Vector2u										image_size,
		const std::vector<vk2d::Color8>					&	image_data,
		vk2d::Vector2i										hot_spot );

	void													DestroyCursor(
		vk2d::Cursor									*	cursor );

	vk2d::GamepadEventCallbackFun							GetGamepadEventCallback();

	void													SetGamepadEventCallback(
		vk2d::GamepadEventCallbackFun						gamepad_event_callback_function );

	bool													IsGamepadPresent(
		vk2d::Gamepad										gamepad );

	std::string												GetGamepadName(
		vk2d::Gamepad										gamepad );

	vk2d::GamepadState										QueryGamepadState(
		vk2d::Gamepad										gamepad );

	// TODO: gamepad mapping
	void													SetGamepadMapping();

	vk2d::Window										*	CreateOutputWindow(
		vk2d::WindowCreateInfo							&	window_create_info );
	void													CloseOutputWindow(
		vk2d::Window									*	window );

	vk2d::_internal::DescriptorAutoPool					*	GetDescriptorPool();

	vk2d::Sampler										*	CreateSampler(
		const vk2d::SamplerCreateInfo					&	sampler_create_info );

	void													DestroySampler(
		vk2d::Sampler									*	sampler );

	vk2d::Multisamples										GetMaximumSupportedMultisampling();
	vk2d::Multisamples										GetAllSupportedMultisampling();

	vk2d::PFN_VK2D_ReportFunction							GetReportFunction() const;

	void													Report(
		vk2d::ReportSeverity								severity,
		const std::string								&	message );

	vk2d::_internal::ThreadPool							*	GetThreadPool() const;
	const std::vector<uint32_t>							&	GetLoaderThreads() const;
	const std::vector<uint32_t>							&	GetGeneralThreads() const;
	vk2d::ResourceManager								*	GetResourceManager() const;

	VkInstance												GetVulkanInstance() const;
	VkPhysicalDevice										GetVulkanPhysicalDevice() const;
	VkDevice												GetVulkanDevice() const;

	vk2d::_internal::ResolvedQueue							GetPrimaryRenderQueue() const;
	vk2d::_internal::ResolvedQueue							GetSecondaryRenderQueue() const;
	vk2d::_internal::ResolvedQueue							GetPrimaryComputeQueue() const;
	vk2d::_internal::ResolvedQueue							GetPrimaryTransferQueue() const;

	const VkPhysicalDeviceProperties					&	GetVulkanPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties				&	GetVulkanPhysicalDeviceMemoryProperties() const;
	const VkPhysicalDeviceFeatures						&	GetVulkanPhysicalDeviceFeatures() const;

	vk2d::_internal::ShaderProgram							GetShaderModules(
		vk2d::_internal::ShaderProgramID						id ) const;

	vk2d::_internal::ShaderProgram							GetCompatibleShaderModules(
		bool												multitextured,
		bool												custom_uv_border_color,
		uint32_t											vertices_per_primitive );

	VkPipeline												GetVulkanPipeline(
		const vk2d::_internal::PipelineSettings			&	settings );

	VkPipeline												CreateVulkanPipeline(
		const vk2d::_internal::PipelineSettings			&	settings );

	VkPipelineCache											GetVulkanPipelineCache() const;
	VkPipelineLayout										GetVulkanPipelineLayout() const;

	const vk2d::_internal::DescriptorSetLayout			&	GetSamplerTextureDescriptorSetLayout() const;
	const vk2d::_internal::DescriptorSetLayout			&	GetUniformBufferDescriptorSetLayout() const;
	const vk2d::_internal::DescriptorSetLayout			&	GetStorageBufferDescriptorSetLayout() const;

//	VkDescriptorSet											GetDefaultTextureDescriptorSet() const;
	vk2d::TextureResource								*	GetDefaultTexture() const;
	vk2d::Sampler										*	GetDefaultSampler() const;

	vk2d::_internal::DeviceMemoryPool					*	GetDeviceMemoryPool() const;

	bool													IsGood() const;


	bool													CreateInstance();
	bool													PickPhysicalDevice();
	bool													CreateDeviceAndQueues();
	bool													CreateDescriptorPool();
	bool													CreateDefaultSampler();
	bool													CreatePipelineCache();
	bool													CreateShaderModules();
	bool													CreateDescriptorSetLayouts();
	bool													CreatePipelineLayout();
	bool													CreateDeviceMemoryPool();
	bool													CreateThreadPool();
	bool													CreateResourceManager();
	bool													CreateDefaultTexture();

	void													DestroyInstance();
	void													DestroyDevice();
	void													DestroyDescriptorPool();
	void													DestroyDefaultSampler();
	void													DestroyPipelineCache();
	void													DestroyPipelines();
	void													DestroyShaderModules();
	void													DestroyDescriptorSetLayouts();
	void													DestroyPipelineLayout();
	void													DestroyDeviceMemoryPool();
	void													DestroyThreadPool();
	void													DestroyResourceManager();
	void													DestroyDefaultTexture();

	std::vector<VkPhysicalDevice>							EnumeratePhysicalDevices();
	VkPhysicalDevice										PickBestVulkanPhysicalDevice();

	vk2d::MonitorUpdateCallbackFun							monitor_update_callback					= nullptr;

private:
	static uint64_t											instance_count;		// used to keep track of Instance instances

	vk2d::InstanceCreateInfo								create_info_copy						= {};

	std::vector<const char*>								instance_layers							= {};
	std::vector<const char*>								instance_extensions						= {};
	std::vector<const char*>								device_extensions						= {};

	vk2d::PFN_VK2D_ReportFunction							report_function							= {};
	std::mutex												report_mutex;

	std::unique_ptr<vk2d::ResourceManager>					resource_manager						= {};
	std::unique_ptr<vk2d::_internal::ThreadPool>			thread_pool								= {};
	std::vector<uint32_t>									loader_threads							= {};
	std::vector<uint32_t>									general_threads							= {};

	VkDebugUtilsMessengerEXT								vk_debug_utils_messenger				= {};

	VkInstance												vk_instance								= {};
	VkPhysicalDevice										vk_physical_device						= {};
	VkDevice												vk_device								= {};

	VkPhysicalDeviceProperties								vk_physical_device_properties			= {};
	VkPhysicalDeviceMemoryProperties						vk_physical_device_memory_properties	= {};
	VkPhysicalDeviceFeatures								vk_physical_device_features				= {};

	std::vector<VkShaderModule>								vk_shader_modules						= {};

	std::map<vk2d::_internal::ShaderProgramID, vk2d::_internal::ShaderProgram>	shader_programs		= {};
	std::map<vk2d::_internal::PipelineSettings, VkPipeline>						vk_pipelines		= {};

	VkPipelineCache											vk_pipeline_cache						= {};
	VkPipelineLayout										vk_pipeline_layout						= {};

	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	sampler_texture_descriptor_set_layout	= {};
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	uniform_buffer_descriptor_set_layout	= {};
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	storage_buffer_descriptor_set_layout	= {};

	vk2d::_internal::ResolvedQueue							primary_render_queue					= {};
	vk2d::_internal::ResolvedQueue							secondary_render_queue					= {};
	vk2d::_internal::ResolvedQueue							primary_compute_queue					= {};
	vk2d::_internal::ResolvedQueue							primary_transfer_queue					= {};

	std::unique_ptr<vk2d::_internal::DeviceMemoryPool>		device_memory_pool						= {};

	std::unique_ptr<vk2d::_internal::DescriptorAutoPool>	descriptor_pool							= {};

	std::unique_ptr<vk2d::Sampler>							default_sampler							= {};

//	vk2d::_internal::CompleteImageResource					default_texture							= {};
//	vk2d::_internal::PoolDescriptorSet						default_texture_descriptor_set			= {};
	vk2d::TextureResource								*	default_texture							= {};

	std::vector<std::unique_ptr<vk2d::Window>>				windows									= {};
	std::vector<std::unique_ptr<vk2d::Cursor>>				cursors									= {};
	std::vector<std::unique_ptr<vk2d::Sampler>>				samplers								= {};

	vk2d::GamepadEventCallbackFun							joystick_event_callback					= {};

	bool													is_good									= {};
};



} // _internal



} // vk2d
