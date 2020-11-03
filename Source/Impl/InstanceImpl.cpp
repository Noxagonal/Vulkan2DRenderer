
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
#include "../Header/Impl/SamplerImpl.h"
#include "../../Include/Interface/TextureResource.h"

#include "../Shaders/Spir-V/IncludeAllShaders.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace vk2d {

namespace _internal {



void VK2D_APIENTRY VK2D_default_ReportFunction(
	vk2d::ReportSeverity			severity,
	std::string						message
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

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
	case vk2d::ReportSeverity::DEVICE_LOST:
		vk2d::SetConsoleColor( vk2d::ConsoleColor::WHITE, vk2d::ConsoleColor::DARK_RED );
		break;
	default:
		vk2d::SetConsoleColor();
		break;
	}

	std::cout << message << "\n";
	vk2d::SetConsoleColor();

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	if( severity == vk2d::ReportSeverity::DEVICE_LOST ) {

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
	}
#endif

	if( severity >= vk2d::ReportSeverity::NON_CRITICAL_ERROR ) {
#ifdef _WIN32
		MessageBox( NULL, message.c_str(), "Critical error!", MB_OK | MB_ICONERROR );
#endif
		if( severity >= vk2d::ReportSeverity::CRITICAL_ERROR ) {
			std::exit( -1 );
		}
	}
}



std::mutex										instance_globals_mutex;
vk2d::Monitor									primary_monitor				= {};
std::vector<vk2d::Monitor>						all_monitors				= {};
std::list<vk2d::_internal::InstanceImpl*>		instance_listeners			= {};

void UpdateMonitorLists(
	bool		globals_locked		= false
)
{
	// Lock globals if not already locked.
	std::unique_lock<std::mutex> unique_lock;
	if( !globals_locked ) {
		unique_lock = std::unique_lock<std::mutex>( instance_globals_mutex );
	}

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
	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );

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
	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );

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
		vk2d::_internal::UpdateMonitorLists( true );
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

#if VK2D_DEBUG_ENABLE
	if( report_function == nullptr ) {
		report_function = VK2D_default_ReportFunction;
	}
#endif

//	Introduce layers and extensions here
//	instance_layers;
//	Instance extensions.
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
	device_extensions.push_back( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );

#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	device_extensions.push_back( VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME );
#endif

	if( !CreateInstance() ) return;
	if( !PickPhysicalDevice() ) return;
	if( !CreateDeviceAndQueues() ) return;

	#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	if( instance_count == 1 ) {
		vk2d::_internal::SetCommandBufferCheckpointQueue(
			vk_device,
			primary_render_queue.GetQueue(),
			primary_render_queue.GetQueueMutex()
		);
	}
	#endif

	if( !PopulateNonStaticallyExposedVulkanFunctions() ) return;
	if( !CreateDescriptorSetLayouts() ) return;
	if( !CreateDescriptorPool() ) return;
	if( !CreatePipelineCache() ) return;
	if( !CreateShaderModules() ) return;
	if( !CreatePipelineLayouts() ) return;
	if( !CreateDeviceMemoryPool() ) return;
	if( !CreateThreadPool() ) return;
	if( !CreateResourceManager() ) return;
	if( !CreateDefaultTexture() ) return;
	if( !CreateDefaultSampler() ) return;
	if( !CreateBlurSampler() ) return;

	vk2d::_internal::instance_listeners.push_back( this );
	creator_thread_id	= std::this_thread::get_id();
	is_good				= true;
}

vk2d::_internal::InstanceImpl::~InstanceImpl()
{
	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );

	vk2d::_internal::instance_listeners.remove( this );

	vkDeviceWaitIdle( vk_device );

	windows.clear();
	render_target_textures.clear();
	cursors.clear();
	samplers.clear();

	DestroyBlurSampler();
	DestroyDefaultSampler();
	DestroyDefaultTexture();
	DestroyResourceManager();
	DestroyThreadPool();
	DestroyDeviceMemoryPool();
	DestroyPipelines();
	DestroyPipelineLayouts();
	DestroyShaderModules();
	DestroyPipelineCaches();
	DestroyDescriptorPool();
	DestroyDescriptorSetLayouts();
	DestroyDevice();
	DestroyInstance();

	// If this is the last instance exiting, also terminate glfw
	--instance_count;
	if( instance_count == 0 ) {
		glfwTerminate();
	}
}

std::vector<vk2d::Monitor*> vk2d::_internal::InstanceImpl::GetMonitors()
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::GetMonitors() must be called from main thread only!" );
		return {};
	}

	std::vector<vk2d::Monitor*> ret;
	ret.reserve( vk2d::_internal::all_monitors.size() );

	for( auto & m : vk2d::_internal::all_monitors ) {
		ret.push_back( &m );
	}

	return ret;
}

vk2d::Monitor * vk2d::_internal::InstanceImpl::GetPrimaryMonitor() const
{
	return &vk2d::_internal::primary_monitor;
}

void vk2d::_internal::InstanceImpl::SetMonitorUpdateCallback(
	vk2d::MonitorUpdateCallbackFun		monitor_update_callback_funtion
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::SetMonitorUpdateCallback() must be called from main thread only!" );
		return;
	}

	monitor_update_callback		= monitor_update_callback_funtion;
}

