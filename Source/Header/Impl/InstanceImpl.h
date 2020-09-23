#pragma once

#include "../Core/SourceCommon.h"
#include "../../../Include/Interface/Instance.h"

#include "../../Header/Core/VulkanMemoryManagement.h"
#include "../Core/QueueResolver.h"

#include "../Core/DescriptorSet.h"
#include "../Core/ShaderInterface.h"

#include "WindowImpl.h"
#include "RenderTargetTextureImpl.h"

#include "../../Include/Interface/Window.h"
#include "../../Include/Interface/Sampler.h"

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

void UpdateMonitorLists( bool globals_locked );



class InstanceImpl {
	friend class vk2d::Instance;
	friend void vk2d::_internal::UpdateMonitorLists( bool globals_locked );

public:
	///				Constructor.
	///	@note		Multithreading: Any thread originally, that thread will then be
	///				considered as the main thread for all child vk2d objects.
	/// @param[in]	instance_create_info 
	///				Reference to InstanceCreateInfo object.
	InstanceImpl(
		const vk2d::InstanceCreateInfo					&	instance_create_info );

	/// @note		Multithreading: Main thread only.
	~InstanceImpl();

	///				Get a list of monitors connected to the system, this will be
	///				needed later if the vk2d application is ran fullscreen mode.
	/// @note		Multithreading: Main thread only.
	/// @return		A list of handles to monitors.
	std::vector<vk2d::Monitor*>								GetMonitors();

	///				Gets the primary monitor of the system, this will be needed
	///				later if the vk2d application is ran fullscreen mode.
	/// @note		Multithreading: Any thread.
	/// @return		Handle to the primary monitor of the system.
	vk2d::Monitor										*	GetPrimaryMonitor() const;

	///				Set monitor update callback, this registers a callback that will be called if
	///				monitors got removed or new monitors were plugged into the system.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	monitor_update_callback_funtion
	///				Function pointer to callback that will be called when monitor is added or
	///				removed from the system.
	void													SetMonitorUpdateCallback(
		vk2d::MonitorUpdateCallbackFun						monitor_update_callback_funtion );

	///				Create a new cursor. Cursor object is needed to set hardware
	///				cursor image. See Cursor class for more information.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	image_path
	///				Path to the image file. Supported formats are
	///				JPG, PNG, TGA, BMP, PSD, GIF, PIC
	/// @param[in]	hot_spot
	///				hot spot is an offset from the image 0x0 coords to the tip
	///				of the cursor. Eg, circular cursor where you want the
	///				exact centre of the image to be the "tip" and the the image
	///				is 64x64 pixels, the hot spot would be 32x32 pixels.
	/// @return		Handle to new Cursor object.
	vk2d::Cursor										*	CreateCursor(
		const std::filesystem::path						&	image_path,
		vk2d::Vector2i										hot_spot );

	// Main thread only.
	vk2d::Cursor										*	CreateCursor(
		vk2d::Vector2u										image_size,
		const std::vector<vk2d::Color8>					&	image_data,
		vk2d::Vector2i										hot_spot );

	// Main thread only.
	void													DestroyCursor(
		vk2d::Cursor									*	cursor );

	// Any thread.
	vk2d::GamepadEventCallbackFun							GetGamepadEventCallback() const;

	// Main thread only.
	void													SetGamepadEventCallback(
		vk2d::GamepadEventCallbackFun						gamepad_event_callback_function );

	// Main thread only.
	bool													IsGamepadPresent(
		vk2d::Gamepad										gamepad );

	// Main thread only.
	std::string												GetGamepadName(
		vk2d::Gamepad										gamepad );

	// Main thread only.
	vk2d::GamepadState										QueryGamepadState(
		vk2d::Gamepad										gamepad );

	// Main thread only.
	// TODO: gamepad mapping
	void													SetGamepadMapping();

	// Main thread only.
	vk2d::Window										*	CreateOutputWindow(
		const vk2d::WindowCreateInfo					&	window_create_info );

