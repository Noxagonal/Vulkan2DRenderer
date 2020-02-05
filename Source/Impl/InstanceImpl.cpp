
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/InstanceImpl.h"
#include "../../Include/Interface/ResourceManager.h"
#include "../Header/Impl/ResourceManagerImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Core/QueueResolver.h"
#include "../Header/Core/ThreadPool.h"
#include "../Header/Core/ThreadPrivateResources.h"
#include "../Header/Core/DescriptorSet.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Sampler.h"
#include "../../Include/Interface/TextureResource.h"

#include "../Shaders/Spir-V/IncludeAllShaders.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include <sstream>
#include <iostream>
#include <vector>
#include <list>


namespace vk2d {

namespace _internal {



void VK2D_APIENTRY VK2D_default_ReportFunction(
	vk2d::ReportSeverity			severity,
	std::string						message
)
{
	switch( severity ) {
	case vk2d::ReportSeverity::NONE:
		vk2d::SetConsoleColor();
		break;
	case vk2d::ReportSeverity::VERBOSE:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::DARK_GRAY );
		break;
	case vk2d::ReportSeverity::INFO:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::DARK_GREEN );
		break;
	case vk2d::ReportSeverity::PERFORMANCE_WARNING:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::BLUE );
		break;
	case vk2d::ReportSeverity::WARNING:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::DARK_YELLOW );
		break;
	case vk2d::ReportSeverity::NON_CRITICAL_ERROR:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::RED );
		break;
	case vk2d::ReportSeverity::CRITICAL_ERROR:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::WHITE, vk2d::ConsoleColor::DARK_RED );
		break;
	default:
		vk2d::SetConsoleColor();
		break;
	}

	std::cout << message << "\n";
	vk2d::SetConsoleColor();

	if( severity == vk2d::ReportSeverity::CRITICAL_ERROR ) {

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS
		auto checkpoints = vk2d::_internal::GetCommandBufferCheckpoints();
		if( checkpoints.size() ) {
			std::cout << "\n\nLatest command buffer checkpoint marker stages: " << checkpoints.size() << "\n";
			for( auto & c : checkpoints ) {
				auto data = reinterpret_cast<vk2d::_internal::CommandBufferCheckpointData*>( c.pCheckpointMarker );
				std::stringstream ss;
				ss << vk2d::_internal::CommandBufferCheckpointTypeToString( data->type )
					<< " : " << data->name
					<< vk2d::_internal::VkPipelineStageFlagBitsToString( c.stage );
				std::cout << ss.str() << "\n";
			}
			std::cout << "\nCommand buffer checkmark trail:\n";
			auto c = reinterpret_cast<CommandBufferCheckpointData*>( checkpoints[ 0 ].pCheckpointMarker )->previous;
			while( c ) {
				auto current = c;
				auto previous= current->previous;
				c = previous;

				std::stringstream ss;
				ss << vk2d::_internal::CommandBufferCheckpointTypeToString( current->type )
					<< " : " << current->name << "\n";
				std::cout << ss.str();

				delete current;
			}
			delete reinterpret_cast<CommandBufferCheckpointData*>( checkpoints[ 0 ].pCheckpointMarker );
		}
#endif

#ifdef _WIN32
		MessageBox( NULL, message.c_str(), "Critical error!", MB_OK | MB_ICONERROR );
#endif

		std::exit( -1 );
	}
}



vk2d::Monitor									primary_monitor				= {};
std::vector<vk2d::Monitor>						all_monitors				= {};
std::list<vk2d::_internal::InstanceImpl*>		instance_listeners			= {};

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
	for( auto l : vk2d::_internal::instance_listeners ) {
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

		for( auto l : vk2d::_internal::instance_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( vk2d::Gamepad( joystick ), vk2d::GamepadEvent::CONNECTED, joystic_name );
			}
		}
	} else {
		for( auto l : vk2d::_internal::instance_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( vk2d::Gamepad( joystick ), vk2d::GamepadEvent::DISCONNECTED, std::string( "" ) );
			}
		}
	}
}

} // _internal

} // vk2d








uint64_t vk2d::_internal::InstanceImpl::instance_count				= 0;