vk2d::Cursor * vk2d::_internal::InstanceImpl::CreateCursor(
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::CreateCursor() must be called from main thread only!" );
		return {};
	}

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
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::CreateCursor() must be called from main thread only!" );
		return {};
	}

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
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::DestroyCursor() must be called from main thread only!" );
		return;
	}

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

vk2d::GamepadEventCallbackFun vk2d::_internal::InstanceImpl::GetGamepadEventCallback() const
{
	return joystick_event_callback;
}

void vk2d::_internal::InstanceImpl::SetGamepadEventCallback(
	vk2d::GamepadEventCallbackFun		joystick_event_callback_function
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::SetGamepadEventCallback() must be called from main thread only!" );
		return;
	}

	joystick_event_callback		= joystick_event_callback_function;
}

bool vk2d::_internal::InstanceImpl::IsGamepadPresent( vk2d::Gamepad joystick )
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::IsGamepadPresent() must be called from main thread only!" );
		return {};
	}

	if( glfwJoystickIsGamepad( int( joystick ) ) == GLFW_TRUE ) {
		return true;
	} else {
		return false;
	}
}

std::string vk2d::_internal::InstanceImpl::GetGamepadName( vk2d::Gamepad gamepad )
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::GetGamepadName() must be called from main thread only!" );
		return {};
	}

	return glfwGetGamepadName( int( gamepad ) );
}

vk2d::GamepadState vk2d::_internal::InstanceImpl::QueryGamepadState(
	vk2d::Gamepad			gamepad )
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::QueryGamepadState() must be called from main thread only!" );
		return {};
	}

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
	const vk2d::WindowCreateInfo	&	window_create_info
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::CreateOutputWindow() must be called from main thread only!" );
		return {};
	}

	auto new_window = std::unique_ptr<vk2d::Window>( new vk2d::Window( this, window_create_info ) );

	if( new_window->is_good ) {
		windows.push_back( std::move( new_window ) );
		return windows.back().get();
	}
	return {};
}

void vk2d::_internal::InstanceImpl::DestroyOutputWindow(
	vk2d::Window				*	window
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::DestroyOutputWindow() must be called from main thread only!" );
		return;
	}

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

vk2d::RenderTargetTexture * vk2d::_internal::InstanceImpl::CreateRenderTargetTexture(
	const vk2d::RenderTargetTextureCreateInfo		&	render_target_texture_create_info
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::CreateRenderTargetTexture() must be called from main thread only!" );
		return {};
	}

	auto render_target_texture	= std::unique_ptr<vk2d::RenderTargetTexture>(
		new vk2d::RenderTargetTexture( this, render_target_texture_create_info )
	);

	if( render_target_texture && render_target_texture->IsGood() ) {
		auto ret	= render_target_texture.get();
		render_target_textures.push_back( std::move( render_target_texture ) );
		return ret;
	} else {
		return nullptr;
	}
}

void vk2d::_internal::InstanceImpl::DestroyRenderTargetTexture(
	vk2d::RenderTargetTexture						*	render_target_texture
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::DestroyRenderTargetTexture() must be called from main thread only!" );
		return;
	}

	auto result = vkDeviceWaitIdle(
		vk_device
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Cannot destroy render target, error waiting device!" );
	}

	auto it = render_target_textures.begin();
	while( it != render_target_textures.end() ) {
		if( it->get() == render_target_texture ) {
			it = render_target_textures.erase( it );
			break;
		} else {
			++it;
		}
	}
}

vk2d::_internal::PoolDescriptorSet vk2d::_internal::InstanceImpl::AllocateDescriptorSet(
	const vk2d::_internal::DescriptorSetLayout		&	for_descriptor_set_layout
)
{
	std::lock_guard<std::mutex> lock_guard( descriptor_pool_mutex );

	return descriptor_pool->AllocateDescriptorSet(
		for_descriptor_set_layout
	);
}

void vk2d::_internal::InstanceImpl::FreeDescriptorSet(
	vk2d::_internal::PoolDescriptorSet & descriptor_set
)
{
	std::lock_guard<std::mutex> lock_guard( descriptor_pool_mutex );

	descriptor_pool->FreeDescriptorSet(
		descriptor_set
	);
}


/*
vk2d::_internal::DescriptorAutoPool * vk2d::_internal::InstanceImpl::GetDescriptorPool()
{
	return descriptor_pool.get();
}
*/


