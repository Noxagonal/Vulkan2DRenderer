#pragma once

#include <core/SourceCommon.hpp>
#include <interface/instance/Instance.hpp>

#include <containers/Color.hpp>

#include <vulkan/Instance.hpp>
#include <vulkan/Device.hpp>
#include <vulkan/Queue.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>
#include <vulkan/shaders/ShaderInterface.hpp>
#include <system/ThreadPool.hpp>

#include <interface/resources/texture/TextureResourceHandle.hpp>

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

namespace vulkan {
class DescriptorSetLayout;
class DeviceMemoryPool;
} // vulkan

namespace vk2d_internal {

class ThreadPool;
class WindowImpl;
class MonitorImpl;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UpdateMonitorLists( bool globals_locked );



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InstanceImpl {
	friend class Instance;
	friend void UpdateMonitorLists( bool globals_locked );

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::Instance()
	InstanceImpl(
		Instance										&	my_interface,
		const InstanceCreateInfo						&	instance_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::~Instance()
	~InstanceImpl();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get create info structure that was used when creating this instance.
	///
	/// @return		Create info struct used to create this instance.
	const InstanceCreateInfo								GetCreateInfo() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::Run()
	bool													Run();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::GetMonitors()
	std::vector<Monitor*>									GetMonitors();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::GetPrimaryMonitor()
	Monitor												*	GetPrimaryMonitor() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::SetMonitorUpdateCallback()
	void													SetMonitorUpdateCallback(
		PFN_MonitorUpdateCallback							monitor_update_callback_funtion
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::CreateCursor()
	Cursor												*	CreateCursor(
		const std::filesystem::path						&	image_path,
		glm::ivec2											hot_spot
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::CreateCursor()
	Cursor												*	CreateCursor(
		glm::uvec2											image_size,
		const std::vector<Color8>						&	image_data,
		glm::ivec2											hot_spot
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::DestroyCursor()
	void													DestroyCursor(
		Cursor											*	cursor
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::GetGamepadEventCallback()
	PFN_GamepadConnectionEventCallback						GetGamepadEventCallback() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::SetGamepadEventCallback()
	void													SetGamepadEventCallback(
		PFN_GamepadConnectionEventCallback					gamepad_event_callback_function
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::IsGamepadPresent()
	bool													IsGamepadPresent(
		Gamepad												gamepad
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::GetGamepadName()
	std::string												GetGamepadName(
		Gamepad												gamepad
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::QueryGamepadState()
	GamepadState											QueryGamepadState(
		Gamepad												gamepad
	);

	// TODO: gamepad mapping

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::SetGamepadMapping()
	void													SetGamepadMapping();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::CreateOutputWindow()
	Window												*	CreateOutputWindow(
		const WindowCreateInfo							&	window_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::DestroyOutputWindow()
	void													DestroyOutputWindow(
		Window											*	window
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::CreateRenderTargetTexture()
	RenderTargetTexture									*	CreateRenderTargetTexture(
		const RenderTargetTextureCreateInfo				&	render_target_texture_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see Instance::DestroyRenderTargetTexture()
	void													DestroyRenderTargetTexture(
		RenderTargetTexture								*	render_target_texture
	);

	// DescriptorAutoPool								*	GetDescriptorPool();

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
	vulkan::PoolDescriptorSet								AllocateDescriptorSet_DEPRICATED(
		const vulkan::DescriptorSetLayout				&	for_descriptor_set_layout
	);

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
	void													FreeDescriptorSet_DEPRICATED(
		vulkan::PoolDescriptorSet						&	descriptor_set
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		Instance::CreateSampler()
	Sampler												*	CreateSampler(
		const SamplerCreateInfo							&	sampler_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		Instance::DestroySampler()
	void													DestroySampler(
		Sampler											*	sampler
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		Instance::GetMaximumSupportedMultisampling()
	Multisamples											GetMaximumSupportedMultisampling() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		Instance::GetAllSupportedMultisampling()
	Multisamples											GetAllSupportedMultisampling() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get report function so that parts of VK2D may log messages.
	PFN_VK2D_ReportFunction									GetReportFunction() const;

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
		ReportSeverity										severity,
		std::string_view									message
	) const;

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
		std::string_view									message
	) const;

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
		ReportSeverity										severity,
		std::string_view									message
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get internal thread pool.
	///
	/// @see		ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Pointer to thread pool.
	ThreadPool											*	GetThreadPool();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get loader threads.
	///
	///				Loader threads are threads that are allowed to access the system's hard drive. These are generally used to copy
	///				file contents into RAM.
	///
	/// @see		ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		List of loader threads indices.
	const std::vector<uint32_t>							&	GetLoaderThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get material loader thread.
	///
	///				Material loader thread is a singular thread that is used to load materials. It has the same capabilities as a
	///				normal loader thread but is reserved specifically for materials. This is mostly because glslang is not
	///				multithreaded so this thread is kept separate.
	///
	/// @see		ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Material loader thread index.
	uint32_t												GetMaterialLoaderThread() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get general threads.
	///
	///				General threads are meant to do heavy lifting like translating data and decoding packed image files like png
	///				into a list of pixels.
	/// 
	/// @see		ThreadPool
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		List of loader threads indices.
	const std::vector<uint32_t>							&	GetGeneralThreads() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @see		Instance::GetResourceManager()
	ResourceManager										*	GetResourceManager() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan instance.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan instance.
	vulkan::Instance									&	GetVulkanInstance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get Vulkan device.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Vulkan device.
	vulkan::Device										&	GetVulkanDevice();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics shader modules.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	id
	///				Graphics shader program ID. See GraphicsShaderListID_DEPRICATED for more info.
	///
	/// @return		Graphics shader program.
	vulkan::GraphicsShaderList								GetGraphicsShaderList_DEPRICATED(
		vulkan::GraphicsShaderListID_DEPRICATED				id
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute shader modules.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	id
	///				Compute shader program ID. See ComputeShaderProgramID_DEPRICATED for more info.
	///
	/// @return		Compute shader program.
	VkShaderModule											GetComputeShaderModules_DEPRICATED(
		vulkan::ComputeShaderProgramID_DEPRICATED			id
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compatible graphics shader modules.
	///
	///				This is used to find an existing shader module which is compatible with these properties.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	custom_uv_border_color
	///				Tells if the graphics shader need to support custom uv border color.
	/// 
	/// @param[in]	vertices_per_primitive
	///				Tells how many vertices per primitive the shader needs to support, must be a value between 1 and 3 (inclusive).
	///
	/// @return		Graphics shader program.
	vulkan::GraphicsShaderList								GetCompatibleGraphicsShaderList_DEPRICATED(
		bool												custom_uv_border_color,
		uint32_t											vertices_per_primitive
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics pipeline.
	///
	///				Tries to find a graphics pipeline matching these pipeline settings. If no existing pipeline is found, a new one
	///				is created and added to the list of existing pipelines before being returned.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	graphics_pipeline_info
	///				Information about what kind of pipeline we want.
	///
	/// @return		Graphics shader pipeline.
	VkPipeline												GetGraphicsPipeline_DEPRICATED(
		const vulkan::GraphicsPipelineCreateInfo		&	graphics_pipeline_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute pipeline.
	///
	///				Tries to find a compute pipeline matching these pipeline settings. If no existing pipeline is found, a new one
	///				is created and added to the list of existing pipelines before being returned.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	compute_pipeline_info
	///				Information about what kind of pipeline we want.
	///
	/// @return		Graphics shader pipeline.
	VkPipeline												GetComputePipeline_DEPRICATED(
		const vulkan::ComputePipelineCreateInfo			&	compute_pipeline_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics primary render pipeline layout.
	///
	///				Pipeline layout is the interface between shader and data on the GPU, this defines how the shader gets the data.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics primary render pipeline layout.
	VkPipelineLayout										GetGraphicsPrimaryRenderPipelineLayout_MOVE() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics blur pipeline layout.
	///
	///				Pipeline layout is the interface between shader and data on the GPU, this defines how the shader gets the data.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics blur pipeline layout.
	VkPipelineLayout										GetGraphicsBlurPipelineLayout_DEPRICATED() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics sampler descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vulkan::DescriptorSetLayout					&	GetGraphicsSamplerDescriptorSetLayout_MOVE() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics texture descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vulkan::DescriptorSetLayout					&	GetGraphicsTextureDescriptorSetLayout_MOVE() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics render target blur texure descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vulkan::DescriptorSetLayout					&	GetGraphicsRenderTargetBlurTextureDescriptorSetLayout_DEPRICATED() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics uniform buffer descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vulkan::DescriptorSetLayout					&	GetGraphicsUniformBufferDescriptorSetLayout_MOVE() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics storage buffer descriptor set layout.
	///
	///				Descriptor set layout is the layout for a set of data that may be bound at a time in shader.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Descriptor set layout.
	const vulkan::DescriptorSetLayout					&	GetGraphicsStorageBufferDescriptorSetLayout_MOVE() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get default texture.
	///
	///				Default texture is used when a specific texture is not found, is not loaded yet or is not used. This is just a
	///				white 1x1 pixel that is used instead.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default texture handle.
	Texture												*	GetDefaultTexture();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get default sampler.
	///
	///				Default sampler is used when a specific sampler is not used. This sampler is set to some reasonable defaults.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default sampler handle.
	Sampler												*	GetDefaultSampler();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get blur sampler descriptor set.
	///
	///				Sampler descriptor set used with blur pass.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Default sampler handle.
	VkDescriptorSet											GetBlurSamplerDescriptorSet_DEPRICATED() const;

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
		const VkWriteDescriptorSet						*	pDescriptorWrites
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	bool													IsGood() const;

private:
	bool													CreateInstance();
	bool													CreateDeviceAndQueues();
	bool													CreateDescriptorPool();
	bool													CreateDefaultSampler();
	bool													CreateBlurSampler();
	bool													CreateShaderModules_DEPRICATED();
	bool													CreateDescriptorSetLayouts_MOVE();
	bool													CreatePipelineLayouts_MOVE();
	bool													CreateThreadPool();
	bool													CreateResourceManager();
	bool													CreateDefaultTexture();
	bool													PopulateNonStaticallyExposedVulkanFunctions();

	void													DestroyInstance();
	void													DestroyDevice();
	void													DestroyDescriptorPool();
	void													DestroyDefaultSampler();
	void													DestroyBlurSampler();
	void													DestroyShaderModules_DEPRICATED();
	void													DestroyDescriptorSetLayouts();
	void													DestroyPipelineLayouts_MOVE();
	void													DestroyThreadPool();
	void													DestroyResourceManager();
	void													DestroyDefaultTexture();

	Instance											&	my_interface;

	PFN_MonitorUpdateCallback								monitor_update_callback					= nullptr;

	static uint64_t											instance_count;		// used to keep track of number of instances.

	InstanceCreateInfo										create_info_copy						= {};

	std::optional<vulkan::Instance>							vulkan_instance							= {};
	std::optional<vulkan::Device>							vulkan_device							= {};

	PFN_VK2D_ReportFunction									report_function							= {};
	mutable std::mutex										report_mutex;

	std::unique_ptr<ResourceManager>						resource_manager;
	std::optional<ThreadPool>								thread_pool;
	std::vector<uint32_t>									loader_threads;
	uint32_t												material_loader_thread					= {};
	std::vector<uint32_t>									general_threads;

	std::vector<VkShaderModule>								vk_graphics_shader_modules_DEPRICATED;
	std::vector<VkShaderModule>								vk_compute_shader_modules_DEPRICATED;

	std::map<vulkan::GraphicsShaderListID_DEPRICATED, vulkan::GraphicsShaderList>graphics_shader_programs_DEPRICATED;
	std::map<vulkan::ComputeShaderProgramID_DEPRICATED, VkShaderModule>compute_shader_programs_DEPRICATED;

	std::map<vulkan::GraphicsPipelineCreateInfo, vulkan::PipelineHandle>vk_graphics_pipelines_DEPRICATED;
	std::map<vulkan::ComputePipelineCreateInfo, vulkan::PipelineHandle>	vk_compute_pipelines_DEPRICATED;

	VkPipelineLayout										vk_graphics_primary_render_pipeline_layout_MOVE	= {};
	VkPipelineLayout										vk_graphics_blur_pipeline_layout_DEPRICATED			= {};

	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_simple_sampler_descriptor_set_layout_MOVE;
	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_sampler_descriptor_set_layout_MOVE;
	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_texture_descriptor_set_layout_MOVE;
	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_render_target_blur_texture_descriptor_set_layout_MOVE;
	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_uniform_buffer_descriptor_set_layout_MOVE;
	std::unique_ptr<vulkan::DescriptorSetLayout>			graphics_storage_buffer_descriptor_set_layout_MOVE;

	std::mutex												descriptor_pool_mutex;
	std::unique_ptr<vulkan::DescriptorAutoPool>				descriptor_pool;

	std::unique_ptr<Sampler>								default_sampler;
	TextureResourceHandle									default_texture;
	std::unique_ptr<Sampler>								blur_Sampler;
	vulkan::PoolDescriptorSet								blur_sampler_descriptor_set_DEPRICATED		= {};

	std::vector<std::unique_ptr<Window>>					windows;
	std::vector<std::unique_ptr<RenderTargetTexture>>		render_target_textures;
	std::vector<std::unique_ptr<Sampler>>					samplers;
	std::vector<std::unique_ptr<Cursor>>					cursors;

	PFN_GamepadConnectionEventCallback						joystick_event_callback						= {};

	std::thread::id											creator_thread_id							= {};

	PFN_vkCmdPushDescriptorSetKHR							vk_fun_vkCmdPushDescriptorSetKHR			= {};

	bool													is_good										= {};
};



} // vk2d_internal



} // vk2d
