
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/RendererImpl.h"
#include "../../Include/Interface/ResourceManager.h"
#include "../Header/Impl/ResourceManagerImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Core/QueueResolver.h"
#include "../Header/Core/ThreadPool.h"
#include "../Header/Core/ThreadPrivateResources.h"
#include "../Header/Core/DescriptorSet.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Sampler.h"

#include "../Shaders/shader.vert.spv.h"
#include "../Shaders/shader.frag.spv.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include <sstream>
#include <iostream>
#include <vector>
#include <list>


namespace vk2d {

namespace _internal {

vk2d::Monitor									primary_monitor				= {};
std::vector<vk2d::Monitor>						all_monitors				= {};
std::list<vk2d::_internal::RendererImpl*>		renderer_listeners			= {};

void UpdateMonitorLists()
{
	auto BuildMonitorData = [](
		GLFWmonitor		*	monitor
		) -> std::unique_ptr<vk2d::_internal::MonitorImpl>
	{
		VkOffset2D position {};
		{
			int posX = 0, posY = 0;
			glfwGetMonitorPos( monitor, &posX, &posY );
			position		= { int32_t( posX ), int32_t( posY ) };
		}

		VkExtent2D physical_size {};
		{
			int widthMM = 0, heightMM = 0;
			glfwGetMonitorPhysicalSize( monitor, &widthMM, &heightMM );
			physical_size	= { uint32_t( widthMM ), uint32_t( heightMM ) };
		}

		std::string					name					= glfwGetMonitorName( monitor );
		vk2d::MonitorVideoMode		current_video_mode		= {};
		{
			auto glfw_current_video_mode	= glfwGetVideoMode( monitor );
			current_video_mode.size			= { uint32_t( glfw_current_video_mode->width ), uint32_t( glfw_current_video_mode->height ) };
			current_video_mode.redBits		= uint32_t( glfw_current_video_mode->redBits );
			current_video_mode.greenBits	= uint32_t( glfw_current_video_mode->greenBits );
			current_video_mode.blueBits		= uint32_t( glfw_current_video_mode->blueBits );
			current_video_mode.refreshRate	= uint32_t( glfw_current_video_mode->refreshRate );
		}

		std::vector<vk2d::MonitorVideoMode> video_modes		= {};
		{
			int vidModeCount = 0;
			auto glfw_video_modes = glfwGetVideoModes( monitor, &vidModeCount );
			video_modes.resize( vidModeCount );
			for( int i = 0; i < vidModeCount; ++i ) {
				video_modes[ i ].size			= { uint32_t( glfw_video_modes[ i ].width ), uint32_t( glfw_video_modes[ i ].height ) };
				video_modes[ i ].redBits		= glfw_video_modes[ i ].redBits;
				video_modes[ i ].greenBits		= glfw_video_modes[ i ].greenBits;
				video_modes[ i ].blueBits		= glfw_video_modes[ i ].blueBits;
				video_modes[ i ].refreshRate	= glfw_video_modes[ i ].refreshRate;
			};
		}


		return std::make_unique<vk2d::_internal::MonitorImpl>(
			monitor,
			position,
			physical_size,
			name,
			current_video_mode,
			video_modes
			);
	};

	// All monitors
	vk2d::_internal::all_monitors.clear();
	int monitorCount = 0;
	auto monitors = glfwGetMonitors( &monitorCount );
	vk2d::_internal::all_monitors.reserve( monitorCount );
	for( int i = 0; i < monitorCount; ++i ) {

		GLFWmonitor * monitor		= monitors[ i ];
		vk2d::_internal::all_monitors.push_back( std::move( vk2d::Monitor( BuildMonitorData( monitor ) ) ) );
	}

	// primary monitor
	vk2d::_internal::primary_monitor = std::move( vk2d::Monitor( BuildMonitorData( glfwGetPrimaryMonitor() ) ) );

	// callback
	for( auto l : vk2d::_internal::renderer_listeners ) {
		if( l->monitor_update_callback ) {
			l->monitor_update_callback();
		}
	}
}

void glfwMonitorCallback(
	GLFWmonitor		*	monitor,
	int					event
)
{
	vk2d::_internal::UpdateMonitorLists();
}



void glfwJoystickEventCallback( int joystick, int event )
{
	if( event == GLFW_CONNECTED ) {
		std::string joystic_name = glfwGetJoystickName( joystick );

		for( auto l : vk2d::_internal::renderer_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( vk2d::Gamepad( joystick ), vk2d::GamepadEvent::CONNECTED, joystic_name );
			}
		}
	} else {
		for( auto l : vk2d::_internal::renderer_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( vk2d::Gamepad( joystick ), vk2d::GamepadEvent::DISCONNECTED, std::string( "" ) );
			}
		}
	}
}

} // _internal

} // vk2d



uint64_t vk2d::_internal::RendererImpl::renderer_count				= 0;