vk2d::Sampler * vk2d::_internal::InstanceImpl::CreateSampler(
	const vk2d::SamplerCreateInfo	&	sampler_create_info
)
{
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::CreateSampler() must be called from main thread only!" );
		return {};
	}

	auto sampler	= std::unique_ptr<vk2d::Sampler>(
		new vk2d::Sampler( this, sampler_create_info )
	);

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
	if( !IsThisThreadCreatorThread() ) {
		Report( vk2d::ReportSeverity::WARNING, "Instance::DestroySampler() must be called from main thread only!" );
		return;
	}

	auto result = vkDeviceWaitIdle(
		vk_device
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Cannot destroy sampler, error waiting device!" );
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

vk2d::Multisamples vk2d::_internal::InstanceImpl::GetMaximumSupportedMultisampling() const
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

vk2d::Multisamples vk2d::_internal::InstanceImpl::GetAllSupportedMultisampling() const
{
	return vk2d::Multisamples( vk_physical_device_properties.limits.framebufferColorSampleCounts );
}









vk2d::PFN_VK2D_ReportFunction vk2d::_internal::InstanceImpl::GetReportFunction() const
{
	return report_function;
}

void vk2d::_internal::InstanceImpl::Report(
	VkResult					vk_result,
	vk2d::ReportSeverity		severity,
	const std::string		&	message
)
{
	if( report_function ) {
		std::lock_guard<std::mutex> report_lock( report_mutex );

		if( vk_result == VK_ERROR_DEVICE_LOST ) severity = vk2d::ReportSeverity::DEVICE_LOST;

		report_function(
			severity,
			message
		);
	}
}

void vk2d::_internal::InstanceImpl::Report(
	VkResult					vk_result,
	const std::string		&	message )
{
	if( report_function ) {
		std::lock_guard<std::mutex> report_lock( report_mutex );

		auto severity = vk2d::ReportSeverity::NONE;

		switch( vk_result ) {
			case VK_SUCCESS:
			case VK_THREAD_IDLE_KHR:
			case VK_THREAD_DONE_KHR:
			case VK_OPERATION_DEFERRED_KHR:
			case VK_OPERATION_NOT_DEFERRED_KHR:
				severity = vk2d::ReportSeverity::VERBOSE;
				break;

			case VK_PIPELINE_COMPILE_REQUIRED_EXT:
				severity = vk2d::ReportSeverity::INFO;
				break;

			case VK_NOT_READY:
			case VK_TIMEOUT:
			case VK_EVENT_SET:
			case VK_EVENT_RESET:
			case VK_INCOMPLETE:
			case VK_SUBOPTIMAL_KHR:
				severity = vk2d::ReportSeverity::WARNING;
				break;

			case VK_ERROR_OUT_OF_HOST_MEMORY:
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			case VK_ERROR_MEMORY_MAP_FAILED:
			case VK_ERROR_TOO_MANY_OBJECTS:
			case VK_ERROR_FORMAT_NOT_SUPPORTED:
			case VK_ERROR_FRAGMENTED_POOL:
			case VK_ERROR_OUT_OF_POOL_MEMORY:
			case VK_ERROR_FRAGMENTATION:
			case VK_ERROR_OUT_OF_DATE_KHR:
				severity = vk2d::ReportSeverity::NON_CRITICAL_ERROR;
				break;

			case VK_ERROR_INITIALIZATION_FAILED:
			case VK_ERROR_LAYER_NOT_PRESENT:
			case VK_ERROR_EXTENSION_NOT_PRESENT:
			case VK_ERROR_FEATURE_NOT_PRESENT:
			case VK_ERROR_INCOMPATIBLE_DRIVER:
			case VK_ERROR_UNKNOWN:
			case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			case VK_ERROR_SURFACE_LOST_KHR:
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			case VK_ERROR_VALIDATION_FAILED_EXT:
			case VK_ERROR_INVALID_SHADER_NV:
			case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			case VK_ERROR_NOT_PERMITTED_EXT:
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
				severity = vk2d::ReportSeverity::CRITICAL_ERROR;
				break;

			case VK_ERROR_DEVICE_LOST:
				severity = vk2d::ReportSeverity::DEVICE_LOST;
				break;

			default:
				severity = vk2d::ReportSeverity::CRITICAL_ERROR;
				break;
		}

		report_function(
			severity,
			message
		);
	}
}

void vk2d::_internal::InstanceImpl::Report( vk2d::ReportSeverity severity, const std::string & message )
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

vk2d::_internal::GraphicsShaderProgram vk2d::_internal::InstanceImpl::GetGraphicsShaderModules(
	vk2d::_internal::GraphicsShaderProgramID			id
) const
{
	auto collection = graphics_shader_programs.find( id );
	if( collection != graphics_shader_programs.end() ) {
		return collection->second;
	}
	return {};
}

vk2d::_internal::ComputeShaderProgram vk2d::_internal::InstanceImpl::GetComputeShaderModules(
	vk2d::_internal::ComputeShaderProgramID	id
) const
{
	auto collection = compute_shader_programs.find( id );
	if( collection != compute_shader_programs.end() ) {
		return collection->second;
	}
	return {};
}

vk2d::_internal::GraphicsShaderProgram vk2d::_internal::InstanceImpl::GetCompatibleGraphicsShaderModules(
	bool				multitextured,
	bool				custom_uv_border_color,
	uint32_t			vertices_per_primitive
) const
{
	if( multitextured ) {
		if( custom_uv_border_color ) {
			if( vertices_per_primitive == 1 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_POINT_UV_BORDER_COLOR );
			}
			if( vertices_per_primitive == 2 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_LINE_UV_BORDER_COLOR );
			}
			if( vertices_per_primitive == 3 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR );
			}
		} else {
			if( vertices_per_primitive == 1 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_POINT );
			}
			if( vertices_per_primitive == 2 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_LINE );
			}
			if( vertices_per_primitive == 3 ) {
				return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_TRIANGLE );
			}
		}
	} else {
		if( custom_uv_border_color ) {
			return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::SINGLE_TEXTURED_UV_BORDER_COLOR );
		} else {
			return GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::SINGLE_TEXTURED );
		}
	}

	return {};
}