vk2d::_internal::InstanceImpl::InstanceImpl( const InstanceCreateInfo & instance_create_info )
{
	vk2d::_internal::instance_listeners.push_back( this );

	// Initialize glfw if this is the first instance.
	if( instance_count == 0 ) {
		if( glfwInit() != GLFW_TRUE ) {
			if( instance_create_info.report_function ) {
				instance_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot initialize GLFW!" );
			}
			return;
		}
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwSetMonitorCallback( vk2d::_internal::glfwMonitorCallback );
		glfwSetJoystickCallback( vk2d::_internal::glfwJoystickEventCallback );
		vk2d::_internal::UpdateMonitorLists();
	}
	++instance_count;

	// Check if vulkan is supported
	if( glfwVulkanSupported() == GLFW_FALSE ) {
		if( instance_create_info.report_function ) {
			instance_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Vulkan is not supported on this machine!\nUpdating your graphics drivers might fix this issue!" );
		}
		return;
	}

	create_info_copy	= instance_create_info;
	report_function		= create_info_copy.report_function;

#if VK2D_BUILD_OPTION_DEBUG_ENABLE
	if( report_function == nullptr ) {
		report_function = VK2D_default_ReportFunction;
	}
#endif

//	Introduce layers and extensions here
//	instance_layers;
//	Instance extensions.
	instance_extensions.push_back( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
	{
		// Adding glfw instance extensions
		uint32_t glfw_instance_extension_count = 0;
		auto glfw_instance_extensions_ptr = glfwGetRequiredInstanceExtensions( &glfw_instance_extension_count );
		for( uint32_t i = 0; i < glfw_instance_extension_count; ++i ) {
			instance_extensions.push_back( glfw_instance_extensions_ptr[ i ] );
		}
	}

	// Device extensions.
	device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	device_extensions.push_back( VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME );

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS
	device_extensions.push_back( VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME );
#endif

	if( !CreateInstance() ) return;
	if( !PickPhysicalDevice() ) return;
	if( !CreateDeviceAndQueues() ) return;

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS
	if( instance_count == 1 ) {
		vk2d::_internal::SetCommandBufferCheckpointHost( this );
	}
#endif

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

vk2d::_internal::InstanceImpl::~InstanceImpl()
{
	vkDeviceWaitIdle( vk_device );

	windows.clear();
	cursors.clear();
	samplers.clear();

	DestroyDefaultSampler();
	DestroyDefaultTexture();
	DestroyResourceManager();
	DestroyThreadPool();
	DestroyDeviceMemoryPool();
	DestroyPipelines();
	DestroyPipelineLayout();
	DestroyShaderModules();
	DestroyPipelineCache();
	DestroyDescriptorPool();
	DestroyDescriptorSetLayouts();
	DestroyDevice();
	DestroyInstance();

	// If this is the last instance exiting, also terminate glfw
	--instance_count;
	if( instance_count == 0 ) {
		glfwTerminate();
	}

	vk2d::_internal::instance_listeners.remove( this );
}

std::vector<vk2d::Monitor*> vk2d::_internal::InstanceImpl::GetMonitors()
{
	std::vector<vk2d::Monitor*> ret;
	ret.reserve( vk2d::_internal::all_monitors.size() );

	for( auto & m : vk2d::_internal::all_monitors ) {
		ret.push_back( &m );
	}

	return ret;
}

vk2d::Monitor * vk2d::_internal::InstanceImpl::GetPrimaryMonitor()
{
	return &vk2d::_internal::primary_monitor;
}

void vk2d::_internal::InstanceImpl::SetMonitorUpdateCallback(
	vk2d::MonitorUpdateCallbackFun		monitor_update_callback_funtion
)
{
	monitor_update_callback		= monitor_update_callback_funtion;
}

vk2d::Cursor * vk2d::_internal::InstanceImpl::CreateCursor(
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	auto cursor = std::unique_ptr<vk2d::Cursor>( new vk2d::Cursor(
		this,
		image_path,
		hot_spot
	) );
	if( cursor && cursor->IsGood() ) {
		auto ret = cursor.get();
		cursors.push_back( std::move( cursor ) );
		return ret;
	} else {
		Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
	}
	return {};
}

vk2d::Cursor * vk2d::_internal::InstanceImpl::CreateCursor(
	vk2d::Vector2u							image_size,
	const std::vector<vk2d::Color8>		&	image_data,
	vk2d::Vector2i							hot_spot
)
{
	auto cursor = std::unique_ptr<vk2d::Cursor>( new vk2d::Cursor(
		this,
		image_size,
		image_data,
		hot_spot
	) );
	if( cursor && cursor->IsGood() ) {
		auto ret = cursor.get();
		cursors.push_back( std::move( cursor ) );
		return ret;
	} else {
		Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
	}
	return {};
}

void vk2d::_internal::InstanceImpl::DestroyCursor(
	vk2d::Cursor		*	cursor
)
{
	auto it = cursors.begin();
	while( it != cursors.end() ) {
		if( it->get() == cursor ) {
			it = cursors.erase( it );
			return;
		} else {
			++it;
		}
	}
}

vk2d::GamepadEventCallbackFun vk2d::_internal::InstanceImpl::GetGamepadEventCallback()
{
	return vk2d::GamepadEventCallbackFun();
}

void vk2d::_internal::InstanceImpl::SetGamepadEventCallback(
	vk2d::GamepadEventCallbackFun		joystick_event_callback_function
)
{
	joystick_event_callback		= joystick_event_callback_function;
}

bool vk2d::_internal::InstanceImpl::IsGamepadPresent( vk2d::Gamepad joystick )
{
	if( glfwJoystickIsGamepad( int( joystick ) ) == GLFW_TRUE ) {
		return true;
	} else {
		return false;
	}
}

std::string vk2d::_internal::InstanceImpl::GetGamepadName( vk2d::Gamepad gamepad )
{
	return glfwGetGamepadName( int( gamepad ) );
}

vk2d::GamepadState vk2d::_internal::InstanceImpl::QueryGamepadState(
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



vk2d::Window * vk2d::_internal::InstanceImpl::CreateOutputWindow(
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

void vk2d::_internal::InstanceImpl::CloseOutputWindow(
	vk2d::Window				*	window
)
{
	vkDeviceWaitIdle( vk_device );

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



vk2d::_internal::DescriptorAutoPool * vk2d::_internal::InstanceImpl::GetDescriptorPool()
{
	return descriptor_pool.get();
}



vk2d::Sampler * vk2d::_internal::InstanceImpl::CreateSampler(
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

void vk2d::_internal::InstanceImpl::DestroySampler(
	vk2d::Sampler					*	sampler
)
{
	if( vkDeviceWaitIdle( vk_device ) != VK_SUCCESS ) {
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

vk2d::Multisamples vk2d::_internal::InstanceImpl::GetMaximumSupportedMultisampling()
{
	vk2d::Multisamples max_samples	= vk2d::Multisamples( vk_physical_device_properties.limits.framebufferColorSampleCounts );
	if( uint32_t( max_samples )			& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_64 ) )	return vk2d::Multisamples::SAMPLE_COUNT_64;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_32 ) )	return vk2d::Multisamples::SAMPLE_COUNT_32;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_16 ) )	return vk2d::Multisamples::SAMPLE_COUNT_16;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_8 ) )	return vk2d::Multisamples::SAMPLE_COUNT_8;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_4 ) )	return vk2d::Multisamples::SAMPLE_COUNT_4;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_2 ) )	return vk2d::Multisamples::SAMPLE_COUNT_2;
	else if( uint32_t( max_samples )	& uint32_t( vk2d::Multisamples::SAMPLE_COUNT_1 ) )	return vk2d::Multisamples::SAMPLE_COUNT_1;
	return vk2d::Multisamples::SAMPLE_COUNT_1;
}