vk2d::_internal::RendererImpl::RendererImpl( const RendererCreateInfo & renderer_create_info )
{
	vk2d::_internal::renderer_listeners.push_back( this );

	// Initialize glfw if this is the first renderer.
	if( renderer_count == 0 ) {
		if( glfwInit() != GLFW_TRUE ) {
			if( renderer_create_info.report_function ) {
				renderer_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot initialize GLFW!" );
			}
			return;
		}
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwSetMonitorCallback( vk2d::_internal::glfwMonitorCallback );
		glfwSetJoystickCallback( vk2d::_internal::glfwJoystickEventCallback );
		vk2d::_internal::UpdateMonitorLists();
	}
	++renderer_count;

	// Check if vulkan is supported
	if( glfwVulkanSupported() == GLFW_FALSE ) {
		if( renderer_create_info.report_function ) {
			renderer_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Vulkan is not supported on this machine!\nUpdating your graphics drivers might fix this issue!" );
		}
		return;
	}

	create_info_copy	= renderer_create_info;
	report_function		= create_info_copy.report_function;

//	Introduce layers and extensions here
//	instance_layers;
//	instance_extensions;
	device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	device_extensions.push_back( VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME );
	{
		// Adding glfw instance extensions
		uint32_t glfw_instance_extension_count = 0;
		auto glfw_instance_extensions_ptr = glfwGetRequiredInstanceExtensions( &glfw_instance_extension_count );
		for( uint32_t i = 0; i < glfw_instance_extension_count; ++i ) {
			instance_extensions.push_back( glfw_instance_extensions_ptr[ i ] );
		}
	}

	if( !CreateInstance() ) return;
	if( !PickPhysicalDevice() ) return;
	if( !CreateDeviceAndQueues() ) return;
	if( !CreateDescriptorSetLayouts() ) return;
	if( !CreateDescriptorPool() ) return;
	if( !CreatePipelineCache() ) return;
	if( !CreateShaderModules() ) return;
	if( !CreatePipelineLayout() ) return;
	if( !CreateDeviceMemoryPool() ) return;
	if( !CreateThreadPool() ) return;
	if( !CreateResourceManager() ) return;
	if( !CreateDefaultTexture() ) return;
	if( !CreateDefaultSampler() ) return;

	is_good		= true;
}

vk2d::_internal::RendererImpl::~RendererImpl()
{
	vkDeviceWaitIdle( device );

	windows.clear();
	samplers.clear();

	DestroyDefaultSampler();
	DestroyDefaultTexture();
	DestroyResourceManager();
	DestroyThreadPool();
	DestroyDeviceMemoryPool();
	DestroyPipelineLayout();
	DestroyShaderModules();
	DestroyPipelineCache();
	DestroyDescriptorPool();
	DestroyDescriptorSetLayouts();
	DestroyDevice();
	DestroyInstance();

	// If this is the last renderer exiting, also terminate glfw
	--renderer_count;
	if( renderer_count == 0 ) {
		glfwTerminate();
	}

	vk2d::_internal::renderer_listeners.remove( this );
}

std::vector<vk2d::Monitor*> vk2d::_internal::RendererImpl::GetMonitors()
{
	std::vector<vk2d::Monitor*> ret;
	ret.reserve( vk2d::_internal::all_monitors.size() );

	for( auto & m : vk2d::_internal::all_monitors ) {
		ret.push_back( &m );
	}

	return ret;
}

vk2d::Monitor * vk2d::_internal::RendererImpl::GetPrimaryMonitor()
{
	return &vk2d::_internal::primary_monitor;
}

void vk2d::_internal::RendererImpl::SetMonitorUpdateCallback(
	vk2d::MonitorUpdateCallbackFun		monitor_update_callback_funtion
)
{
	monitor_update_callback		= monitor_update_callback_funtion;
}

vk2d::GamepadEventCallbackFun vk2d::_internal::RendererImpl::GetGamepadEventCallback()
{
	return vk2d::GamepadEventCallbackFun();
}

void vk2d::_internal::RendererImpl::SetGamepadEventCallback(
	vk2d::GamepadEventCallbackFun		joystick_event_callback_function
)
{
	joystick_event_callback		= joystick_event_callback_function;
}

bool vk2d::_internal::RendererImpl::IsGamepadPresent( vk2d::Gamepad joystick )
{
	if( glfwJoystickIsGamepad( int( joystick ) ) == GLFW_TRUE ) {
		return true;
	} else {
		return false;
	}
}

std::string vk2d::_internal::RendererImpl::GetGamepadName( vk2d::Gamepad gamepad )
{
	return glfwGetGamepadName( int( gamepad ) );
}

vk2d::GamepadState vk2d::_internal::RendererImpl::QueryGamepadState(
	vk2d::Gamepad			gamepad )
{
	vk2d::GamepadState	gamepad_state {};
	GLFWgamepadstate	glfw_gamepad_state {};

	int32_t button_array_size	= int32_t( sizeof( glfw_gamepad_state.buttons ) / sizeof( *glfw_gamepad_state.buttons ) );
	button_array_size			= std::min( button_array_size, int32_t( gamepad_state.buttons.size() ) );

	int32_t axes_array_size		= int32_t( sizeof( glfw_gamepad_state.axes ) / sizeof( *glfw_gamepad_state.axes ) );
	axes_array_size				= std::min( axes_array_size, int32_t( gamepad_state.axes.size() ) );

	if( glfwGetGamepadState( int( gamepad ), &glfw_gamepad_state ) != GLFW_FALSE ) {

		for( int i = 0; i < button_array_size; ++i ) {
			gamepad_state.buttons[ i ]		= bool( glfw_gamepad_state.buttons[ i ] );
		}
		for( int i = 0; i < axes_array_size; ++i ) {
			gamepad_state.axes[ i ]			= float( glfw_gamepad_state.axes[ i ] );
		}

		return gamepad_state;
	} else {
		return {};
	}
}