VkPipeline vk2d::_internal::InstanceImpl::GetGraphicsPipeline(
	const vk2d::_internal::GraphicsPipelineSettings		&	settings
)
{
	// TODO: Potential multithreaded access conflict, don't know if this will be a requirement at this point.
	// Need mutex here as this function can be called from multiple threads.
	// Alternatively we could create vulkan pipelines per window and render
	// target texture, but that would probably be wasteful. Regardless this
	// function will be called very often so we'll need a way of caching
	// these pipelines within the windows and render target textures.
	// Since the pipelines won't change throughout the execution a simple
	// std::map per window and render target with a pipeline pointer
	// would probably work well, if the pipeline is not found in the local
	// cache, use this function to get it / create it.
	auto p_it = vk_graphics_pipelines.find( settings );
	if( p_it != vk_graphics_pipelines.end() ) {
		return p_it->second;
	}
	return CreateGraphicsPipeline( settings );
}

VkPipeline vk2d::_internal::InstanceImpl::GetComputePipeline(
	const vk2d::_internal::ComputePipelineSettings		&	settings
)
{
	// TODO: Potential multithreaded access conflict, don't know if this will be a requirement at this point.
	// Need mutex here as this function can be called from multiple threads.
	// Alternatively we could create vulkan pipelines per window and render
	// target texture, but that would probably be wasteful. Regardless this
	// function will be called very often so we'll need a way of caching
	// these pipelines within the windows and render target textures.
	// Since the pipelines won't change throughout the execution a simple
	// std::map per window and render target with a pipeline pointer
	// would probably work well, if the pipeline is not found in the local
	// cache, use this function to get it / create it.
	auto p_it = vk_compute_pipelines.find( settings );
	if( p_it != vk_compute_pipelines.end() ) {
		return p_it->second;
	}
	return CreateComputePipeline( settings );
}

VkPipeline vk2d::_internal::InstanceImpl::CreateGraphicsPipeline(
	const vk2d::_internal::GraphicsPipelineSettings		&	settings
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
	pipeline_create_info.layout					= GetGraphicsPipelineLayout();
	pipeline_create_info.renderPass				= settings.vk_render_pass;
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	VkPipeline pipeline {};
	auto result = vkCreateGraphicsPipelines(
		vk_device,
		GetGraphicsPipelineCache(),
		1,
		&pipeline_create_info,
		nullptr,
		&pipeline
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot create Vulkan graphics pipeline!" );
		return {};
	}

	vk_graphics_pipelines[ settings ] = pipeline;
	return pipeline;
}

VkPipeline vk2d::_internal::InstanceImpl::CreateComputePipeline( const vk2d::_internal::ComputePipelineSettings & settings )
{
	VkPipelineShaderStageCreateInfo stage_create_info {};
	stage_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_create_info.pNext						= nullptr;
	stage_create_info.flags						= 0;
	stage_create_info.stage						= VK_SHADER_STAGE_COMPUTE_BIT;
	stage_create_info.module					= settings.shader_programs.compute;
	stage_create_info.pName						= "main";
	stage_create_info.pSpecializationInfo		= nullptr;

	VkComputePipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType					= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_create_info.pNext					= nullptr;
	pipeline_create_info.flags					= 0;
	pipeline_create_info.stage					= stage_create_info;
	pipeline_create_info.layout					= settings.pipeline_layout;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	VkPipeline pipeline {};
	auto result = vkCreateComputePipelines(
		vk_device,
		GetComputePipelineCache(),
		1,
		&pipeline_create_info,
		nullptr,
		&pipeline
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot create Vulkan compute pipeline!" );
		return {};
	}

	vk_compute_pipelines[ settings ] = pipeline;
	return pipeline;
}

VkPipelineCache vk2d::_internal::InstanceImpl::GetGraphicsPipelineCache() const
{
	return vk_graphics_pipeline_cache;
}

VkPipelineCache vk2d::_internal::InstanceImpl::GetComputePipelineCache() const
{
	return vk_compute_pipeline_cache;
}

VkPipelineLayout vk2d::_internal::InstanceImpl::GetGraphicsPipelineLayout() const
{
	return vk_graphics_pipeline_layout;
}