vk2d::Multisamples vk2d::_internal::InstanceImpl::GetAllSupportedMultisampling()
{
	return vk2d::Multisamples( vk_physical_device_properties.limits.framebufferColorSampleCounts );
}









vk2d::PFN_VK2D_ReportFunction vk2d::_internal::InstanceImpl::GetReportFunction() const
{
	return report_function;
}

void vk2d::_internal::InstanceImpl::Report(
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

vk2d::_internal::ThreadPool * vk2d::_internal::InstanceImpl::GetThreadPool() const
{
	return thread_pool.get();
}

const std::vector<uint32_t> & vk2d::_internal::InstanceImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & vk2d::_internal::InstanceImpl::GetGeneralThreads() const
{
	return general_threads;
}

vk2d::ResourceManager * vk2d::_internal::InstanceImpl::GetResourceManager() const
{
	return resource_manager.get();
}

VkInstance vk2d::_internal::InstanceImpl::GetVulkanInstance() const
{
	return vk_instance;
}

VkPhysicalDevice vk2d::_internal::InstanceImpl::GetVulkanPhysicalDevice() const
{
	return vk_physical_device;
}

VkDevice vk2d::_internal::InstanceImpl::GetVulkanDevice() const
{
	return vk_device;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::InstanceImpl::GetPrimaryRenderQueue() const
{
	return primary_render_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::InstanceImpl::GetSecondaryRenderQueue() const
{
	return secondary_render_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::InstanceImpl::GetPrimaryComputeQueue() const
{
	return primary_compute_queue;
}

vk2d::_internal::ResolvedQueue vk2d::_internal::InstanceImpl::GetPrimaryTransferQueue() const
{
	return primary_transfer_queue;
}

const VkPhysicalDeviceProperties & vk2d::_internal::InstanceImpl::GetVulkanPhysicalDeviceProperties() const
{
	return vk_physical_device_properties;
}

const VkPhysicalDeviceMemoryProperties & vk2d::_internal::InstanceImpl::GetVulkanPhysicalDeviceMemoryProperties() const
{
	return vk_physical_device_memory_properties;
}

const VkPhysicalDeviceFeatures & vk2d::_internal::InstanceImpl::GetVulkanPhysicalDeviceFeatures() const
{
	return vk_physical_device_features;
}

vk2d::_internal::ShaderProgram vk2d::_internal::InstanceImpl::GetShaderModules(
	vk2d::_internal::ShaderProgramID			id ) const
{
	auto collection = shader_programs.find( id );
	if( collection != shader_programs.end() ) {
		return collection->second;
	}
	return {};
}

vk2d::_internal::ShaderProgram vk2d::_internal::InstanceImpl::GetCompatibleShaderModules(
	bool				multitextured,
	bool				custom_uv_border_color,
	uint32_t			vertices_per_primitive
)
{
	if( multitextured ) {
		if( custom_uv_border_color ) {
			if( vertices_per_primitive == 1 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_POINT_UV_BORDER_COLOR );
			}
			if( vertices_per_primitive == 2 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_LINE_UV_BORDER_COLOR );
			}
			if( vertices_per_primitive == 3 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR );
			}
		} else {
			if( vertices_per_primitive == 1 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_POINT );
			}
			if( vertices_per_primitive == 2 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_LINE );
			}
			if( vertices_per_primitive == 3 ) {
				return GetShaderModules( vk2d::_internal::ShaderProgramID::MULTITEXTURED_TRIANGLE );
			}
		}
	} else {
		if( custom_uv_border_color ) {
			return GetShaderModules( vk2d::_internal::ShaderProgramID::SINGLE_TEXTURED_UV_BORDER_COLOR );
		} else {
			return GetShaderModules( vk2d::_internal::ShaderProgramID::SINGLE_TEXTURED );
		}
	}

	return {};
}

VkPipeline vk2d::_internal::InstanceImpl::GetVulkanPipeline(
	const vk2d::_internal::PipelineSettings		&	settings
)
{
	auto p_it = vk_pipelines.find( settings );
	if( p_it != vk_pipelines.end() ) {
		return p_it->second;
	}
	return CreateVulkanPipeline( settings );
}

