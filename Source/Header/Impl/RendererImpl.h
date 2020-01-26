#pragma once

#include "../Header/Core/SourceCommon.h"
#include "../../../Include/Interface/Renderer.h"

#include "../../Header/Core/VulkanMemoryManagement.h"
#include "../Core/QueueResolver.h"

#include "../Impl/WindowImpl.h"
#include "../Core/DescriptorSet.h"

#include <list>
#include <vector>
#include <array>
#include <memory>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>




namespace vk2d {

class Window;
class Monitor;
class Renderer;
class ResourceManager;
class Cursor;
class TextureResource;

namespace _internal {

class ThreadPool;
class DescriptorSetLayout;
class MonitorImpl;



// Descriptor set allocations.
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA					= 0;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_INDEX_BUFFER_AS_STORAGE_BUFFER		= 1;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_VERTEX_BUFFER_AS_STORAGE_BUFFER	= 2;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_TEXTURE_AND_SAMPLER				= 3;
constexpr uint32_t DESCRIPTOR_SET_ALLOCATION_TEXTURE_CHANNEL_WEIGHTS			= 4;



struct WindowCoordinateScaling {
	alignas( 8 )	vk2d::Vector2f				multiplier				= {};
	alignas( 8 )	vk2d::Vector2f				offset					= {};
};

struct WindowFrameData {
	alignas( 8 )	WindowCoordinateScaling		coordinate_scaling		= {};
};

struct PushConstants
{
	alignas( 4 )	uint32_t					index_offset			= {};	// Offset into the index buffer.
	alignas( 4 )	uint32_t					index_count				= {};	// Amount of indices this shader should handle.
	alignas( 4 )	uint32_t					vertex_offset			= {};	// Offset to first vertex in vertex buffer.
	alignas( 4 )	uint32_t					texture_channel_offset	= {};	// Location of the texture channels in the texture channel weights ssbo.
	alignas( 4 )	uint32_t					texture_channel_count	= {};	// Just the amount of texture channels.
};




class RendererImpl {
	friend class vk2d::Renderer;

public:
	RendererImpl(
		const vk2d::RendererCreateInfo					&	renderer_create_info );

	~RendererImpl();

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

	const VkPhysicalDeviceProperties					&	GetPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties				&	GetPhysicalDeviceMemoryProperties() const;
	const VkPhysicalDeviceFeatures						&	GetPhysicalDeviceFeatures() const;

	VkShaderModule											GetVertexShaderModule() const;
	VkShaderModule											GetFragmentShaderModule() const;

	VkPipelineCache											GetPipelineCache() const;
	VkPipelineLayout										GetPipelineLayout() const;

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
	void													DestroyShaderModules();
	void													DestroyDescriptorSetLayouts();
	void													DestroyPipelineLayout();
	void													DestroyDeviceMemoryPool();
	void													DestroyThreadPool();
	void													DestroyResourceManager();
	void													DestroyDefaultTexture();

	std::vector<VkPhysicalDevice>							EnumeratePhysicalDevices();
	VkPhysicalDevice										PickBestPhysicalDevice();

	vk2d::MonitorUpdateCallbackFun							monitor_update_callback					= nullptr;

private:
	static uint64_t											renderer_count;		// used to keep track of Renderer instances

	vk2d::RendererCreateInfo								create_info_copy						= {};

	std::vector<const char*>								instance_layers							= {};
	std::vector<const char*>								instance_extensions						= {};
	std::vector<const char*>								device_extensions						= {};

	vk2d::PFN_VK2D_ReportFunction							report_function							= {};
	std::mutex												report_mutex;

	std::unique_ptr<vk2d::ResourceManager>					resource_manager						= {};
	std::unique_ptr<vk2d::_internal::ThreadPool>			thread_pool								= {};
	std::vector<uint32_t>									loader_threads							= {};
	std::vector<uint32_t>									general_threads							= {};

	VkDebugUtilsMessengerEXT								debug_utils_messenger					= {};

	VkInstance												instance								= {};
	VkPhysicalDevice										physical_device							= {};
	VkDevice												device									= {};

	VkPhysicalDeviceProperties								physical_device_properties				= {};
	VkPhysicalDeviceMemoryProperties						physical_device_memory_properties		= {};
	VkPhysicalDeviceFeatures								physical_device_features				= {};

	VkShaderModule											vertex_shader_module					= {};
	VkShaderModule											fragment_shader_module					= {};
	VkPipelineCache											pipeline_cache							= {};
	VkPipelineLayout										pipeline_layout							= {};

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