VkPipelineLayout vk2d::_internal::InstanceImpl::GetComputeBlurPipelineLayout() const
{
	return vk_compute_blur_pipeline_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetComputeBasicSamplerDescriptorSetLayout() const
{
	return *compute_basic_sampler_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetComputeBlurTexturesDescriptorSetLayout() const
{
	return *compute_blur_textures_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetGraphicsSamplerDescriptorSetLayout() const
{
	return *graphics_sampler_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetGraphicsTextureDescriptorSetLayout() const
{
	return *graphics_texture_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetGraphicsUniformBufferDescriptorSetLayout() const
{
	return *graphics_uniform_buffer_descriptor_set_layout;
}

const vk2d::_internal::DescriptorSetLayout & vk2d::_internal::InstanceImpl::GetGraphicsStorageBufferDescriptorSetLayout() const
{
	return *graphics_storage_buffer_descriptor_set_layout;
}

vk2d::Texture * vk2d::_internal::InstanceImpl::GetDefaultTexture() const
{
	return default_texture;
}

vk2d::Sampler * vk2d::_internal::InstanceImpl::GetDefaultSampler() const
{
	return default_sampler.get();
}

VkDescriptorSet vk2d::_internal::InstanceImpl::GetBlurSamplerDescriptorSet() const
{
	return blur_sampler_descriptor_set.descriptorSet;
}

vk2d::_internal::DeviceMemoryPool * vk2d::_internal::InstanceImpl::GetDeviceMemoryPool() const
{
	return device_memory_pool.get();
}

std::thread::id vk2d::_internal::InstanceImpl::GetCreatorThreadID() const
{
	return creator_thread_id;
}

bool vk2d::_internal::InstanceImpl::IsThisThreadCreatorThread() const
{
	return creator_thread_id == std::this_thread::get_id();
}

void vk2d::_internal::InstanceImpl::VkFun_vkCmdPushDescriptorSetKHR(
	VkCommandBuffer					commandBuffer,
	VkPipelineBindPoint				pipelineBindPoint,
	VkPipelineLayout				layout,
	uint32_t						set,
	uint32_t						descriptorWriteCount,
	const VkWriteDescriptorSet	*	pDescriptorWrites
)
{
	vk_fun_vkCmdPushDescriptorSetKHR(
		commandBuffer,
		pipelineBindPoint,
		layout,
		set,
		descriptorWriteCount,
		pDescriptorWrites
	);
}

bool vk2d::_internal::InstanceImpl::IsGood() const
{
	return is_good;
}









#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE

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
	ss_title << "Vulkan Validation: " << str_severity << " | " << str_type;

	std::stringstream ss_message;
	ss_message << "\n" << ss_title.str() << ":\n\n - " << pCallbackData->pMessage << "\n\n";
	// TODO: labels, object, message id name / number;

	auto instance = reinterpret_cast<vk2d::_internal::InstanceImpl*>( pUserData );
	assert( instance );

	instance->GetReportFunction()( vk2d_severity, ss_message.str() );

	//std::cout << ss_message.str();
	//#if _WIN32
	//if( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) {
	//	MessageBox( NULL, ss_message.str().c_str(), ss_title.str().c_str(), MB_OK | MB_ICONERROR );
	//}
	//#endif

	return VK_FALSE;
}

} // _internal

} // vk2d

#endif









bool vk2d::_internal::InstanceImpl::CreateInstance()
{

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	instance_layers.push_back( "VK_LAYER_KHRONOS_validation" );
	// instance_layers.push_back( "VK_LAYER_LUNARG_device_simulation" );
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
#if VK2D_BUILD_OPTION_VULKAN_GPU_ASSISTED_VALIDATION && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
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
		assert( VK_HEADER_VERSION_COMPLETE >= VK2D_BUILD_OPTION_VULKAN_MINIMUM_REQUIRED_VERSION );

		VkApplicationInfo application_info {};
		application_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pNext				= nullptr;
		application_info.pApplicationName	= create_info_copy.application_name.c_str();
		application_info.applicationVersion	= create_info_copy.application_version.ToVulkanVersion();
		application_info.pEngineName		= create_info_copy.engine_name.c_str();
		application_info.engineVersion		= create_info_copy.engine_version.ToVulkanVersion();
		application_info.apiVersion			= VK2D_BUILD_OPTION_VULKAN_MINIMUM_REQUIRED_VERSION;

		VkInstanceCreateInfo instance_create_info {};
		instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
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

		auto result = vkCreateInstance(
			&instance_create_info,
			nullptr,
			&vk_instance
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create vulkan instance!" );
			return false;
		}
	}

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	{
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( vk_instance, "vkCreateDebugUtilsMessengerEXT" );
		if( !createDebugUtilsMessenger ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return false;
		}
		auto result = createDebugUtilsMessenger(
			vk_instance,
			&debug_utils_create_info,
			nullptr,
			&vk_debug_utils_messenger
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create vulkan debug object!" );
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
	features.samplerAnisotropy						= VK_TRUE;
	features.fillModeNonSolid						= VK_TRUE;
	features.wideLines								= VK_TRUE;
	features.geometryShader							= VK_TRUE;
	features.shaderStorageImageWriteWithoutFormat	= VK_TRUE;
//	features.fragmentStoresAndAtomics				= VK_TRUE;

	VkPhysicalDeviceVulkan12Features features_1_2 {};
	features_1_2.sType								= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features_1_2.pNext								= nullptr;
	features_1_2.samplerMirrorClampToEdge			= VK_TRUE;
	features_1_2.timelineSemaphore					= VK_TRUE;

	VkDeviceCreateInfo device_create_info {};
	device_create_info.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext						= &features_1_2;
	device_create_info.flags						= 0;
	device_create_info.queueCreateInfoCount			= uint32_t( queue_create_infos.size() );
	device_create_info.pQueueCreateInfos			= queue_create_infos.data();
	device_create_info.enabledLayerCount			= 0;
	device_create_info.ppEnabledLayerNames			= nullptr;
	device_create_info.enabledExtensionCount		= uint32_t( device_extensions.size() );
	device_create_info.ppEnabledExtensionNames		= device_extensions.data();
	device_create_info.pEnabledFeatures				= &features;

	auto result = vkCreateDevice(
		vk_physical_device,
		&device_create_info,
		nullptr,
		&vk_device
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot create Vulkan device!" );
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

bool vk2d::_internal::InstanceImpl::CreateBlurSampler()
{
	{
		vk2d::SamplerCreateInfo sampler_create_info {};
		sampler_create_info.minification_filter				= vk2d::SamplerFilter::LINEAR;
		sampler_create_info.magnification_filter			= vk2d::SamplerFilter::LINEAR;
		sampler_create_info.mipmap_mode						= vk2d::SamplerMipmapMode::NEAREST;
		sampler_create_info.address_mode_u					= vk2d::SamplerAddressMode::CLAMP_TO_EDGE;
		sampler_create_info.address_mode_v					= vk2d::SamplerAddressMode::CLAMP_TO_EDGE;
		sampler_create_info.border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
		sampler_create_info.mipmap_enable					= false;
		sampler_create_info.mipmap_max_anisotropy			= 1.0f;
		sampler_create_info.mipmap_level_of_detail_bias		= 0.0f;
		sampler_create_info.mipmap_min_level_of_detail		= 0.0f;
		sampler_create_info.mipmap_max_level_of_detail		= 128.0f;
		blur_sampler			= std::unique_ptr<vk2d::Sampler>( new vk2d::Sampler( this, sampler_create_info ) );
		if( !blur_sampler || !blur_sampler->IsGood() ) {
			blur_sampler		= nullptr;
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create blur sampler!" );
			return false;
		}
	}

	{
		blur_sampler_descriptor_set = AllocateDescriptorSet( *compute_basic_sampler_descriptor_set_layout );
		if( blur_sampler_descriptor_set != VK_SUCCESS ) {
			FreeDescriptorSet( blur_sampler_descriptor_set );
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create blur sampler descriptor set!" );
			return false;
		}
	}

	{
		VkDescriptorImageInfo image_info {};
		image_info.sampler		= blur_sampler->impl->GetVulkanSampler();
		image_info.imageView	= VK_NULL_HANDLE;
		image_info.imageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;

		VkWriteDescriptorSet descriptor_write {};
		descriptor_write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.pNext				= nullptr;
		descriptor_write.dstSet				= blur_sampler_descriptor_set.descriptorSet;
		descriptor_write.dstBinding			= 0;
		descriptor_write.dstArrayElement	= 0;
		descriptor_write.descriptorCount	= 1;
		descriptor_write.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_write.pImageInfo			= &image_info;
		descriptor_write.pBufferInfo		= nullptr;
		descriptor_write.pTexelBufferView	= nullptr;
		vkUpdateDescriptorSets(
			vk_device,
			1, &descriptor_write,
			0, nullptr
		);
	}

	return true;
}









bool vk2d::_internal::InstanceImpl::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info {};
	pipeline_cache_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.pNext				= nullptr;
	pipeline_cache_create_info.flags				= 0;
	pipeline_cache_create_info.initialDataSize		= 0;
	pipeline_cache_create_info.pInitialData			= nullptr;

	auto result = vkCreatePipelineCache(
		vk_device,
		&pipeline_cache_create_info,
		nullptr,
		&vk_graphics_pipeline_cache
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot create Vulkan pipeline cache!" );
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
		auto result = vkCreateShaderModule(
			vk_device,
			&shader_create_info,
			nullptr,
			&shader_module
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create Vulkan shader module!" );
			return {};
		}
		return shader_module;
	};


	////////////////////////////////
	// Graphics shaders
	////////////////////////////////

	{
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
		vk_graphics_shader_modules.push_back( single_textured_vertex );
		vk_graphics_shader_modules.push_back( single_textured_fragment );
		vk_graphics_shader_modules.push_back( single_textured_fragment_uv_border_color );

		vk_graphics_shader_modules.push_back( multitextured_vertex );
		vk_graphics_shader_modules.push_back( multitextured_fragment_triangle );
		vk_graphics_shader_modules.push_back( multitextured_fragment_line );
		vk_graphics_shader_modules.push_back( multitextured_fragment_point );
		vk_graphics_shader_modules.push_back( multitextured_fragment_triangle_uv_border_color );
		vk_graphics_shader_modules.push_back( multitextured_fragment_line_uv_border_color );
		vk_graphics_shader_modules.push_back( multitextured_fragment_point_uv_border_color );

		// Collect a listing of shader units, which is a collection of shader modules needed to create a pipeline.
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::SINGLE_TEXTURED ]							= vk2d::_internal::GraphicsShaderProgram( single_textured_vertex, single_textured_fragment );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::SINGLE_TEXTURED_UV_BORDER_COLOR ]			= vk2d::_internal::GraphicsShaderProgram( single_textured_vertex, single_textured_fragment_uv_border_color );

		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_TRIANGLE ]					= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_triangle );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_LINE ]						= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_line );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_POINT ]						= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_point );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR ]	= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_triangle_uv_border_color );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_LINE_UV_BORDER_COLOR ]		= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_line_uv_border_color );
		graphics_shader_programs[ vk2d::_internal::GraphicsShaderProgramID::MULTITEXTURED_POINT_UV_BORDER_COLOR ]		= vk2d::_internal::GraphicsShaderProgram( multitextured_vertex, multitextured_fragment_point_uv_border_color );
	}



	////////////////////////////////
	// Compute shaders
	////////////////////////////////

	{
		// Render target texture blur shaders
		auto render_target_texture_blur_pass_1			= CreateModule(
			RenderTargetTextureBlurPass1_comp_shader_data.data(),
			RenderTargetTextureBlurPass1_comp_shader_data.size()
		);
		auto render_target_texture_blur_pass_2			= CreateModule(
			RenderTargetTextureBlurPass2_comp_shader_data.data(),
			RenderTargetTextureBlurPass2_comp_shader_data.size()
		);

		// List all individual shader modules into a vector
		vk_compute_shader_modules.push_back( render_target_texture_blur_pass_1 );
		vk_compute_shader_modules.push_back( render_target_texture_blur_pass_2 );

		// Collect a listing of shader units, which is a collection of shader modules needed to create a pipeline.
		compute_shader_programs[ vk2d::_internal::ComputeShaderProgramID::RENDER_TARGET_BLUR_PASS_1 ]				= vk2d::_internal::ComputeShaderProgram( render_target_texture_blur_pass_1 );
		compute_shader_programs[ vk2d::_internal::ComputeShaderProgramID::RENDER_TARGET_BLUR_PASS_2 ]				= vk2d::_internal::ComputeShaderProgram( render_target_texture_blur_pass_2 );
	}



	return true;
}