	// Main thread only.
	void													DestroyOutputWindow(
		vk2d::Window									*	window );

	// Main thread only.
	vk2d::RenderTargetTexture							*	CreateRenderTargetTexture(
		const vk2d::RenderTargetTextureCreateInfo		&	render_target_texture_create_info );

	// Main thread only.
	void													DestroyRenderTargetTexture(
		vk2d::RenderTargetTexture						*	render_target_texture );

	// Main thread only.
	// vk2d::_internal::DescriptorAutoPool					*	GetDescriptorPool();

	/// 			Allocate descriptor set directly from instance.
	/// 			Prefer to use per thread descriptor auto pool if possible.
	///				Instance field DescriptorAutoPool is not directly exposed as
	///				DescriptorAutoPool is single thread only.
	///	@note		Multithreading: Any thread.
	/// @param[in]	for_descriptor_set_layout
	///				tells what type of a descriptor set we should allocate.
	/// @return		PoolDescriptorSet
	vk2d::_internal::PoolDescriptorSet						AllocateDescriptorSet(
		const vk2d::_internal::DescriptorSetLayout		&	for_descriptor_set_layout );

	///				Free descriptor set that was directly allocated from instance.
	///				Prefer to use per thread descriptor auto pool if possible.
	///				Instance field DescriptorAutoPool is not directly exposed as
	///				DescriptorAutoPool is single thread only.
	/// @note		Multithreading: Any thread.
	/// @param[in]	descriptor_set
	///				DescriptorSet that was previously allocated from the same instance.
	void													FreeDescriptorSet(
		vk2d::_internal::PoolDescriptorSet				&	descriptor_set );

	///				Create sampler and return a handle to it. InstanceImpl will save
	///				the sampler internally so we don't have to worry about freeing
	///				manually at the end, though it can be done with DestroySampler().
	/// @note		Multithreading: Main thread only.
	/// @param[in]	sampler_create_info 
	///				SamplerCreateInfo structure defines what type of sampler we want to create.
	/// @return		new Sampler object handle.
	vk2d::Sampler										*	CreateSampler(
		const vk2d::SamplerCreateInfo					&	sampler_create_info );

	///				Manually destroy Sampler. If parameter is nullptr then this
	///				function does nothing.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	sampler
	///				pointer to Sampler object handle or nullptr.
	void													DestroySampler(
		vk2d::Sampler									*	sampler );

	// Any thread.
	vk2d::Multisamples										GetMaximumSupportedMultisampling() const;

	// Any thread.
	vk2d::Multisamples										GetAllSupportedMultisampling() const;

	// Any thread.
	vk2d::PFN_VK2D_ReportFunction							GetReportFunction() const;

	// Any thread.
	void													Report(
		VkResult											vk_result,
		vk2d::ReportSeverity								severity,
		const std::string								&	message );

	// Any thread.
	void													Report(
		VkResult											vk_result,
		const std::string								&	message );

	// Any thread.
	void													Report(
		vk2d::ReportSeverity								severity,
		const std::string								&	message );

	// Any thread.
	vk2d::_internal::ThreadPool							*	GetThreadPool() const;

	// Any thread.
	const std::vector<uint32_t>							&	GetLoaderThreads() const;

	// Any thread.
	const std::vector<uint32_t>							&	GetGeneralThreads() const;

	// Any thread.
	vk2d::ResourceManager								*	GetResourceManager() const;

	// Any thread.
	VkInstance												GetVulkanInstance() const;

	// Any thread.
	VkPhysicalDevice										GetVulkanPhysicalDevice() const;

	// Any thread.
	VkDevice												GetVulkanDevice() const;

	// Any thread.
	vk2d::_internal::ResolvedQueue							GetPrimaryRenderQueue() const;

	// Any thread.
	vk2d::_internal::ResolvedQueue							GetSecondaryRenderQueue() const;

	// Any thread.
	vk2d::_internal::ResolvedQueue							GetPrimaryComputeQueue() const;

	// Any thread.
	vk2d::_internal::ResolvedQueue							GetPrimaryTransferQueue() const;