vk2d::Window * vk2d::_internal::RendererImpl::CreateOutputWindow(
	vk2d::WindowCreateInfo	&	window_create_info
)
{
	auto new_window = std::unique_ptr<vk2d::Window>( new vk2d::Window( this, window_create_info ) );

	if( new_window->is_good ) {
		windows.push_back( std::move( new_window ) );
		return windows.back().get();
	}
	return {};
}

void vk2d::_internal::RendererImpl::CloseOutputWindow(
	vk2d::Window				*	window
)
{
	vkDeviceWaitIdle( device );

	auto it = windows.begin();
	while( it != windows.end() ) {
		if( it->get() == window ) {
			it = windows.erase( it );
			break;
		} else {
			++it;
		}
	}
}



vk2d::_internal::DescriptorAutoPool * vk2d::_internal::RendererImpl::GetDescriptorPool()
{
	return descriptor_pool.get();
}



vk2d::Sampler * vk2d::_internal::RendererImpl::CreateSampler(
	const vk2d::SamplerCreateInfo	&	sampler_create_info
)
{
	auto sampler	= std::unique_ptr<vk2d::Sampler>( new vk2d::Sampler( this, sampler_create_info ) );

	if( sampler && sampler->IsGood() ) {
		auto ret	= sampler.get();
		samplers.push_back( std::move( sampler ) );
		return ret;
	} else {
		return nullptr;
	}
}

void vk2d::_internal::RendererImpl::DestroySampler(
	vk2d::Sampler					*	sampler
)
{
	if( vkDeviceWaitIdle( device ) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Device lost!" );
	}

	auto it = samplers.begin();
	while( it != samplers.end() ) {
		if( it->get() == sampler ) {
			it = samplers.erase( it );
			break;
		} else {
			++it;
		}
	}
}

vk2d::Multisamples vk2d::_internal::RendererImpl::GetMaximumSupportedMultisampling()
{
	vk2d::Multisamples max_samples	= vk2d::Multisamples( physical_device_properties.limits.framebufferColorSampleCounts );
	if( uint32_t( max_samples )			& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_64 ) )	return vk2d::Multisamples::SAMPLE_COUNT_64;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_32 ) )	return vk2d::Multisamples::SAMPLE_COUNT_32;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_16 ) )	return vk2d::Multisamples::SAMPLE_COUNT_16;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_8 ) )	return vk2d::Multisamples::SAMPLE_COUNT_8;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_4 ) )	return vk2d::Multisamples::SAMPLE_COUNT_4;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_2 ) )	return vk2d::Multisamples::SAMPLE_COUNT_2;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_1 ) )	return vk2d::Multisamples::SAMPLE_COUNT_1;
	return vk2d::Multisamples::SAMPLE_COUNT_1;
}

vk2d::Multisamples vk2d::_internal::RendererImpl::GetAllSupportedMultisampling()
{
	return vk2d::Multisamples( physical_device_properties.limits.framebufferColorSampleCounts );
}









vk2d::PFN_VK2D_ReportFunction vk2d::_internal::RendererImpl::GetReportFunction() const
{
	return report_function;
}

void vk2d::_internal::RendererImpl::Report(
	vk2d::ReportSeverity		severity,
	const std::string		&	message
)
{
	if( report_function ) {
		std::lock_guard<std::mutex> report_lock( report_mutex );
		report_function(
			severity,
			message
		);
	}
}

vk2d::_internal::ThreadPool * vk2d::_internal::RendererImpl::GetThreadPool() const
{
	return thread_pool.get();
}

const std::vector<uint32_t> & vk2d::_internal::RendererImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & vk2d::_internal::RendererImpl::GetGeneralThreads() const
{
	return general_threads;
}

vk2d::ResourceManager * vk2d::_internal::RendererImpl::GetResourceManager() const
{
	return resource_manager.get();
}

VkInstance vk2d::_internal::RendererImpl::GetVulkanInstance() const
{
	return instance;
}

VkPhysicalDevice vk2d::_internal::RendererImpl::GetVulkanPhysicalDevice() const
{
	return physical_device;
}