VkPipeline vk2d::_internal::InstanceImpl::CreateVulkanPipeline(
	const vk2d::_internal::PipelineSettings		&	settings
)
{
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos {};
	shader_stage_create_infos[ 0 ].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 0 ].pNext				= nullptr;
	shader_stage_create_infos[ 0 ].flags				= 0;
	shader_stage_create_infos[ 0 ].stage				= VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_create_infos[ 0 ].module				= settings.shader_programs.vertex;
	shader_stage_create_infos[ 0 ].pName				= "main";
	shader_stage_create_infos[ 0 ].pSpecializationInfo	= nullptr;

	shader_stage_create_infos[ 1 ].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 1 ].pNext				= nullptr;
	shader_stage_create_infos[ 1 ].flags				= 0;
	shader_stage_create_infos[ 1 ].stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_create_infos[ 1 ].module				= settings.shader_programs.fragment;
	shader_stage_create_infos[ 1 ].pName				= "main";
	shader_stage_create_infos[ 1 ].pSpecializationInfo	= nullptr;

	// Make sure this matches Vertex in RenderPrimitives.h
	std::array<VkVertexInputBindingDescription, 0> vertex_input_binding_descriptions {};
	//	vertex_input_binding_descriptions[ 0 ].binding		= 0;
	//	vertex_input_binding_descriptions[ 0 ].stride		= sizeof( vk2d::Vertex );
	//	vertex_input_binding_descriptions[ 0 ].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 0> vertex_input_attribute_descriptions {};
	//	vertex_input_attribute_descriptions[ 0 ].location	= 0;
	//	vertex_input_attribute_descriptions[ 0 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 0 ].format		= VK_FORMAT_R32G32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 0 ].offset		= offsetof( vk2d::Vertex, vertex_coords );
	//
	//	vertex_input_attribute_descriptions[ 1 ].location	= 1;
	//	vertex_input_attribute_descriptions[ 1 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 1 ].format		= VK_FORMAT_R32G32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 1 ].offset		= offsetof( vk2d::Vertex, uv_coords );
	//
	//	vertex_input_attribute_descriptions[ 2 ].location	= 2;
	//	vertex_input_attribute_descriptions[ 2 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 2 ].format		= VK_FORMAT_R32G32B32A32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 2 ].offset		= offsetof( vk2d::Vertex, color );
	//
	//	vertex_input_attribute_descriptions[ 3 ].location	= 3;
	//	vertex_input_attribute_descriptions[ 3 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 3 ].format		= VK_FORMAT_R32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 3 ].offset		= offsetof( vk2d::Vertex, point_size );

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
	vertex_input_state_create_info.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.pNext							= nullptr;
	vertex_input_state_create_info.flags							= 0;
	vertex_input_state_create_info.vertexBindingDescriptionCount	= uint32_t( vertex_input_binding_descriptions.size() );
	vertex_input_state_create_info.pVertexBindingDescriptions		= vertex_input_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount	= uint32_t( vertex_input_attribute_descriptions.size() );
	vertex_input_state_create_info.pVertexAttributeDescriptions		= vertex_input_attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {};
	input_assembly_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.pNext						= nullptr;
	input_assembly_state_create_info.flags						= 0;
	input_assembly_state_create_info.topology					= settings.primitive_topology;
	input_assembly_state_create_info.primitiveRestartEnable		= VK_FALSE;

	VkViewport viewport {};
	viewport.x			= 0;
	viewport.y			= 0;
	viewport.width		= 512;
	viewport.height		= 512;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor {
		{ 0, 0 },
		{ 512, 512 }
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info {};
	viewport_state_create_info.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.pNext			= nullptr;
	viewport_state_create_info.flags			= 0;
	viewport_state_create_info.viewportCount	= 1;
	viewport_state_create_info.pViewports		= &viewport;
	viewport_state_create_info.scissorCount		= 1;
	viewport_state_create_info.pScissors		= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
	rasterization_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.pNext						= nullptr;
	rasterization_state_create_info.flags						= 0;
	rasterization_state_create_info.depthClampEnable			= VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable		= VK_FALSE;
	rasterization_state_create_info.polygonMode					= settings.polygon_mode;
	rasterization_state_create_info.cullMode					= VK_CULL_MODE_NONE;
	rasterization_state_create_info.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable				= VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor		= 0.0f;
	rasterization_state_create_info.depthBiasClamp				= 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor		= 0.0f;
	rasterization_state_create_info.lineWidth					= 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
	multisample_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.pNext						= nullptr;
	multisample_state_create_info.flags						= 0;
	multisample_state_create_info.rasterizationSamples		= VkSampleCountFlagBits( settings.samples );
	multisample_state_create_info.sampleShadingEnable		= VK_FALSE;
	multisample_state_create_info.minSampleShading			= 1.0f;
	multisample_state_create_info.pSampleMask				= nullptr;
	multisample_state_create_info.alphaToCoverageEnable		= VK_FALSE;
	multisample_state_create_info.alphaToOneEnable			= VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info {};
	depth_stencil_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.pNext					= nullptr;
	depth_stencil_state_create_info.flags					= 0;
	depth_stencil_state_create_info.depthTestEnable			= VK_FALSE;
	depth_stencil_state_create_info.depthWriteEnable		= VK_FALSE;
	depth_stencil_state_create_info.depthCompareOp			= VK_COMPARE_OP_NEVER;
	depth_stencil_state_create_info.depthBoundsTestEnable	= VK_FALSE;
	depth_stencil_state_create_info.stencilTestEnable		= VK_FALSE;
	depth_stencil_state_create_info.front.failOp			= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.passOp			= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.depthFailOp		= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.compareOp			= VK_COMPARE_OP_NEVER;
	depth_stencil_state_create_info.front.compareMask		= 0;
	depth_stencil_state_create_info.front.writeMask			= 0;
	depth_stencil_state_create_info.front.reference			= 0;
	depth_stencil_state_create_info.back					= depth_stencil_state_create_info.front;
	depth_stencil_state_create_info.minDepthBounds			= 0.0f;
	depth_stencil_state_create_info.maxDepthBounds			= 1.0f;

	std::array<VkPipelineColorBlendAttachmentState, 1> color_blend_attachment_states {};
	color_blend_attachment_states[ 0 ].blendEnable			= VK_TRUE;
	color_blend_attachment_states[ 0 ].srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].colorBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].srcAlphaBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].alphaBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].colorWriteMask		=
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
	color_blend_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.pNext					= nullptr;
	color_blend_state_create_info.flags					= 0;
	color_blend_state_create_info.logicOpEnable			= VK_FALSE;
	color_blend_state_create_info.logicOp				= VK_LOGIC_OP_CLEAR;
	color_blend_state_create_info.attachmentCount		= uint32_t( color_blend_attachment_states.size() );
	color_blend_state_create_info.pAttachments			= color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[ 0 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 1 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 2 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 3 ]	= 0.0f;

	std::vector<VkDynamicState> dynamic_states {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
	dynamic_state_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pNext				= nullptr;
	dynamic_state_create_info.flags				= 0;
	dynamic_state_create_info.dynamicStateCount	= uint32_t( dynamic_states.size() );
	dynamic_state_create_info.pDynamicStates	= dynamic_states.data();

	VkGraphicsPipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.pNext					= nullptr;
	pipeline_create_info.flags					= 0;
	pipeline_create_info.stageCount				= uint32_t( shader_stage_create_infos.size() );
	pipeline_create_info.pStages				= shader_stage_create_infos.data();
	pipeline_create_info.pVertexInputState		= &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState	= &input_assembly_state_create_info;
	pipeline_create_info.pTessellationState		= nullptr;
	pipeline_create_info.pViewportState			= &viewport_state_create_info;
	pipeline_create_info.pRasterizationState	= &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState		= &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState		= &depth_stencil_state_create_info;
	pipeline_create_info.pColorBlendState		= &color_blend_state_create_info;
	pipeline_create_info.pDynamicState			= &dynamic_state_create_info;
	pipeline_create_info.layout					= GetVulkanPipelineLayout();
	pipeline_create_info.renderPass				= settings.render_pass;
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	VkPipeline pipeline {};
	if( vkCreateGraphicsPipelines(
		vk_device,
		GetVulkanPipelineCache(),
		1,
		&pipeline_create_info,
		nullptr,
		&pipeline
	) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan graphics pipeline!" );
		return {};
	}

	vk_pipelines[ settings ] = pipeline;
	return pipeline;
}