bool vk2d::_internal::InstanceImpl::CreateDescriptorSetLayouts()
{
	// These must match shader set types.

	// Graphics: Descriptor set layout for sampler.
	// Binding 0 = Sampler
	// Binding 1 = Uniform buffer for sampler data
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

		graphics_sampler_descriptor_set_layout = vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !graphics_sampler_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return false;
		}
	}

	// Graphics: Descriptor set layout for texture.
	// Binding 0 = Sampled image
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

		graphics_texture_descriptor_set_layout = vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !graphics_texture_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return false;
		}
	}

	// Graphics: General uniform buffer descriptor set layout
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

		graphics_uniform_buffer_descriptor_set_layout			= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !graphics_uniform_buffer_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create storage buffer descriptor set layout!" );
			return false;
		}
	}

	// Graphics: General storage buffer descriptor set layout
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

		graphics_storage_buffer_descriptor_set_layout			= vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !graphics_storage_buffer_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create storage buffer descriptor set layout!" );
			return false;
		}
	}

	// Compute: Descriptor set layout for basic sampler.
	// Binding 0 = Sampler
	{
		std::array<VkDescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		compute_basic_sampler_descriptor_set_layout = vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !compute_basic_sampler_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return false;
		}
	}

	// Compute: Descriptor set layout for blur textures.
	// Binding 0 = Sampled image
	// Binding 1 = Storage image
	{
		std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		descriptor_set_layout_bindings[ 1 ].binding				= 1;
		descriptor_set_layout_bindings[ 1 ].descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor_set_layout_bindings[ 1 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 1 ].stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;
		descriptor_set_layout_bindings[ 1 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		compute_blur_textures_descriptor_set_layout = vk2d::_internal::CreateDescriptorSetLayout(
			this,
			vk_device,
			&descriptor_set_layout_create_info
		);
		if( !compute_blur_textures_descriptor_set_layout ) {
			Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create blur textures descriptor set layout!" );
			return false;
		}
	}

	return true;
}