	// Any thread.
	const VkPhysicalDeviceProperties					&	GetVulkanPhysicalDeviceProperties() const;

	// Any thread.
	const VkPhysicalDeviceMemoryProperties				&	GetVulkanPhysicalDeviceMemoryProperties() const;

	// Any thread.
	const VkPhysicalDeviceFeatures						&	GetVulkanPhysicalDeviceFeatures() const;


	// Any thread.
	vk2d::_internal::ShaderProgram							GetShaderModules(
		vk2d::_internal::ShaderProgramID					id ) const;


	// Any thread.
	vk2d::_internal::ShaderProgram							GetCompatibleShaderModules(
		bool												multitextured,
		bool												custom_uv_border_color,
		uint32_t											vertices_per_primitive ) const;


	// Any thread.
	VkPipeline												GetVulkanPipeline(
		const vk2d::_internal::PipelineSettings			&	settings );


	// Any thread.
	VkPipeline												CreateVulkanPipeline(
		const vk2d::_internal::PipelineSettings			&	settings );


	// Any thread.
	VkPipelineCache											GetVulkanPipelineCache() const;

	// Any thread.
	VkPipelineLayout										GetVulkanPipelineLayout() const;


	// Any thread.
	const vk2d::_internal::DescriptorSetLayout			&	GetSamplerTextureDescriptorSetLayout() const;

	// Any thread.
	const vk2d::_internal::DescriptorSetLayout			&	GetUniformBufferDescriptorSetLayout() const;

	// Any thread.
	const vk2d::_internal::DescriptorSetLayout			&	GetStorageBufferDescriptorSetLayout() const;

	// Any thread.
	vk2d::Texture										*	GetDefaultTexture() const;

	// Any thread.
	vk2d::Sampler										*	GetDefaultSampler() const;


	// Any thread.
	vk2d::_internal::DeviceMemoryPool					*	GetDeviceMemoryPool() const;


	// Any thread.
	std::thread::id											GetCreatorThreadID() const;

	// returns true if this function was called from the same
	// thread as which created this instance, false otherwise.
	// Any thread.
	bool													IsThisThreadCreatorThread() const;

	// Any thread.
	bool													IsGood() const;

private:
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

	std::vector<VkShaderModule>								vk_shader_modules;

	std::map<vk2d::_internal::ShaderProgramID, vk2d::_internal::ShaderProgram>	shader_programs;
	std::map<vk2d::_internal::PipelineSettings, VkPipeline>						vk_pipelines;

	VkPipelineCache											vk_pipeline_cache						= {};
	VkPipelineLayout										vk_pipeline_layout						= {};

	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	sampler_texture_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	uniform_buffer_descriptor_set_layout;
	std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	storage_buffer_descriptor_set_layout;

	vk2d::_internal::ResolvedQueue							primary_render_queue					= {};
	vk2d::_internal::ResolvedQueue							secondary_render_queue					= {};
	vk2d::_internal::ResolvedQueue							primary_compute_queue					= {};
	vk2d::_internal::ResolvedQueue							primary_transfer_queue					= {};

	std::unique_ptr<vk2d::_internal::DeviceMemoryPool>		device_memory_pool;

	std::mutex												descriptor_pool_mutex;
	std::unique_ptr<vk2d::_internal::DescriptorAutoPool>	descriptor_pool;

	std::unique_ptr<vk2d::Sampler>							default_sampler;

	vk2d::TextureResource								*	default_texture							= {};

	std::vector<std::unique_ptr<vk2d::Window>>				windows;
	std::vector<std::unique_ptr<vk2d::RenderTargetTexture>>	render_target_textures;
	std::vector<std::unique_ptr<vk2d::Sampler>>				samplers;
	std::vector<std::unique_ptr<vk2d::Cursor>>				cursors;

	vk2d::GamepadEventCallbackFun							joystick_event_callback					= {};

	std::thread::id											creator_thread_id						= {};

	bool													is_good									= {};
};



} // _internal



} // vk2d