VkPipelineCache vk2d::_internal::InstanceImpl::GetVulkanPipelineCache() const
{
	return vk_pipeline_cache;
}

VkPipelineLayout vk2d::_internal::InstanceImpl::GetVulkanPipelineLayout() const
{
	return vk_pipeline_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetSamplerTextureDescriptorSetLayout() const
{
	return *sampler_texture_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetUniformBufferDescriptorSetLayout() const
{
	return *uniform_buffer_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetStorageBufferDescriptorSetLayout() const
{
	return *storage_buffer_descriptor_set_layout;
}

vk2d::TextureResource * vk2d::_internal::InstanceImpl::GetDefaultTexture() const
{
	return default_texture;
}

vk2d::Sampler * vk2d::_internal::InstanceImpl::GetDefaultSampler() const
{
	return default_sampler.get();
}

vk2d::_internal::DeviceMemoryPool * vk2d::_internal::InstanceImpl::GetDeviceMemoryPool() const
{
	return device_memory_pool.get();
}

bool vk2d::_internal::InstanceImpl::IsGood() const
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
	vk2d::ReportSeverity vk2d_severity;
	switch( messageSeverity ) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		vk2d_severity	= vk2d::ReportSeverity::VERBOSE;
		str_severity	= "VERBOSE";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		vk2d_severity	= vk2d::ReportSeverity::INFO;
		str_severity	= "INFO";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		vk2d_severity	= vk2d::ReportSeverity::WARNING;
		str_severity	= "WARNING";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		vk2d_severity	= vk2d::ReportSeverity::NON_CRITICAL_ERROR;
		str_severity	= "ERROR";
		break;
	default:
		vk2d_severity	= vk2d::ReportSeverity::INFO;
		str_severity	= "<UNKNOWN>";
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
		str_type = "<UNKNOWN>";
		assert( 0 );
		break;
	}

	std::stringstream ss_title;
	ss_title << "Vulkan Validation: " << str_type << " " << str_severity;

	std::stringstream ss_message;
	ss_message << ss_title.str() << ":\n - " << pCallbackData->pMessage << "\n";
	// TODO: labels, object, message id name / number;

	auto instance = reinterpret_cast<vk2d::_internal::InstanceImpl*>( pUserData );
	assert( instance );

	instance->Report( vk2d_severity, ss_message.str() );

	return VK_FALSE;
}

} // _internal

} // vk2d

#endif