bool vk2d::_internal::InstanceImpl::CreatePipelineLayouts()
{
	// Graphics pipeline layout
	{
		// This must match shader layout.
		std::vector<VkDescriptorSetLayout> set_layouts {
			graphics_uniform_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 0 is FrameData.
			graphics_storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 1 is vertex index buffer as storage buffer.
			graphics_storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 2 is vertex buffer as storage buffer.
			graphics_sampler_descriptor_set_layout->GetVulkanDescriptorSetLayout(),			// Pipeline set 3 is sampler and it's data uniform.
			graphics_texture_descriptor_set_layout->GetVulkanDescriptorSetLayout(),			// Pipeline set 4 is texture.
			graphics_storage_buffer_descriptor_set_layout->GetVulkanDescriptorSetLayout()	// Pipeline set 5 is texture channel weight data.
		};

		std::array<VkPushConstantRange, 1> push_constant_ranges {};
		push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_ranges[ 0 ].offset		= 0;
		push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vk2d::_internal::GraphicsPushConstants ) );

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext					= nullptr;
		pipeline_layout_create_info.flags					= 0;
		pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
		pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
		pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
		pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

		auto result = vkCreatePipelineLayout(
			vk_device,
			&pipeline_layout_create_info,
			nullptr,
			&vk_graphics_pipeline_layout
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create Vulkan pipeline layout!" );
			return false;
		}
	}

	// Compute blur pipeline layout
	{
		// This must match shader layout.
		std::vector<VkDescriptorSetLayout> set_layouts {
			compute_basic_sampler_descriptor_set_layout->GetVulkanDescriptorSetLayout(),	// Pipeline set 0 is basic sampler.
			compute_blur_textures_descriptor_set_layout->GetVulkanDescriptorSetLayout()		// Pipeline set 1 is attached textures.
		};

		std::array<VkPushConstantRange, 1> push_constant_ranges {};
		push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_COMPUTE_BIT;
		push_constant_ranges[ 0 ].offset		= 0;
		push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vk2d::_internal::ComputeBlurPushConstants ) );

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext					= nullptr;
		pipeline_layout_create_info.flags					= 0;
		pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
		pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
		pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
		pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

		auto result = vkCreatePipelineLayout(
			vk_device,
			&pipeline_layout_create_info,
			nullptr,
			&vk_compute_blur_pipeline_layout
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create Vulkan pipeline layout!" );
			return false;
		}
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