VkDevice vk2d::_internal::RendererImpl::GetVulkanDevice() const
{
	return device;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::RendererImpl::GetPrimaryRenderQueue() const
{
	return primary_render_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::RendererImpl::GetSecondaryRenderQueue() const
{
	return secondary_render_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::RendererImpl::GetPrimaryComputeQueue() const
{
	return primary_compute_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::RendererImpl::GetPrimaryTransferQueue() const
{
	return primary_transfer_queue;
}

const VkPhysicalDeviceProperties & vk2d::_internal::RendererImpl::GetPhysicalDeviceProperties() const
{
	return physical_device_properties;
}

const VkPhysicalDeviceMemoryProperties & vk2d::_internal::RendererImpl::GetPhysicalDeviceMemoryProperties() const
{
	return physical_device_memory_properties;
}

const VkPhysicalDeviceFeatures & vk2d::_internal::RendererImpl::GetPhysicalDeviceFeatures() const
{
	return physical_device_features;
}

VkShaderModule vk2d::_internal::RendererImpl::GetVertexShaderModule() const
{
	return vertex_shader_module;
}

VkShaderModule vk2d::_internal::RendererImpl::GetFragmentShaderModule() const
{
	return fragment_shader_module;
}

VkPipelineCache vk2d::_internal::RendererImpl::GetPipelineCache() const
{
	return pipeline_cache;
}

VkPipelineLayout vk2d::_internal::RendererImpl::GetPipelineLayout() const
{
	return pipeline_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::RendererImpl::GetSamplerDescriptorSetLayout() const
{
	return *sampler_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::RendererImpl::GetTextureDescriptorSetLayout() const
{
	return *texture_descriptor_set_layout;
}

VkDescriptorSet vk2d::_internal::RendererImpl::GetDefaultTextureDescriptorSet() const
{
	return default_texture_descriptor_set.descriptorSet;
}

vk2d::Sampler * vk2d::_internal::RendererImpl::GetDefaultSampler() const
{
	return default_sampler.get();
}

vk2d::_internal::DeviceMemoryPool * vk2d::_internal::RendererImpl::GetDeviceMemoryPool() const
{
	return device_memory_pool.get();
}

bool vk2d::_internal::RendererImpl::IsGood() const
{
	return is_good;
}









#if VK2D_BUILD_OPTION_VULKAN_VALIDATION

namespace vk2d {

namespace _internal {

VkBool32 VKAPI_PTR DebugMessenger(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	void*                                            pUserData )
{
	std::string str_severity;
	switch( messageSeverity ) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		str_severity = "VERBOSE";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		str_severity = "INFO";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		str_severity = "WARNING";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		str_severity = "ERROR";
		break;
	default:
		assert( 0 );
		break;
	}

	std::string str_type;
	switch( messageTypes ) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		str_type = "GENERAL";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		str_type = "VALIDATION";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		str_type = "PERFORMANCE";
		break;
	default:
		break;
	}

	std::stringstream ss_title;
	ss_title << str_type << " " << str_severity;

	std::stringstream ss_message;
	ss_message << ss_title.str() << ":\n\n" << pCallbackData->pMessage << "\n";
	// TODO: labels, object, message id name / number;

	auto str_message = ss_message.str();
	std::cout << str_message << "\n";

#if VK_USE_PLATFORM_WIN32_KHR
	UINT MBIType		= 0;
	if( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )	MBIType	= MB_ICONWARNING;
	if( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )		MBIType	= MB_ICONERROR;
	if( MBIType ) {
		MessageBox( NULL, str_message.c_str(), ss_title.str().c_str(), MB_OK | MB_SYSTEMMODAL | MBIType );
	}
#endif

	return VK_FALSE;
}

} // _internal

} // vk2d

#endif









bool vk2d::_internal::RendererImpl::CreateInstance()
{

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
	instance_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
	instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	VkDebugUtilsMessageSeverityFlagsEXT severity_flags {};
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_INFO
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_WARNING
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_ERROR
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#endif

	VkDebugUtilsMessengerCreateInfoEXT			debug_utils_create_info {};
	debug_utils_create_info.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_utils_create_info.pNext				= nullptr;
	debug_utils_create_info.flags				= 0;
	debug_utils_create_info.messageSeverity		= severity_flags;
	debug_utils_create_info.messageType			=
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_utils_create_info.pfnUserCallback		= vk2d::_internal::DebugMessenger;
	debug_utils_create_info.pUserData			= nullptr;
#endif

	{
		VkApplicationInfo application_info {};
		application_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pNext				= nullptr;
		application_info.pApplicationName	= create_info_copy.application_name.c_str();
		application_info.applicationVersion	= create_info_copy.application_version.ToVulkanVersion();
		application_info.pEngineName		= create_info_copy.engine_name.c_str();
		application_info.engineVersion		= create_info_copy.engine_version.ToVulkanVersion();
		application_info.apiVersion			= VK_API_VERSION_1_0;

		VkInstanceCreateInfo instance_create_info {};
		instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
		instance_create_info.pNext						= &debug_utils_create_info;
#else
		instance_create_info.pNext						= nullptr;
#endif
		instance_create_info.flags						= 0;
		instance_create_info.pApplicationInfo			= &application_info;
		instance_create_info.enabledLayerCount			= uint32_t( instance_layers.size() );
		instance_create_info.ppEnabledLayerNames		= instance_layers.data();
		instance_create_info.enabledExtensionCount		= uint32_t( instance_extensions.size() );
		instance_create_info.ppEnabledExtensionNames	= instance_extensions.data();

		VkResult result = vkCreateInstance(
			&instance_create_info,
			nullptr,
			&instance
		);
		if( result != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create vulkan instance!" );
			return false;
		}
	}

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
	{
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		if( !createDebugUtilsMessenger ) {
			Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return false;
		}
		auto result = createDebugUtilsMessenger(
			instance,
			&debug_utils_create_info,
			nullptr,
			&debug_utils_messenger
		);
		if( result != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return false;
		}
	}
#endif

	return true;
}









bool vk2d::_internal::RendererImpl::PickPhysicalDevice()
{
	physical_device	= PickBestPhysicalDevice();
	if( !physical_device ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot find physical device!" );
	}

	if( physical_device ) {
		vkGetPhysicalDeviceProperties(
			physical_device,
			&physical_device_properties
		);
		vkGetPhysicalDeviceMemoryProperties(
			physical_device,
			&physical_device_memory_properties
		);
		vkGetPhysicalDeviceFeatures(
			physical_device,
			&physical_device_features
		);

		return true;
	}
	return false;
}









bool vk2d::_internal::RendererImpl::CreateDeviceAndQueues()
{
	std::vector<std::pair<VkQueueFlags, float>> queue_requests {
		{ VK_QUEUE_GRAPHICS_BIT, 1.0f },
		{ VK_QUEUE_GRAPHICS_BIT, 0.2f },
		{ VK_QUEUE_COMPUTE_BIT, 1.0f },
		{ VK_QUEUE_TRANSFER_BIT, 1.0f }
	};
	DeviceQueueResolver queue_resolver( instance, physical_device, queue_requests );
	if( !queue_resolver.IsGood() ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create queue resolver!" );
		return false;
	}
	auto queue_create_infos = queue_resolver.GetDeviceQueueCreateInfos();

	VkPhysicalDeviceFeatures features {};
	features.samplerAnisotropy					= VK_TRUE;
	features.fillModeNonSolid					= VK_TRUE;
	features.wideLines							= VK_TRUE;

	VkDeviceCreateInfo device_create_info {};
	device_create_info.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext					= nullptr;
	device_create_info.flags					= 0;
	device_create_info.queueCreateInfoCount		= uint32_t( queue_create_infos.size() );
	device_create_info.pQueueCreateInfos		= queue_create_infos.data();
	device_create_info.enabledLayerCount		= 0;
	device_create_info.ppEnabledLayerNames		= nullptr;
	device_create_info.enabledExtensionCount	= uint32_t( device_extensions.size() );
	device_create_info.ppEnabledExtensionNames	= device_extensions.data();
	device_create_info.pEnabledFeatures			= &features;

	auto result = vkCreateDevice(
		physical_device,
		&device_create_info,
		nullptr,
		&device
	);
	if( result != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan device!" );
		return false;
	}

	auto resolved_queues = queue_resolver.GetQueues( device );
	if( resolved_queues.size() != queue_requests.size() ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot get correct amount of queues from queue resolver!" );
		return false;
	}
	primary_render_queue		= resolved_queues[ 0 ];
	secondary_render_queue		= resolved_queues[ 1 ];
	primary_compute_queue		= resolved_queues[ 2 ];
	primary_transfer_queue		= resolved_queues[ 3 ];

	return true;
}

bool vk2d::_internal::RendererImpl::CreateDescriptorPool()
{
	descriptor_pool			= vk2d::_internal::CreateDescriptorAutoPool(
		this,
		device
	);

	if( descriptor_pool ) {
		return true;
	}
	return false;
}









bool vk2d::_internal::RendererImpl::CreateDefaultSampler()
{
	vk2d::SamplerCreateInfo sampler_create_info {};
	sampler_create_info.minification_filter				= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.magnification_filter			= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.mipmap_mode						= vk2d::SamplerMipmapMode::LINEAR;
	sampler_create_info.address_mode_u					= vk2d::SamplerAddressMode::REPEAT;
	sampler_create_info.address_mode_v					= vk2d::SamplerAddressMode::REPEAT;
	sampler_create_info.border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
	sampler_create_info.mipmap_enable					= true;
	sampler_create_info.mipmap_max_anisotropy			= 16.0f;
	sampler_create_info.mipmap_level_of_detail_bias		= 0.0f;
	sampler_create_info.mipmap_min_level_of_detail		= 0.0f;
	sampler_create_info.mipmap_max_level_of_detail		= 128.0f;
	default_sampler			= std::unique_ptr<vk2d::Sampler>( new vk2d::Sampler( this, sampler_create_info ) );
	if( default_sampler && default_sampler->IsGood() ) {
		return true;
	}

	Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default sampler!" );
	return false;
}









bool vk2d::_internal::RendererImpl::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info {};
	pipeline_cache_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.pNext				= nullptr;
	pipeline_cache_create_info.flags				= 0;
	pipeline_cache_create_info.initialDataSize		= 0;
	pipeline_cache_create_info.pInitialData			= nullptr;

	if( vkCreatePipelineCache(
		device,
		&pipeline_cache_create_info,
		nullptr,
		&pipeline_cache
	) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan pipeline cache!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::RendererImpl::CreateShaderModules()
{

	{
		VkShaderModuleCreateInfo shader_create_info {};
		shader_create_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.pNext		= nullptr;
		shader_create_info.flags		= 0;
		shader_create_info.codeSize		= sizeof( vertex_shader_data );
		shader_create_info.pCode		= vertex_shader_data;

		if( vkCreateShaderModule(
			device,
			&shader_create_info,
			nullptr,
			&vertex_shader_module
		) != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan vertex shader module!" );
			return false;
		}
	}

	{
		VkShaderModuleCreateInfo shader_create_info {};
		shader_create_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.pNext		= nullptr;
		shader_create_info.flags		= 0;
		shader_create_info.codeSize		= sizeof( fragment_shader_data );
		shader_create_info.pCode		= fragment_shader_data;

		if( vkCreateShaderModule(
			device,
			&shader_create_info,
			nullptr,
			&fragment_shader_module
		) != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan fragment shader module!" );
			return false;
		}
	}

	return true;
}









bool vk2d::_internal::RendererImpl::CreateDescriptorSetLayouts()
{
	// These must match shader layout.

	// Set 0 layout, sampler
	{
		std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		descriptor_set_layout_bindings[ 1 ].binding				= 1;
		descriptor_set_layout_bindings[ 1 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_set_layout_bindings[ 1 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 1 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 1 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		sampler_descriptor_set_layout		= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			device,
			&descriptor_set_layout_create_info
		);
		if( !sampler_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return false;
		}
	}

	// Set 0 layout, image
	{
		std::array<VkDescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		texture_descriptor_set_layout		= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			device,
			&descriptor_set_layout_create_info
		);
		if( !texture_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create image descriptor set layout!" );
			return false;
		}
	}

	return true;
}









bool vk2d::_internal::RendererImpl::CreatePipelineLayout()
{
	// This must match shader layout.

	std::vector<VkDescriptorSetLayout> set_layouts {
		sampler_descriptor_set_layout->GetVulkanDescriptorSetLayout(),
		texture_descriptor_set_layout->GetVulkanDescriptorSetLayout()
	};

	std::array<VkPushConstantRange, 1> push_constant_ranges {};
	push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_ranges[ 0 ].offset		= 0;
	push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vk2d::_internal::WindowCoordinateScaling ) );

	VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
	pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pNext					= nullptr;
	pipeline_layout_create_info.flags					= 0;
	pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
	pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
	pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

	if( vkCreatePipelineLayout(
		device,
		&pipeline_layout_create_info,
		nullptr,
		&pipeline_layout
	) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan pipeline layout!" );
		return false;
	}

	return true;
}

bool vk2d::_internal::RendererImpl::CreateDeviceMemoryPool()
{
	device_memory_pool		= MakeDeviceMemoryPool(
		physical_device,
		device
	);
	if( !device_memory_pool ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create memory pool!" );
		return false;
	}
	return true;
}

bool vk2d::_internal::RendererImpl::CreateThreadPool()
{
	uint32_t thread_count					= uint32_t( std::thread::hardware_concurrency() );
	if( thread_count == 0 ) thread_count	= 8;
	--thread_count;

	uint32_t loader_thread_count = create_info_copy.resource_loader_thread_count;
	if( loader_thread_count == UINT32_MAX )		loader_thread_count		= thread_count / 2;
	if( loader_thread_count > thread_count )	loader_thread_count		= thread_count;
	if( loader_thread_count == 0 )				loader_thread_count		= 1;

	uint32_t general_thread_count = 1;

	std::vector<std::unique_ptr<vk2d::_internal::ThreadPrivateResource>> thread_resources;
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<vk2d::_internal::ThreadLoaderResource>( this ) );
	}
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<vk2d::_internal::ThreadGeneralResource>() );
	}

	loader_threads.resize( loader_thread_count );
	general_threads.resize( general_thread_count );
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		loader_threads[ i ]		= i;
	}
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		general_threads[ i ]	= loader_thread_count + i;
	}

	thread_pool				= std::make_unique<vk2d::_internal::ThreadPool>( std::move( thread_resources ) );
	if( thread_pool && thread_pool->IsGood() ) {
		return true;
	} else {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create thread pool!" );
		return false;
	}
}