bool vk2d::_internal::InstanceImpl::CreateInstance()
{

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
	instance_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
	instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	VkDebugUtilsMessageSeverityFlagsEXT severity_flags {};
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_VERBOSE
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_INFO
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_WARNING
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_SEVERITY_ERROR
	severity_flags		|= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#endif

	std::vector<VkValidationFeatureEnableEXT> enabled_validation_features {
#if VK2D_BUILD_OPTION_VULKAN_GPU_ASSISTED_VALIDATION
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
#endif
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_BEST_PRACTICES
		VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
#endif
	};
	std::vector<VkValidationFeatureDisableEXT> disabled_validation_features {
	};

	VkValidationFeaturesEXT validation_features {};
	validation_features.sType							= VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validation_features.pNext							= nullptr;
	validation_features.enabledValidationFeatureCount	= uint32_t( enabled_validation_features.size() );
	validation_features.pEnabledValidationFeatures		= enabled_validation_features.data();
	validation_features.disabledValidationFeatureCount	= uint32_t( disabled_validation_features.size() );
	validation_features.pDisabledValidationFeatures		= disabled_validation_features.data();

	VkDebugUtilsMessengerCreateInfoEXT			debug_utils_create_info {};
	debug_utils_create_info.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_utils_create_info.pNext				= &validation_features;
	debug_utils_create_info.flags				= 0;
	debug_utils_create_info.messageSeverity		= severity_flags;
	debug_utils_create_info.messageType			=
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_utils_create_info.pfnUserCallback		= vk2d::_internal::DebugMessenger;
	debug_utils_create_info.pUserData			= this;
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
			&vk_instance
		);
		if( result != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create vulkan instance!" );
			return false;
		}
	}

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
	{
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( vk_instance, "vkCreateDebugUtilsMessengerEXT" );
		if( !createDebugUtilsMessenger ) {
			Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return false;
		}
		auto result = createDebugUtilsMessenger(
			vk_instance,
			&debug_utils_create_info,
			nullptr,
			&vk_debug_utils_messenger
		);
		if( result != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return false;
		}
	}
#endif

	return true;
}









bool vk2d::_internal::InstanceImpl::PickPhysicalDevice()
{
	vk_physical_device	= PickBestVulkanPhysicalDevice();
	if( !vk_physical_device ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot find physical device!" );
	}

	if( vk_physical_device ) {
		vkGetPhysicalDeviceProperties(
			vk_physical_device,
			&vk_physical_device_properties
		);
		vkGetPhysicalDeviceMemoryProperties(
			vk_physical_device,
			&vk_physical_device_memory_properties
		);
		vkGetPhysicalDeviceFeatures(
			vk_physical_device,
			&vk_physical_device_features
		);

		return true;
	}
	return false;
}









bool vk2d::_internal::InstanceImpl::CreateDeviceAndQueues()
{
	std::vector<std::pair<VkQueueFlags, float>> queue_requests {
		{ VK_QUEUE_GRAPHICS_BIT, 1.0f },
		{ VK_QUEUE_GRAPHICS_BIT, 0.2f },
		{ VK_QUEUE_COMPUTE_BIT, 0.9f },
		{ VK_QUEUE_TRANSFER_BIT, 0.5f }
	};
	DeviceQueueResolver queue_resolver( vk_instance, vk_physical_device, queue_requests );
	if( !queue_resolver.IsGood() ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create queue resolver!" );
		return false;
	}
	auto queue_create_infos = queue_resolver.GetDeviceQueueCreateInfos();

	VkPhysicalDeviceFeatures features {};
	features.samplerAnisotropy					= VK_TRUE;
	features.fillModeNonSolid					= VK_TRUE;
	features.wideLines							= VK_TRUE;
	features.geometryShader						= VK_TRUE;
//	features.fragmentStoresAndAtomics			= VK_TRUE;

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
		vk_physical_device,
		&device_create_info,
		nullptr,
		&vk_device
	);
	if( result != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan device!" );
		return false;
	}

	auto resolved_queues = queue_resolver.GetQueues( vk_device );
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

bool vk2d::_internal::InstanceImpl::CreateDescriptorPool()
{
	descriptor_pool			= vk2d::_internal::CreateDescriptorAutoPool(
		this,
		vk_device
	);

	if( descriptor_pool ) {
		return true;
	}
	return false;
}









bool vk2d::_internal::InstanceImpl::CreateDefaultSampler()
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
	} else {
		default_sampler		= nullptr;
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default sampler!" );
		return false;
	}
}