bool vk2d::_internal::InstanceImpl::PopulateNonStaticallyExposedVulkanFunctions()
{
	vk_fun_vkCmdPushDescriptorSetKHR		= (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr( vk_device, "vkCmdPushDescriptorSetKHR" );
	if( !vk_fun_vkCmdPushDescriptorSetKHR ) {
		Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Instance, cannot get vulkan function: vkCmdPushDescriptorSetKHR()!" );
		return false;
	}

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

void vk2d::_internal::InstanceImpl::DestroyBlurSampler()
{
	FreeDescriptorSet( blur_sampler_descriptor_set );
	blur_sampler			= {};
}

void vk2d::_internal::InstanceImpl::DestroyPipelineCaches()
{
	vkDestroyPipelineCache(
		vk_device,
		vk_graphics_pipeline_cache,
		nullptr
	);
	vk_graphics_pipeline_cache			= {};

	vkDestroyPipelineCache(
		vk_device,
		vk_compute_pipeline_cache,
		nullptr
	);
	vk_compute_pipeline_cache			= {};
}

void vk2d::_internal::InstanceImpl::DestroyPipelines()
{
	for( auto p : vk_graphics_pipelines ) {
		vkDestroyPipeline(
			vk_device,
			p.second,
			nullptr
		);
	}
	vk_graphics_pipelines.clear();

	for( auto p : vk_compute_pipelines ) {
		vkDestroyPipeline(
			vk_device,
			p.second,
			nullptr
		);
	}
	vk_compute_pipelines.clear();
}

void vk2d::_internal::InstanceImpl::DestroyShaderModules()
{
	for( auto s : vk_graphics_shader_modules ) {
		vkDestroyShaderModule(
			vk_device,
			s,
			nullptr
		);
	}
	vk_graphics_shader_modules.clear();
	graphics_shader_programs.clear();

	for( auto s : vk_compute_shader_modules ) {
		vkDestroyShaderModule(
			vk_device,
			s,
			nullptr
		);
	}
	vk_compute_shader_modules.clear();
	compute_shader_programs.clear();
}

void vk2d::_internal::InstanceImpl::DestroyDescriptorSetLayouts()
{
	graphics_sampler_descriptor_set_layout			= nullptr;
	graphics_texture_descriptor_set_layout			= nullptr;
	graphics_uniform_buffer_descriptor_set_layout	= nullptr;
	graphics_storage_buffer_descriptor_set_layout	= nullptr;
	compute_basic_sampler_descriptor_set_layout		= nullptr;
	compute_blur_textures_descriptor_set_layout		= nullptr;
}

void vk2d::_internal::InstanceImpl::DestroyPipelineLayouts()
{
	vkDestroyPipelineLayout(
		vk_device,
		vk_graphics_pipeline_layout,
		nullptr
	);
	vk_graphics_pipeline_layout			= {};

	vkDestroyPipelineLayout(
		vk_device,
		vk_compute_blur_pipeline_layout,
		nullptr
	);
	vk_compute_blur_pipeline_layout		= {};
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
}









std::vector<VkPhysicalDevice> vk2d::_internal::InstanceImpl::EnumeratePhysicalDevices()
{
	auto result = VK_SUCCESS;

	uint32_t physical_device_count		= UINT32_MAX;
	result = vkEnumeratePhysicalDevices(
		vk_instance,
		&physical_device_count,
		nullptr
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot enumerate physical devices!" );
		return {};
	}

	std::vector<VkPhysicalDevice> physical_devices( physical_device_count );
	result = vkEnumeratePhysicalDevices(
		vk_instance,
		&physical_device_count,
		physical_devices.data()
	);
	if( result != VK_SUCCESS ) {
		Report( result, "Internal error: Cannot enumerate physical devices!" );
		return {};
	}

	return physical_devices;
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