bool vk2d::_internal::RendererImpl::CreateResourceManager()
{
	resource_manager		= std::make_unique<vk2d::ResourceManager>(
		this
		);
	if( resource_manager && resource_manager->IsGood() ) {
		return true;
	} else {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create resource manager!" );
		return false;
	}
}

bool vk2d::_internal::RendererImpl::CreateDefaultTexture()
{
	{
		// Create texture
		{
			VkImageCreateInfo image_create_info {};
			image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_create_info.pNext						= nullptr;
			image_create_info.flags						= 0;
			image_create_info.imageType					= VK_IMAGE_TYPE_2D;
			image_create_info.format					= VK_FORMAT_R8G8B8A8_UNORM;
			image_create_info.extent					= { 1, 1, 1 };
			image_create_info.mipLevels					= 1;
			image_create_info.arrayLayers				= 1;
			image_create_info.samples					= VK_SAMPLE_COUNT_1_BIT;
			image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
			image_create_info.usage						= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			image_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
			image_create_info.queueFamilyIndexCount		= 0;
			image_create_info.pQueueFamilyIndices		= nullptr;
			image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;

			VkImageViewCreateInfo image_view_create_info {};
			image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.pNext				= nullptr;
			image_view_create_info.flags				= 0;
			image_view_create_info.image				= VK_NULL_HANDLE;
			image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format				= image_create_info.format;
			image_view_create_info.components			= {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			};
			image_view_create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel	= 0;
			image_view_create_info.subresourceRange.levelCount		= 1;
			image_view_create_info.subresourceRange.baseArrayLayer	= 0;
			image_view_create_info.subresourceRange.layerCount		= 1;

			default_texture		= device_memory_pool->CreateCompleteImageResource(
				&image_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&image_view_create_info
			);
			if( default_texture != VK_SUCCESS ) {
				Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
				return false;
			}
		}
		// Clear default texture image
		{
			VkCommandPool		cpool		= VK_NULL_HANDLE;
			VkCommandBuffer		cbuffer		= VK_NULL_HANDLE;
			{
				VkCommandPoolCreateInfo pool_create_info {};
				pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				pool_create_info.pNext				= nullptr;
				pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
				pool_create_info.queueFamilyIndex	= primary_render_queue.GetQueueFamilyIndex();
				if( vkCreateCommandPool(
					device,
					&pool_create_info,
					nullptr,
					&cpool
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}

				VkCommandBufferAllocateInfo command_buffer_allocate_info {};
				command_buffer_allocate_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				command_buffer_allocate_info.pNext					= nullptr;
				command_buffer_allocate_info.commandPool			= cpool;
				command_buffer_allocate_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				command_buffer_allocate_info.commandBufferCount		= 1;
				if( vkAllocateCommandBuffers(
					device,
					&command_buffer_allocate_info,
					&cbuffer
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}
			}
			// Record clear commands
			{
				VkCommandBufferBeginInfo cbuffer_begin_info {};
				cbuffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cbuffer_begin_info.pNext				= nullptr;
				cbuffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				cbuffer_begin_info.pInheritanceInfo		= nullptr;
				if( vkBeginCommandBuffer(
					cbuffer,
					&cbuffer_begin_info
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}

				{
					VkImageMemoryBarrier image_memory_barrier {};
					image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					image_memory_barrier.pNext								= nullptr;
					image_memory_barrier.srcAccessMask						= 0;
					image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
					image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
					image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.image								= default_texture.image;
					image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					image_memory_barrier.subresourceRange.baseMipLevel		= 0;
					image_memory_barrier.subresourceRange.levelCount		= 1;
					image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
					image_memory_barrier.subresourceRange.layerCount		= 1;
					vkCmdPipelineBarrier(
						cbuffer,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &image_memory_barrier
					);
				}

				{
					VkClearColorValue color {};
					color.float32[ 0 ]		= 1.0f;
					color.float32[ 1 ]		= 1.0f;
					color.float32[ 2 ]		= 1.0f;
					color.float32[ 3 ]		= 1.0f;
					VkImageSubresourceRange range {};
					range.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					range.baseMipLevel		= 0;
					range.levelCount		= 1;
					range.baseArrayLayer	= 0;
					range.layerCount		= 1;
					vkCmdClearColorImage(
						cbuffer,
						default_texture.image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						&color,
						1,
						&range
					);
				}

				{
					VkImageMemoryBarrier image_memory_barrier {};
					image_memory_barrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					image_memory_barrier.pNext								= nullptr;
					image_memory_barrier.srcAccessMask						= VK_ACCESS_MEMORY_WRITE_BIT;
					image_memory_barrier.dstAccessMask						= VK_ACCESS_MEMORY_READ_BIT;
					image_memory_barrier.oldLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					image_memory_barrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_memory_barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
					image_memory_barrier.image								= default_texture.image;
					image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
					image_memory_barrier.subresourceRange.baseMipLevel		= 0;
					image_memory_barrier.subresourceRange.levelCount		= 1;
					image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
					image_memory_barrier.subresourceRange.layerCount		= 1;
					vkCmdPipelineBarrier(
						cbuffer,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &image_memory_barrier
					);
				}

				if( vkEndCommandBuffer(
					cbuffer
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}
			}
			// Submit and wait for command buffer completion
			{
				VkFence fence	= VK_NULL_HANDLE;
				{
					VkFenceCreateInfo fence_create_info {};
					fence_create_info.sType			= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
					fence_create_info.pNext			= nullptr;
					fence_create_info.flags			= 0;
					if( vkCreateFence(
						device,
						&fence_create_info,
						nullptr,
						&fence
					) != VK_SUCCESS ) {
						Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
						return false;
					}
				}
				VkSubmitInfo submit_info {};
				submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.pNext					= nullptr;
				submit_info.waitSemaphoreCount		= 0;
				submit_info.pWaitSemaphores			= nullptr;
				submit_info.pWaitDstStageMask		= nullptr;
				submit_info.commandBufferCount		= 1;
				submit_info.pCommandBuffers			= &cbuffer;
				submit_info.signalSemaphoreCount	= 0;
				submit_info.pSignalSemaphores		= nullptr;
				if( primary_render_queue.Submit(
					submit_info,
					fence
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}

				if( vkWaitForFences(
					device,
					1, &fence,
					VK_TRUE,
					UINT64_MAX
				) != VK_SUCCESS ) {
					Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default texture!" );
					return false;
				}

				vkDestroyFence(
					device,
					fence,
					nullptr
				);
			}
			// Cleanup.
			{
				vkDestroyCommandPool(
					device,
					cpool,
					nullptr
				);
			}
		}
	}
	// Allocate and update default texture descriptor set.
	{
		default_texture_descriptor_set	= descriptor_pool->AllocateDescriptorSet( *texture_descriptor_set_layout );

		VkDescriptorImageInfo descriptor_image_info {};
		descriptor_image_info.sampler			= VK_NULL_HANDLE;
		descriptor_image_info.imageView			= default_texture.view;
		descriptor_image_info.imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		std::array<VkWriteDescriptorSet, 1> descriptor_writes {};
		descriptor_writes[ 0 ].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[ 0 ].pNext			= nullptr;
		descriptor_writes[ 0 ].dstSet			= default_texture_descriptor_set.descriptorSet;
		descriptor_writes[ 0 ].dstBinding		= 0;
		descriptor_writes[ 0 ].dstArrayElement	= 0;
		descriptor_writes[ 0 ].descriptorCount	= 1;
		descriptor_writes[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptor_writes[ 0 ].pImageInfo		= &descriptor_image_info;
		descriptor_writes[ 0 ].pBufferInfo		= nullptr;
		descriptor_writes[ 0 ].pTexelBufferView	= nullptr;

		vkUpdateDescriptorSets(
			device,
			uint32_t( descriptor_writes.size() ),
			descriptor_writes.data(),
			0, nullptr
		);
	}

	return true;
}



void vk2d::_internal::RendererImpl::DestroyInstance()
{
	if( debug_utils_messenger ) {
		auto destroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if( destroyDebugUtilsMessenger ) {
			destroyDebugUtilsMessenger(
				instance,
				debug_utils_messenger,
				nullptr
			);
		}

		debug_utils_messenger	= {};
	}

	vkDestroyInstance(
		instance,
		nullptr
	);

	instance				= {};
}

void vk2d::_internal::RendererImpl::DestroyDevice()
{
	vkDestroyDevice(
		device,
		nullptr
	);

	device					= {};
}

void vk2d::_internal::RendererImpl::DestroyDescriptorPool()
{
	descriptor_pool			= nullptr;
}

void vk2d::_internal::RendererImpl::DestroyDefaultSampler()
{
	default_sampler			= {};
}

void vk2d::_internal::RendererImpl::DestroyPipelineCache()
{
	vkDestroyPipelineCache(
		device,
		pipeline_cache,
		nullptr
	);

	pipeline_cache			= {};
}

void vk2d::_internal::RendererImpl::DestroyShaderModules()
{
	vkDestroyShaderModule(
		device,
		vertex_shader_module,
		nullptr
	);
	vkDestroyShaderModule(
		device,
		fragment_shader_module,
		nullptr
	);

	vertex_shader_module		= {};
	fragment_shader_module		= {};
}

void vk2d::_internal::RendererImpl::DestroyDescriptorSetLayouts()
{
	sampler_descriptor_set_layout	= nullptr;
	texture_descriptor_set_layout	= nullptr;
}

void vk2d::_internal::RendererImpl::DestroyPipelineLayout()
{
	vkDestroyPipelineLayout(
		device,
		pipeline_layout,
		nullptr
	);

	pipeline_layout			= {};
}

void vk2d::_internal::RendererImpl::DestroyDeviceMemoryPool()
{
	device_memory_pool		= {};
}

void vk2d::_internal::RendererImpl::DestroyThreadPool()
{
	thread_pool				= {};
}

void vk2d::_internal::RendererImpl::DestroyResourceManager()
{
	resource_manager		= {};
}

void vk2d::_internal::RendererImpl::DestroyDefaultTexture()
{
	descriptor_pool->FreeDescriptorSet( default_texture_descriptor_set );
	device_memory_pool->FreeCompleteResource( default_texture );

	default_texture						= {};
	default_texture_descriptor_set		= {};
}









std::vector<VkPhysicalDevice> vk2d::_internal::RendererImpl::EnumeratePhysicalDevices()
{
	uint32_t physical_device_count		= UINT32_MAX;
	if( vkEnumeratePhysicalDevices( instance, &physical_device_count, nullptr ) == VK_SUCCESS ) {
		std::vector<VkPhysicalDevice> physical_devices( physical_device_count );
		if( vkEnumeratePhysicalDevices( instance, &physical_device_count, physical_devices.data() ) == VK_SUCCESS ) {
			return physical_devices;
		}
	}
	Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot enumerate physical devices!" );
	return {};
}









VkPhysicalDevice vk2d::_internal::RendererImpl::PickBestPhysicalDevice()
{
	auto physicalDevices = EnumeratePhysicalDevices();

	std::vector<uint64_t> scores( physicalDevices.size() );
	for( size_t i=0; i < physicalDevices.size(); ++i ) {
		auto pd = physicalDevices[ i ];
		auto & s = scores[ i ];
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties( pd, &properties );
		s += 1000; // some intial score
		s += uint64_t( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) * 16000;
		s += uint64_t( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ) * 5000;
		s += uint64_t( properties.limits.maxImageDimension2D );
		s += uint64_t( properties.limits.maxPerStageDescriptorUniformBuffers ) * 20;
		s += uint64_t( properties.limits.maxPerStageDescriptorSampledImages ) * 40;
		s += uint64_t( properties.limits.maxVertexInputBindings ) * 10;
		s += uint64_t( properties.limits.maxComputeWorkGroupCount[ 0 ] );
		s += uint64_t( properties.limits.maxComputeWorkGroupCount[ 1 ] );
		s += uint64_t( properties.limits.maxComputeWorkGroupCount[ 2 ] );
		s += uint64_t( properties.limits.maxComputeWorkGroupInvocations );
		s += uint64_t( properties.limits.maxSamplerAnisotropy ) * 200;

		// Check if physical device can present
		bool		physicalDeviceCanPresent	= false;
		uint32_t	queueFamilyCount			= 0;
		vkGetPhysicalDeviceQueueFamilyProperties( pd, &queueFamilyCount, nullptr );
		for( uint32_t i=0; i < queueFamilyCount; ++i ) {
			if( glfwGetPhysicalDevicePresentationSupport( instance, pd, i ) ) {
				physicalDeviceCanPresent		= true;
				break;
			}
		}
		// If the physical device cannot present anything we won't even consider it
		if( !physicalDeviceCanPresent ) {
			s = 0;
		}
	}

	VkPhysicalDevice best_physical_device	= VK_NULL_HANDLE;
	uint64_t best_score_so_far				= 0;
	for( size_t i=0; i < physicalDevices.size(); ++i ) {
		if( scores[ i ] > best_score_so_far ) {
			best_score_so_far		= scores[ i ];
			best_physical_device	= physicalDevices[ i ];
		}
	}

	if( !best_physical_device ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot find physical device capable of presenting to any surface!" );
	}

	return best_physical_device;
}
