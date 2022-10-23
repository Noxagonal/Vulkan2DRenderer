#pragma once

#include "core/SourceCommon.h"
#include "interface/Instance.h"

#include "types/Color.hpp"

#include "system/QueueResolver.h"
#include "system/DescriptorSet.h"
#include "system/ShaderInterface.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace vk2d {

class Window;
class Monitor;
class Cursor;
class ResourceManager;
class TextureResource;
class Sampler;
class RenderTargetTexture;

namespace _internal {

class ThreadPool;
class DescriptorSetLayout;
class WindowImpl;
class DeviceMemoryPool;
class MonitorImpl;

void UpdateMonitorLists( bool globals_locked );



class InstanceImpl {
	friend class vk2d::Instance;
	friend void vk2d::_internal::UpdateMonitorLists( bool globals_locked );

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::Instance()
	InstanceImpl(
		vk2d::Instance									*	my_interface,
		const vk2d::InstanceCreateInfo					&	instance_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::~Instance()
	~InstanceImpl();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::Run()
	bool													Run();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::GetMonitors()
	std::vector<vk2d::Monitor*>								GetMonitors();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::GetPrimaryMonitor()
	vk2d::Monitor										*	GetPrimaryMonitor() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::SetMonitorUpdateCallback()
	void													SetMonitorUpdateCallback(
		vk2d::PFN_MonitorUpdateCallback						monitor_update_callback_funtion );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::CreateCursor()
	vk2d::Cursor										*	CreateCursor(
		const std::filesystem::path						&	image_path,
		glm::ivec2											hot_spot );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::CreateCursor()
	vk2d::Cursor										*	CreateCursor(
		glm::uvec2											image_size,
		const std::vector<vk2d::Color8>					&	image_data,
		glm::ivec2											hot_spot );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::DestroyCursor()
	void													DestroyCursor(
		vk2d::Cursor									*	cursor );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::GetGamepadEventCallback()
	vk2d::PFN_GamepadConnectionEventCallback				GetGamepadEventCallback() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::SetGamepadEventCallback()
	void													SetGamepadEventCallback(
		vk2d::PFN_GamepadConnectionEventCallback			gamepad_event_callback_function );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::IsGamepadPresent()
	bool													IsGamepadPresent(
		vk2d::Gamepad										gamepad );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::GetGamepadName()
	std::string												GetGamepadName(
		vk2d::Gamepad										gamepad );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::QueryGamepadState()
	vk2d::GamepadState										QueryGamepadState(
		vk2d::Gamepad										gamepad );

	// TODO: gamepad mapping

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::SetGamepadMapping()
	void													SetGamepadMapping();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::CreateOutputWindow()
	vk2d::Window										*	CreateOutputWindow(
		const vk2d::WindowCreateInfo					&	window_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::DestroyOutputWindow()
	void													DestroyOutputWindow(
		vk2d::Window									*	window );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::CreateRenderTargetTexture()
	vk2d::RenderTargetTexture							*	CreateRenderTargetTexture(
		const vk2d::RenderTargetTextureCreateInfo		&	render_target_texture_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see vk2d::Instance::DestroyRenderTargetTexture()
	void													DestroyRenderTargetTexture(
		vk2d::RenderTargetTexture						*	render_target_texture );

	// vk2d::_internal::DescriptorAutoPool					*	GetDescriptorPool();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Allocate descriptor set directly from instance.
	///
	///				Prefer to use per thread descriptor auto pool if possible. Instance field DescriptorAutoPool is not directly
	///				exposed as DescriptorAutoPool is single thread only.
	///
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	for_descriptor_set_layout
	///				tells what type of a descriptor set we should allocate.
	/// 
	/// @return		PoolDescriptorSet
	vk2d::_internal::PoolDescriptorSet						AllocateDescriptorSet(
		const vk2d::_internal::DescriptorSetLayout		&	for_descriptor_set_layout );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Free descriptor set that was directly allocated from instance.
	/// 
	///				Prefer to use per thread descriptor auto pool if possible. Instance field DescriptorAutoPool is not directly
	///				exposed as DescriptorAutoPool is single thread only.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	descriptor_set
	///				DescriptorSet that was previously allocated from the same instance.
	void													FreeDescriptorSet(
		vk2d::_internal::PoolDescriptorSet				&	descriptor_set );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		vk2d::Instance::CreateSampler()
	vk2d::Sampler										*	CreateSampler(
		const vk2d::SamplerCreateInfo					&	sampler_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		vk2d::Instance::DestroySampler()
	void													DestroySampler(
		vk2d::Sampler									*	sampler );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		vk2d::Instance::GetMaximumSupportedMultisampling()
	vk2d::Multisamples										GetMaximumSupportedMultisampling() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		vk2d::Instance::GetAllSupportedMultisampling()
	vk2d::Multisamples										GetAllSupportedMultisampling() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get report function so that parts of VK2D may log messages.
	vk2d::PFN_VK2D_ReportFunction							GetReportFunction() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Log a message.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param		vk_result
	///				Vulkan result.
	///
	/// @param		severity
	///				Severity of the message.
	///
	/// @param		message
	///				Message.
	void													Report(
		VkResult											vk_result,
		vk2d::ReportSeverity								severity,
		const std::string								&	message );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Log a message.
	///
	/// @note		Multithreading: Any thread.
	/// 
	/// @param		vk_result
	///				Vulkan result.
	///
	/// @param		message
	///				Message.
	void													Report(
		VkResult											vk_result,
		const std::string								&	message );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Log a message.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param		severity
	///				Severity of the message.
	///
	/// @param		message
	///				Message.
	void													Report(
		vk2d::ReportSeverity								severity,
		const std::string								&	message );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get internal thread pool.
	///
	/// @see		vk2d::_internal::ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Pointer to thread pool.
	vk2d::_internal::ThreadPool							*	GetThreadPool() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get loader threads.
	///
	///				Loader threads are threads that are allowed to access the system's hard drive. These are generally used to copy
	///				file contents into RAM.
	///
	/// @see		vk2d::_internal::ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		List of loader threads indices.
	const std::vector<uint32_t>							&	GetLoaderThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get general threads.
	///
	///				General threads are meant to do heavy lifting like translating data and decoding packed image files like png
	///				into a list of pixels.
	/// 
	/// @see		vk2d::_internal::ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		List of loader threads indices.
	const std::vector<uint32_t>							&	GetGeneralThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		vk2d::Instance::GetResourceManager()
	vk2d::ResourceManager								*	GetResourceManager() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan instance.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan instance handle.
	VkInstance												GetVulkanInstance() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan physical device handle.
	VkPhysicalDevice										GetVulkanPhysicalDevice() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan device.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan device handle.
	VkDevice												GetVulkanDevice() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get primary render queue.
	///
	///				Primary render queue is used to do high throughput rendering, all window and render target textures are rendered
	///				in this queue.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Resolved queue object.
	vk2d::_internal::ResolvedQueue							GetPrimaryRenderQueue() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get secondary render queue.
	///
	///				Secondary render queue is used to finalize data already in the GPU, for example generating mip maps for
	///				textures.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Resolved queue object.
	vk2d::_internal::ResolvedQueue							GetSecondaryRenderQueue() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute queue.
	///
	///				Compute queue is used to do compute tasks. Highly repeatable tasks should be done in this queue. For example,
	///				image analysis.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Resolved queue object.
	vk2d::_internal::ResolvedQueue							GetPrimaryComputeQueue() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get transfer queue.
	///
	///				Transfer queue is used to upload data to the GPU, this task may run asynchronously to the other GPU tasks,
	///				allowing more asynchronous operations.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Resolved queue object.
	vk2d::_internal::ResolvedQueue							GetPrimaryTransferQueue() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device properties.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device properties.
	const VkPhysicalDeviceProperties					&	GetVulkanPhysicalDeviceProperties() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device memory properties.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device memory properties.
	const VkPhysicalDeviceMemoryProperties				&	GetVulkanPhysicalDeviceMemoryProperties() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan physical device features.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Reference to internal physical device features.
	const VkPhysicalDeviceFeatures						&	GetVulkanPhysicalDeviceFeatures() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics shader modules.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	id
	///				Graphics shader program ID. See vk2d::_internal::GraphicsShaderProgramID for more info.
	///
	/// @return		Graphics shader program.
	vk2d::_internal::GraphicsShaderProgram					GetGraphicsShaderModules(
		vk2d::_internal::GraphicsShaderProgramID			id ) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute shader modules.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	id
	///				Compute shader program ID. See vk2d::_internal::ComputeShaderProgramID for more info.
	///
	/// @return		Compute shader program.
	VkShaderModule											GetComputeShaderModules(
		vk2d::_internal::ComputeShaderProgramID				id ) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compatible graphics shader modules.
	///
	///				This is used to find an existing shader module which is compatible with these properties.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	multitextured
	///				Tells if the graphics shader need to support multitextured meshes.
	/// 
	/// @param[in]	custom_uv_border_color
	///				Tells if the graphics shader need to support custom uv border color.
	/// 
	/// @param[in]	vertices_per_primitive
	///				Tells how many vertices per primitive the shader needs to support, must be a value between 1 and 3 (inclusive).
	///
	/// @return		Graphics shader program.
	vk2d::_internal::GraphicsShaderProgram					GetCompatibleGraphicsShaderModules(
		bool												multitextured,
		bool												custom_uv_border_color,
		uint32_t											vertices_per_primitive ) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics pipeline.
	///
	///				Tries to find a graphics pipeline matching these pipeline settings. If no existing pipeline is found, a new one
	///				is created and added to the list of existing pipelines before being returned.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	settings
	///				Pipeline settings we wish to have.
	///
	/// @return		Graphics shader pipeline.
	VkPipeline												GetGraphicsPipeline(
		const vk2d::_internal::GraphicsPipelineSettings	&	settings );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute pipeline.
	///
	///				Tries to find a compute pipeline matching these pipeline settings. If no existing pipeline is found, a new one
	///				is created and added to the list of existing pipelines before being returned.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	settings
	///				Pipeline settings we wish to have.
	///
	/// @return		Graphics shader pipeline.
	VkPipeline												GetComputePipeline(
		const vk2d::_internal::ComputePipelineSettings	&	settings );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create graphics pipeline.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	settings
	///				Pipeline settings need the new pipeline to have.
	///
	/// @return		New graphics shader pipeline.
	VkPipeline												CreateGraphicsPipeline(
		const vk2d::_internal::GraphicsPipelineSettings	&	settings );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create compute pipeline.
	///
	///				Tries to find a compute pipeline matching these pipeline settings. If no existing pipeline is found, a new one
	///				is created and added to the list of existing pipelines before being returned.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	settings
	///				Pipeline settings need the new pipeline to have.
	///
	/// @return		New compute shader pipeline.
	VkPipeline												CreateComputePipeline(
		const vk2d::_internal::ComputePipelineSettings	&	settings );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache											GetGraphicsPipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache											GetComputePipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics primary render pipeline layout.
	///
	///				Pipeline layout is the interface between shader and data on the GPU, this defines how the shader gets the data.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics primary render pipeline layout.
	VkPipelineLayout										GetGraphicsPrimaryRenderPipelineLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics blur pipeline layout.
	///
	///				Pipeline layout is the interface between shader and data on the GPU, this defines how the shader gets the data.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics blur pipeline layout.
	VkPipelineLayout										GetGraphicsBlurPipelineLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics sampler descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vk2d::_internal::DescriptorSetLayout			&	GetGraphicsSamplerDescriptorSetLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics texture descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vk2d::_internal::DescriptorSetLayout			&	GetGraphicsTextureDescriptorSetLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics render target blur texure descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vk2d::_internal::DescriptorSetLayout			&	GetGraphicsRenderTargetBlurTextureDescriptorSetLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics uniform buffer descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vk2d::_internal::DescriptorSetLayout			&	GetGraphicsUniformBufferDescriptorSetLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics storage buffer descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vk2d::_internal::DescriptorSetLayout			&	GetGraphicsStorageBufferDescriptorSetLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get default texture.
	///
	///				Default texture is used when a specific texture is not found, is not loaded yet or is not used. This is just a
	///				white 1x1 pixel that is used instead.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default texture handle.
	vk2d::Texture										*	GetDefaultTexture() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get default sampler.
	///
	///				Default sampler is used when a specific sampler is not used. This sampler is set to some reasonable defaults.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default sampler handle.
	vk2d::Sampler										*	GetDefaultSampler() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get blur sampler descriptor set.
	///
	///				Sampler descriptor set used with blur pass.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default sampler handle.
	VkDescriptorSet											GetBlurSamplerDescriptorSet() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get device memory pool.
	///
	///				Vulkan objects require memory backing and that memory must be managed manually as only a limited number of
	///				allocations from the GPU are permitted. Alignment requirements and memory types complicate things further.
	///				Device memory pool takes care of these details.
	///
	/// @see		vk2d::_internal::DeviceMemoryPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Pointer to device memory pool.
	vk2d::_internal::DeviceMemoryPool					*	GetDeviceMemoryPool() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get id of the thread that made this VK2D instance.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Thread id.
	std::thread::id											GetCreatorThreadID() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the calling thread is the same as the thread that created this VK2D instance.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		true if the thread is the same that created this VK2D instance, false otherwise.
	bool													IsThisThreadCreatorThread() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Indirection to Vulkan extension. vkCmdPushDescriptorSetKHR().
	///
	///				This allows to updating descriptor sets directly into the command buffer.
	///
	/// @note		Multithreading: Any thread.
	/// 
	/// @param		commandBuffer
	///				Vulkan command buffer to record this command to.
	/// 
	/// @param		pipelineBindPoint
	///				Which pipeline to bind.
	///
	/// @param		layout
	///				Pipeline layout.
	/// 
	/// @param		set
	///				Which descriptor set to update.
	/// 
	/// @param		descriptorWriteCount
	///				Number of elements in pDescriptorWrites.
	/// 
	/// @param		pDescriptorWrites
	///				Pointer to an array of VkWriteDescriptorSet objects describing how the descriptor sets are updated.
	void													VkFun_vkCmdPushDescriptorSetKHR(
		VkCommandBuffer										commandBuffer,
		VkPipelineBindPoint									pipelineBindPoint,
		VkPipelineLayout									layout,
		uint32_t											set,
		uint32_t											descriptorWriteCount,
		const VkWriteDescriptorSet						*	pDescriptorWrites );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	bool													IsGood() const;

private:
	bool													CreateInstance();
	bool													PickPhysicalDevice();
	bool													CreateDeviceAndQueues();
	bool													CreateDescriptorPool();
	bool													CreateDefaultSampler();
	bool													CreateBlurSampler();
	bool													CreatePipelineCache();
	bool													CreateShaderModules();
	bool													CreateDescriptorSetLayouts();
	bool													CreatePipelineLayouts();
	bool													CreateDeviceMemoryPool();
	bool													CreateThreadPool();
	bool													CreateResourceManager();
	bool													CreateDefaultTexture();
	bool													PopulateNonStaticallyExposedVulkanFunctions();

	void													DestroyInstance();
	void													DestroyDevice();
	void													DestroyDescriptorPool();
	void													DestroyDefaultSampler();
	void													DestroyBlurSampler();
	void													DestroyPipelineCaches();
	void													DestroyPipelines();
	void													DestroyShaderModules();
	void													DestroyDescriptorSetLayouts();
	void													DestroyPipelineLayouts();
	void													DestroyDeviceMemoryPool();
	void													DestroyThreadPool();
	void													DestroyResourceManager();
	void													DestroyDefaultTexture();

	std::vector<VkPhysicalDevice>							EnumeratePhysicalDevices();
	VkPhysicalDevice										PickBestVulkanPhysicalDevice();

	vk2d::Instance										*	my_interface							= {};

	vk2d::PFN_MonitorUpdateCallback							monitor_update_callback					= nullptr;

	static uint64_t											instance_count;		// used to keep track of Instance instances

	vk2d::InstanceCreateInfo								create_info_copy						= {};

	std::vector<const char*>								instance_layers;
	std::vector<const char*>								instance_extensions;
	std::vector<const char*>								device_extensions;

	vk2d::PFN_VK2D_ReportFunction							report_function							= {};
	std::mutex												report_mutex;

	std::unique_ptr<vk2d::ResourceManager>					resource_manager;
	std::unique_ptr<vk2d::_internal::ThreadPool>			thread_pool;
	std::vector<uint32_t>									loader_threads;
	std::vector<uint32_t>									general_threads;

	VkDebugUtilsMessengerEXT								vk_debug_utils_messenger				= {};

	VkInstance												vk_instance								= {};
	VkPhysicalDevice										vk_physical_device						= {};
	VkDevice												vk_device								= {};

	VkPhysicalDeviceProperties								vk_physical_device_properties			= {};
	VkPhysicalDeviceMemoryProperties						vk_physical_device_memory_properties	= {};
	VkPhysicalDeviceFeatures								vk_physical_device_features				= {};

	std::vector<VkShaderModule>								vk_graphics_shader_modules;
	std::vector<VkShaderModule>								vk_compute_shader_modules;

	std::map<vk2d::_internal::GraphicsShaderProgramID, vk2d::_internal::GraphicsShaderProgram>
															graphics_shader_programs;
	std::map<vk2d::_internal::ComputeShaderProgramID, VkShaderModule>
															compute_shader_programs;

	std::map<vk2d::_internal::GraphicsPipelineSettings, VkPipeline>
															vk_graphics_pipelines;
	std::map<vk2d::_internal::ComputePipelineSettings, VkPipeline>
															vk_compute_pipelines;

	VkPipelineCache											vk_graphics_pipeline_cache					= {};
	VkPipelineCache											vk_compute_pipeline_cache					= {};

	VkPipelineLayout										vk_graphics_primary_render_pipeline_layout	= {};
	VkPipelineLayout										vk_graphics_blur_pipeline_layout			= {};

	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_simple_sampler_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_sampler_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_texture_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_render_target_blur_texture_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_uniform_buffer_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	graphics_storage_buffer_descriptor_set_layout;

	vk2d::_internal::ResolvedQueue							primary_render_queue						= {};
	vk2d::_internal::ResolvedQueue							secondary_render_queue						= {};
	vk2d::_internal::ResolvedQueue							primary_compute_queue						= {};
	vk2d::_internal::ResolvedQueue							primary_transfer_queue						= {};

	std::unique_ptr<vk2d::_internal::DeviceMemoryPool>		device_memory_pool;

	std::mutex												descriptor_pool_mutex;
	std::unique_ptr<vk2d::_internal::DescriptorAutoPool>	descriptor_pool;

	std::unique_ptr<vk2d::Sampler>							default_sampler;
	vk2d::TextureResource								*	default_texture								= {};
	std::unique_ptr<vk2d::Sampler>							blur_sampler;
	vk2d::_internal::PoolDescriptorSet						blur_sampler_descriptor_set					= {};

	std::vector<std::unique_ptr<vk2d::Window>>				windows;
	std::vector<std::unique_ptr<vk2d::RenderTargetTexture>>	render_target_textures;
	std::vector<std::unique_ptr<vk2d::Sampler>>				samplers;
	std::vector<std::unique_ptr<vk2d::Cursor>>				cursors;

	vk2d::PFN_GamepadConnectionEventCallback				joystick_event_callback						= {};

	std::thread::id											creator_thread_id							= {};

	PFN_vkCmdPushDescriptorSetKHR							vk_fun_vkCmdPushDescriptorSetKHR			= {};

	bool													is_good										= {};
};



} // _internal



} // vk2d