bool vk2d::_internal::InstanceImpl::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info {};
	pipeline_cache_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.pNext				= nullptr;
	pipeline_cache_create_info.flags				= 0;
	pipeline_cache_create_info.initialDataSize		= 0;
	pipeline_cache_create_info.pInitialData			= nullptr;

	if( vkCreatePipelineCache(
		vk_device,
		&pipeline_cache_create_info,
		nullptr,
		&vk_pipeline_cache
	) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan pipeline cache!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::InstanceImpl::CreateShaderModules()
{
	auto CreateModule = [ this ](
		uint32_t	*	code,
		size_t			element_count
		) -> VkShaderModule
	{
		VkShaderModuleCreateInfo shader_create_info {};
		shader_create_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.pNext		= nullptr;
		shader_create_info.flags		= 0;
		shader_create_info.codeSize		= element_count * sizeof( uint32_t );
		shader_create_info.pCode		= code;

		VkShaderModule shader_module {};
		if( vkCreateShaderModule(
			vk_device,
			&shader_create_info,
			nullptr,
			&shader_module
		) != VK_SUCCESS ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan shader module!" );
			return {};
		}
		return shader_module;
	};



	// Create individual shader modules for single textured entries.
	auto single_textured_vertex								= CreateModule(
		SingleTexturedVertex_vert_shader_data.data(),
		SingleTexturedVertex_vert_shader_data.size()
	);
	auto single_textured_fragment							= CreateModule(
		SingleTexturedFragment_frag_shader_data.data(),
		SingleTexturedFragment_frag_shader_data.size()
	);
	auto single_textured_fragment_uv_border_color			= CreateModule(
		SingleTexturedFragmentWithUVBorderColor_frag_shader_data.data(),
		SingleTexturedFragmentWithUVBorderColor_frag_shader_data.size()
	);



	// Create individual shader modules for single textured entries.
	auto multitextured_vertex								= CreateModule(
		MultitexturedVertex_vert_shader_data.data(),
		MultitexturedVertex_vert_shader_data.size()
	);

	auto multitextured_fragment_triangle					= CreateModule(
		MultitexturedFragmentTriangle_frag_shader_data.data(),
		MultitexturedFragmentTriangle_frag_shader_data.size()
	);
	auto multitextured_fragment_line						= CreateModule(
		MultitexturedFragmentLine_frag_shader_data.data(),
		MultitexturedFragmentLine_frag_shader_data.size()
	);
	auto multitextured_fragment_point						= CreateModule(
		MultitexturedFragmentPoint_frag_shader_data.data(),
		MultitexturedFragmentPoint_frag_shader_data.size()
	);
	auto multitextured_fragment_triangle_uv_border_color	= CreateModule(
		MultitexturedFragmentTriangleWithUVBorderColor_frag_shader_data.data(),
		MultitexturedFragmentTriangleWithUVBorderColor_frag_shader_data.size()
	);
	auto multitextured_fragment_line_uv_border_color		= CreateModule(
		MultitexturedFragmentLineWithUVBorderColor_frag_shader_data.data(),
		MultitexturedFragmentLineWithUVBorderColor_frag_shader_data.size()
	);
	auto multitextured_fragment_point_uv_border_color		= CreateModule(
		MultitexturedFragmentPointWithUVBorderColor_frag_shader_data.data(),
		MultitexturedFragmentPointWithUVBorderColor_frag_shader_data.size()
	);

	// List all individual shader modules into a vector
	vk_shader_modules.push_back( single_textured_vertex );
	vk_shader_modules.push_back( single_textured_fragment );
	vk_shader_modules.push_back( single_textured_fragment_uv_border_color );

	vk_shader_modules.push_back( multitextured_vertex );
	vk_shader_modules.push_back( multitextured_fragment_triangle );
	vk_shader_modules.push_back( multitextured_fragment_line );
	vk_shader_modules.push_back( multitextured_fragment_point );
	vk_shader_modules.push_back( multitextured_fragment_triangle_uv_border_color );
	vk_shader_modules.push_back( multitextured_fragment_line_uv_border_color );
	vk_shader_modules.push_back( multitextured_fragment_point_uv_border_color );

	// Collect a listing of shader units, which is a collection of shader modules needed to create a pipeline.
	shader_programs[ vk2d::_internal::ShaderProgramID::SINGLE_TEXTURED ]							= vk2d::_internal::ShaderProgram( single_textured_vertex, single_textured_fragment );
	shader_programs[ vk2d::_internal::ShaderProgramID::SINGLE_TEXTURED_UV_BORDER_COLOR ]			= vk2d::_internal::ShaderProgram( single_textured_vertex, single_textured_fragment_uv_border_color );

	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_TRIANGLE ]					= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_triangle );
	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_LINE ]						= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_line );
	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_POINT ]						= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_point );
	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR ]	= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_triangle_uv_border_color );
	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_LINE_UV_BORDER_COLOR ]		= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_line_uv_border_color );
	shader_programs[ vk2d::_internal::ShaderProgramID::MULTITEXTURED_POINT_UV_BORDER_COLOR ]		= vk2d::_internal::ShaderProgram( multitextured_vertex, multitextured_fragment_point_uv_border_color );

	return true;
}









bool vk2d::_internal::InstanceImpl::CreateDescriptorSetLayouts()
{
	// These must match shader set types.

	// Set 1 layout, Combined image sampler and sampler data
	// Binding 0 = Combined image sampler
	// Binding 1 = Uniform buffer for sampler data
	{
		std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// Need this to access array textures
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

		sampler_texture_descriptor_set_layout		= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !sampler_texture_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return false;
		}
	}

	// General uniform buffer descriptor set layout
	{
		std::array<VkDescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		uniform_buffer_descriptor_set_layout			= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !uniform_buffer_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create storage buffer descriptor set layout!" );
			return false;
		}
	}

	// General storage buffer descriptor set layout
	{
		std::array<VkDescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		storage_buffer_descriptor_set_layout			= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !storage_buffer_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create storage buffer descriptor set layout!" );
			return false;
		}
	}

	return true;
}









bool vk2d::_internal::InstanceImpl::CreatePipelineLayout()
{
	// This must match shader layout.

	std::vector<VkDescriptorSetLayout> set_layouts {
		uniform_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 0 is WindowFrameData.
		storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 1 is vertex index buffer as storage buffer.
		storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 2 is vertex buffer as storage buffer.
		sampler_texture_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 3 is combined sampler texture and it's data uniform.
		storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout()	// Pipeline set 4 is texture channel weight data.
	};

	std::array<VkPushConstantRange, 1> push_constant_ranges {};
	push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_ranges[ 0 ].offset		= 0;
	push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vk2d::_internal::PushConstants ) );

	VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
	pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pNext					= nullptr;
	pipeline_layout_create_info.flags					= 0;
	pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
	pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
	pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

	if( vkCreatePipelineLayout(
		vk_device,
		&pipeline_layout_create_info,
		nullptr,
		&vk_pipeline_layout
	) != VK_SUCCESS ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan pipeline layout!" );
		return false;
	}

	return true;
}

bool vk2d::_internal::InstanceImpl::CreateDeviceMemoryPool()
{
	device_memory_pool		= MakeDeviceMemoryPool(
		vk_physical_device,
		vk_device
	);
	if( !device_memory_pool ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create memory pool!" );
		return false;
	}
	return true;
}

bool vk2d::_internal::InstanceImpl::CreateThreadPool()
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

bool vk2d::_internal::InstanceImpl::CreateResourceManager()
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

bool vk2d::_internal::InstanceImpl::CreateDefaultTexture()
{
	default_texture		= resource_manager->CreateTextureResource(
		vk2d::Vector2u( 1, 1 ),
		{ vk2d::Color8( 255, 255, 255, 255 ) }
	);
	return true;
}



void vk2d::_internal::InstanceImpl::DestroyInstance()
{
	if( vk_debug_utils_messenger ) {
		auto destroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( vk_instance, "vkDestroyDebugUtilsMessengerEXT" );
		if( destroyDebugUtilsMessenger ) {
			destroyDebugUtilsMessenger(
				vk_instance,
				vk_debug_utils_messenger,
				nullptr
			);
		}

		vk_debug_utils_messenger	= {};
	}

	vkDestroyInstance(
		vk_instance,
		nullptr
	);

	vk_instance				= {};
}

void vk2d::_internal::InstanceImpl::DestroyDevice()
{
	vkDestroyDevice(
		vk_device,
		nullptr
	);

	vk_device					= {};
}

void vk2d::_internal::InstanceImpl::DestroyDescriptorPool()
{
	descriptor_pool			= nullptr;
}

void vk2d::_internal::InstanceImpl::DestroyDefaultSampler()
{
	default_sampler			= {};
}

void vk2d::_internal::InstanceImpl::DestroyPipelineCache()
{
	vkDestroyPipelineCache(
		vk_device,
		vk_pipeline_cache,
		nullptr
	);

	vk_pipeline_cache			= {};
}

void vk2d::_internal::InstanceImpl::DestroyPipelines()
{
	for( auto p : vk_pipelines ) {
		vkDestroyPipeline(
			vk_device,
			p.second,
			nullptr
		);
	}
	vk_pipelines.clear();
}

void vk2d::_internal::InstanceImpl::DestroyShaderModules()
{
	for( auto s : vk_shader_modules ) {
		vkDestroyShaderModule(
			vk_device,
			s,
			nullptr
		);
	}
	vk_shader_modules.clear();
	shader_programs.clear();
}

void vk2d::_internal::InstanceImpl::DestroyDescriptorSetLayouts()
{
	sampler_texture_descriptor_set_layout	= nullptr;
	uniform_buffer_descriptor_set_layout	= nullptr;
	storage_buffer_descriptor_set_layout	= nullptr;
}

void vk2d::_internal::InstanceImpl::DestroyPipelineLayout()
{
	vkDestroyPipelineLayout(
		vk_device,
		vk_pipeline_layout,
		nullptr
	);

	vk_pipeline_layout			= {};
}

void vk2d::_internal::InstanceImpl::DestroyDeviceMemoryPool()
{
	device_memory_pool		= {};
}

void vk2d::_internal::InstanceImpl::DestroyThreadPool()
{
	thread_pool				= {};
}

void vk2d::_internal::InstanceImpl::DestroyResourceManager()
{
	resource_manager		= {};
}

void vk2d::_internal::InstanceImpl::DestroyDefaultTexture()
{
	resource_manager->DestroyResource( default_texture );
	default_texture						= {};

	// Old code replaced by texture resource.
//	descriptor_pool->FreeDescriptorSet( default_texture_descriptor_set );
//	device_memory_pool->FreeCompleteResource( default_texture );

//	default_texture						= {};
//	default_texture_descriptor_set		= {};
}









std::vector<VkPhysicalDevice> vk2d::_internal::InstanceImpl::EnumeratePhysicalDevices()
{
	uint32_t physical_device_count		= UINT32_MAX;
	if( vkEnumeratePhysicalDevices( vk_instance, &physical_device_count, nullptr ) == VK_SUCCESS ) {
		std::vector<VkPhysicalDevice> physical_devices( physical_device_count );
		if( vkEnumeratePhysicalDevices( vk_instance, &physical_device_count, physical_devices.data() ) == VK_SUCCESS ) {
			return physical_devices;
		}
	}
	Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot enumerate physical devices!" );
	return {};
}









VkPhysicalDevice vk2d::_internal::InstanceImpl::PickBestVulkanPhysicalDevice()
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
			if( glfwGetPhysicalDevicePresentationSupport( vk_instance, pd, i ) ) {
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
