
#include <core/SourceCommon.hpp>

#include "InstanceImpl.hpp"

#include "../monitor/MonitorImpl.hpp"
#include "../cursor/CursorImpl.hpp"

#include <core/SystemConsole.hpp>

#include <vulkan/utils/QueueResolver.hpp>
#include <system/ThreadPool.hpp>
#include <system/ThreadLoaderResource.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>

#include "../resource_manager/ResourceManagerImpl.hpp"
#include "../resources/texture/TextureResourceImpl.hpp"
#include "../window/WindowImpl.hpp"
#include "../sampler/SamplerImpl.hpp"

#include <containers/Color.hpp>

#include <Spir-V/IncludeAllShaders.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



#if VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <Windows.h>
#endif


//Missing constants in header version 131, added in 141
#if (VK_HEADER_VERSION == 131)
	typedef enum MissingVkResult {
		VK_THREAD_IDLE_KHR = 1000268000,
		VK_THREAD_DONE_KHR = 1000268001,
		VK_OPERATION_DEFERRED_KHR = 1000268002,
		VK_OPERATION_NOT_DEFERRED_KHR = 1000268003,
		VK_PIPELINE_COMPILE_REQUIRED_EXT = 1000297000
	} MissingVkResult;

	#define VK_HEADER_VERSION_COMPLETE VK_MAKE_VERSION(1, 2, VK_HEADER_VERSION)
#endif

//Depricated?
#ifndef VK_ERROR_INCOMPATIBLE_VERSION_KHR
	#define	VK_ERROR_INCOMPATIBLE_VERSION_KHR -1000150000
#endif



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private global varibles.
std::mutex					instance_globals_mutex;
Monitor						primary_monitor					= {};
std::vector<Monitor>		all_monitors					= {};
std::list<InstanceImpl*>	instance_listeners				= {};
uint64_t					InstanceImpl::instance_count	= {};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private function declarations.
void VK2D_default_ReportFunction(
	ReportSeverity			severity,
	std::string_view		message );

void UpdateMonitorLists(
	bool					globals_locked		= false );

void glfwMonitorCallback(
	GLFWmonitor			*	monitor,
	int						event );

void glfwJoystickEventCallback(
	int						joystick,
	int						event );



} // vk2d_internal
} // vk2d



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::InstanceImpl::InstanceImpl(
	Instance					&	my_interface,
	const InstanceCreateInfo	&	instance_create_info
) :
	my_interface( my_interface ),
	create_info_copy( instance_create_info ),
	report_function( instance_create_info.report_function ),
	creator_thread_id( std::this_thread::get_id() )
{
	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );

	// Initialize glfw if this is the first instance.
	if( instance_count == 0 ) {
		if( glfwInit() != GLFW_TRUE ) {
			if( instance_create_info.report_function ) {
				instance_create_info.report_function( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot initialize GLFW!" );
			}
			return;
		}
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwSetMonitorCallback( glfwMonitorCallback );
		glfwSetJoystickCallback( glfwJoystickEventCallback );
		UpdateMonitorLists( true );
	}
	++instance_count;

	// Check if vulkan is supported
	if( glfwVulkanSupported() == GLFW_FALSE ) {
		if( instance_create_info.report_function ) {
			instance_create_info.report_function( ReportSeverity::CRITICAL_ERROR, "Vulkan is not supported on this machine!\nUpdating your graphics drivers might fix this issue!" );
		}
		return;
	}

	if( report_function == nullptr ) {
		report_function = vk2d::vk2d_internal::VK2D_default_ReportFunction;
	}

	if( !CreateInstance() ) return;
	if( !CreateDeviceAndQueues() ) return;

	#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	if( instance_count == 1 ) {
		auto & primary_render_queue = vulkan_device->GetQueue( vulkan::QueueType::PRIMARY_RENDER );
		SetCommandBufferCheckpointQueue(
			*vulkan_device,
			primary_render_queue.GetVulkanQueue(),
			primary_render_queue.GetQueueMutex()
		);
	}
	#endif

	if( !PopulateNonStaticallyExposedVulkanFunctions() ) return;
	if( !CreateDescriptorSetLayouts_MOVE() ) return;
	if( !CreateDescriptorPool() ) return;
	if( !CreateShaderModules_DEPRICATED() ) return;
	if( !CreatePipelineLayouts_MOVE() ) return;
	if( !CreateThreadPool() ) return;
	if( !CreateResourceManager() ) return;
	if( !CreateDefaultTexture() ) return;
	if( !CreateDefaultSampler() ) return;
	if( !CreateBlurSampler() ) return;

	instance_listeners.push_back( this );

	is_good				= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::InstanceImpl::~InstanceImpl()
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );
	instance_listeners.push_back( this );

	vkDeviceWaitIdle( *vulkan_device );

	windows.clear();
	render_target_textures.clear();
	cursors.clear();
	samplers.clear();

	DestroyBlurSampler();
	DestroyDefaultSampler();
	DestroyDefaultTexture();

	vk_graphics_pipelines_DEPRICATED.clear();
	vk_compute_pipelines_DEPRICATED.clear();

	DestroyResourceManager();
	DestroyThreadPool();
	DestroyPipelineLayouts_MOVE();
	DestroyShaderModules_DEPRICATED();
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::InstanceCreateInfo vk2d::vk2d_internal::InstanceImpl::GetCreateInfo() const
{
	return create_info_copy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::Run()
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	glfwPollEvents();

	// TODO: Schedule cleanup tasks at vk2d::vk2d_internal::InstanceImpl::Run().

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<vk2d::Monitor*> vk2d::vk2d_internal::InstanceImpl::GetMonitors()
{
	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::GetMonitors() must be called from main thread only!" );
		return {};
	}

	std::vector<Monitor*> ret;
	ret.reserve( vk2d::vk2d_internal::all_monitors.size() );

	for( auto & m : vk2d::vk2d_internal::all_monitors ) {
		ret.push_back( &m );
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Monitor * vk2d::vk2d_internal::InstanceImpl::GetPrimaryMonitor() const
{
	return &vk2d::vk2d_internal::primary_monitor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::SetMonitorUpdateCallback(
	PFN_MonitorUpdateCallback		monitor_update_callback_funtion
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::SetMonitorUpdateCallback() must be called from main thread only!" );
		return;
	}

	monitor_update_callback = monitor_update_callback_funtion;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Cursor * vk2d::vk2d_internal::InstanceImpl::CreateCursor(
	const std::filesystem::path		&	image_path,
	glm::ivec2							hot_spot
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::CreateCursor() must be called from main thread only!" );
		return {};
	}

	auto cursor = std::unique_ptr<Cursor>(
		new Cursor(
			*this,
			image_path,
			hot_spot
		)
	);
	if( cursor && cursor->IsGood() ) {
		auto ret = cursor.get();
		cursors.push_back( std::move( cursor ) );
		return ret;
	} else {
		Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Cursor * vk2d::vk2d_internal::InstanceImpl::CreateCursor(
	glm::uvec2						image_size,
	const std::vector<Color8>	&	image_data,
	glm::ivec2						hot_spot
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::CreateCursor() must be called from main thread only!" );
		return {};
	}

	auto cursor = std::unique_ptr<Cursor>(
		new Cursor(
			*this,
			image_size,
			image_data,
			hot_spot
		)
	);
	if( cursor && cursor->IsGood() ) {
		auto ret = cursor.get();
		cursors.push_back( std::move( cursor ) );
		return ret;
	} else {
		Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyCursor(
	Cursor		*	cursor
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::DestroyCursor() must be called from main thread only!" );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::PFN_GamepadConnectionEventCallback vk2d::vk2d_internal::InstanceImpl::GetGamepadEventCallback() const
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	return joystick_event_callback;
}

void vk2d::vk2d_internal::InstanceImpl::SetGamepadEventCallback(
	PFN_GamepadConnectionEventCallback		joystick_event_callback_function
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::SetGamepadEventCallback() must be called from main thread only!" );
		return;
	}

	joystick_event_callback		= joystick_event_callback_function;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::IsGamepadPresent( Gamepad joystick )
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::IsGamepadPresent() must be called from main thread only!" );
		return {};
	}

	if( glfwJoystickIsGamepad( int( joystick ) ) == GLFW_TRUE ) {
		return true;
	} else {
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vk2d_internal::InstanceImpl::GetGamepadName( Gamepad gamepad )
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::GetGamepadName() must be called from main thread only!" );
		return {};
	}

	return glfwGetGamepadName( int( gamepad ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::GamepadState vk2d::vk2d_internal::InstanceImpl::QueryGamepadState(
	Gamepad			gamepad )
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::QueryGamepadState() must be called from main thread only!" );
		return {};
	}

	GamepadState	gamepad_state {};
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Window * vk2d::vk2d_internal::InstanceImpl::CreateOutputWindow(
	const WindowCreateInfo	&	window_create_info
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::CreateOutputWindow() must be called from main thread only!" );
		return {};
	}

	auto new_window = std::unique_ptr<Window>(
		new Window(
			*this,
			window_create_info
		)
	);

	if( new_window->IsGood() ) {
		windows.push_back( std::move( new_window ) );
		return windows.back().get();
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyOutputWindow(
	Window				*	window
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::DestroyOutputWindow() must be called from main thread only!" );
		return;
	}

	vkDeviceWaitIdle( *vulkan_device );

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::RenderTargetTexture * vk2d::vk2d_internal::InstanceImpl::CreateRenderTargetTexture(
	const RenderTargetTextureCreateInfo		&	render_target_texture_create_info
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::CreateRenderTargetTexture() must be called from main thread only!" );
		return {};
	}

	auto render_target_texture = std::unique_ptr<RenderTargetTexture>(
		new RenderTargetTexture(
			*this,
			render_target_texture_create_info
		)
	);

	if( render_target_texture && render_target_texture->IsGood() ) {
		auto ret	= render_target_texture.get();
		render_target_textures.push_back( std::move( render_target_texture ) );
		return ret;
	} else {
		return nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyRenderTargetTexture(
	RenderTargetTexture						*	render_target_texture
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::DestroyRenderTargetTexture() must be called from main thread only!" );
		return;
	}

	auto result = vkDeviceWaitIdle(
		*vulkan_device
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PoolDescriptorSet vk2d::vk2d_internal::InstanceImpl::AllocateDescriptorSet_DEPRICATED(
	const vulkan::DescriptorSetLayout & for_descriptor_set_layout
)
{
	std::lock_guard<std::mutex> lock_guard( descriptor_pool_mutex );

	return descriptor_pool->AllocateDescriptorSet(
		for_descriptor_set_layout
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::FreeDescriptorSet_DEPRICATED(
	vulkan::PoolDescriptorSet & descriptor_set
)
{
	std::lock_guard<std::mutex> lock_guard( descriptor_pool_mutex );

	descriptor_pool->FreeDescriptorSet(
		descriptor_set
	);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Sampler * vk2d::vk2d_internal::InstanceImpl::CreateSampler(
	const SamplerCreateInfo	&	sampler_create_info
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::CreateSampler() must be called from main thread only!" );
		return {};
	}

	auto sampler = std::unique_ptr<Sampler>(
		new Sampler(
			*this,
			sampler_create_info
		)
	);

	if( sampler && sampler->IsGood() ) {
		auto ret	= sampler.get();
		samplers.push_back( std::move( sampler ) );
		return ret;
	} else {
		return nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroySampler(
	Sampler					*	sampler
)
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	if( !IsThisThreadCreatorThread() ) {
		Report( ReportSeverity::WARNING, "Instance::DestroySampler() must be called from main thread only!" );
		return;
	}

	auto result = vkDeviceWaitIdle(
		*vulkan_device
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Multisamples vk2d::vk2d_internal::InstanceImpl::GetMaximumSupportedMultisampling() const
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	Multisamples max_samples	= Multisamples( vulkan_device->GetVulkanPhysicalDeviceProperties().limits.framebufferColorSampleCounts );
	if( uint32_t( max_samples )			& uint32_t( Multisamples::SAMPLE_COUNT_64 ) )	return Multisamples::SAMPLE_COUNT_64;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_32 ) )	return Multisamples::SAMPLE_COUNT_32;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_16 ) )	return Multisamples::SAMPLE_COUNT_16;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_8 ) )	return Multisamples::SAMPLE_COUNT_8;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_4 ) )	return Multisamples::SAMPLE_COUNT_4;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_2 ) )	return Multisamples::SAMPLE_COUNT_2;
	else if( uint32_t( max_samples )	& uint32_t( Multisamples::SAMPLE_COUNT_1 ) )	return Multisamples::SAMPLE_COUNT_1;
	return Multisamples::SAMPLE_COUNT_1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Multisamples vk2d::vk2d_internal::InstanceImpl::GetAllSupportedMultisampling() const
{
	VK2D_ASSERT_MAIN_THREAD( *this );

	return Multisamples( vulkan_device->GetVulkanPhysicalDeviceProperties().limits.framebufferColorSampleCounts);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::PFN_VK2D_ReportFunction vk2d::vk2d_internal::InstanceImpl::GetReportFunction() const
{
	return report_function;
}









////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::Report(
	VkResult				vk_result,
	ReportSeverity			severity,
	std::string_view		message
) const
{
	if( report_function )
	{
		if( vk_result == VK_ERROR_DEVICE_LOST ) severity = ReportSeverity::DEVICE_LOST;

		Report(
			severity,
			message
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::Report(
	VkResult				vk_result,
	std::string_view		message
) const
{
	if( report_function )
	{
		auto severity = ReportSeverity::NONE;

		switch( vk_result ) {
			case VK_SUCCESS:
			case VK_THREAD_IDLE_KHR:
			case VK_THREAD_DONE_KHR:
			case VK_OPERATION_DEFERRED_KHR:
			case VK_OPERATION_NOT_DEFERRED_KHR:
				severity = ReportSeverity::VERBOSE;
				break;

			case VK_PIPELINE_COMPILE_REQUIRED_EXT:
				severity = ReportSeverity::INFO;
				break;

			case VK_NOT_READY:
			case VK_TIMEOUT:
			case VK_EVENT_SET:
			case VK_EVENT_RESET:
			case VK_INCOMPLETE:
			case VK_SUBOPTIMAL_KHR:
				severity = ReportSeverity::WARNING;
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
				severity = ReportSeverity::NON_CRITICAL_ERROR;
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
				severity = ReportSeverity::CRITICAL_ERROR;
				break;

			case VK_ERROR_DEVICE_LOST:
				severity = ReportSeverity::DEVICE_LOST;
				break;

			default:
				severity = ReportSeverity::CRITICAL_ERROR;
				break;
		}

		Report(
			severity,
			message
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::Report(
	ReportSeverity			severity,
	std::string_view		message
) const
{
	if( report_function ) {
		std::lock_guard<std::mutex> report_lock( report_mutex );

		report_function(
			severity,
			message
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::ThreadPool * vk2d::vk2d_internal::InstanceImpl::GetThreadPool()
{
	return &thread_pool.value();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<uint32_t> & vk2d::vk2d_internal::InstanceImpl::GetLoaderThreads() const
{
	return loader_threads;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t vk2d::vk2d_internal::InstanceImpl::GetMaterialLoaderThread() const
{
	return material_loader_thread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<uint32_t> & vk2d::vk2d_internal::InstanceImpl::GetGeneralThreads() const
{
	return general_threads;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceManager * vk2d::vk2d_internal::InstanceImpl::GetResourceManager() const
{
	return resource_manager.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::Instance & vk2d::vk2d_internal::InstanceImpl::GetVulkanInstance()
{
	return *vulkan_instance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::Device & vk2d::vk2d_internal::InstanceImpl::GetVulkanDevice()
{
	return *vulkan_device;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::GraphicsShaderList vk2d::vk2d_internal::InstanceImpl::GetGraphicsShaderList_DEPRICATED(
	vulkan::GraphicsShaderListID_DEPRICATED id
) const
{
	auto collection = graphics_shader_programs_DEPRICATED.find( id );
	if( collection != graphics_shader_programs_DEPRICATED.end() ) {
		return collection->second;
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkShaderModule vk2d::vk2d_internal::InstanceImpl::GetComputeShaderModules_DEPRICATED(
	vulkan::ComputeShaderProgramID_DEPRICATED id
) const
{
	auto collection = compute_shader_programs_DEPRICATED.find( id );
	if( collection != compute_shader_programs_DEPRICATED.end() ) {
		return collection->second;
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::GraphicsShaderList vk2d::vk2d_internal::InstanceImpl::GetCompatibleGraphicsShaderList_DEPRICATED(
	bool				custom_uv_border_color,
	uint32_t			vertices_per_primitive
) const
{
	if( custom_uv_border_color ) {
		return GetGraphicsShaderList_DEPRICATED( vulkan::GraphicsShaderListID_DEPRICATED::SINGLE_TEXTURED_UV_BORDER_COLOR );
	} else {
		return GetGraphicsShaderList_DEPRICATED( vulkan::GraphicsShaderListID_DEPRICATED::SINGLE_TEXTURED );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipeline vk2d::vk2d_internal::InstanceImpl::GetGraphicsPipeline_DEPRICATED(
	const vulkan::GraphicsPipelineCreateInfo & graphics_pipeline_info
)
{
	// DEPRICATED: Remove this function later.

	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	// TODO: Potential multithreaded access conflict, don't know if this will be a requirement at this point.
	// Need mutex here as this function can be called from multiple threads.
	// Alternatively we could create vulkan pipelines per window and render
	// target texture, but that would probably be wasteful. Regardless this
	// function will be called very often so we could benefit from caching
	// these pipelines within the windows and render target textures.
	// Since the pipelines won't change throughout the execution a simple
	// std::map per window and render target with a pipeline pointer
	// would probably work well, if the pipeline is not found in the local
	// cache, use this function to get it / create it.
	auto p_it = vk_graphics_pipelines_DEPRICATED.find( graphics_pipeline_info );
	if( p_it != vk_graphics_pipelines_DEPRICATED.end() ) {
		return p_it->second;
	}
	auto pipeline = vulkan_device->GetPipelineManager().GetGraphicsPipeline( graphics_pipeline_info );
	vk_graphics_pipelines_DEPRICATED[ graphics_pipeline_info ] = pipeline;
	return pipeline.GetVulkanPipeline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipeline vk2d::vk2d_internal::InstanceImpl::GetComputePipeline_DEPRICATED(
	const vulkan::ComputePipelineCreateInfo & compute_pipeline_info
)
{
	// DEPRICATED: Remove this function later.

	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	// TODO: Potential multithreaded access conflict, don't know if this will be a requirement at this point.
	// Need mutex here as this function can be called from multiple threads.
	// Alternatively we could create vulkan pipelines per window and render
	// target texture, but that would probably be wasteful. Regardless this
	// function will be called very often so we could benefit from caching
	// these pipelines within the windows and render target textures.
	// Since the pipelines won't change throughout the execution a simple
	// std::map per window and render target with a pipeline pointer
	// would probably work well, if the pipeline is not found in the local
	// cache, use this function to get it / create it.
	auto p_it = vk_compute_pipelines_DEPRICATED.find( compute_pipeline_info );
	if( p_it != vk_compute_pipelines_DEPRICATED.end() ) {
		return p_it->second;
	}

	auto pipeline = vulkan_device->GetPipelineManager().GetComputePipeline( compute_pipeline_info );
	vk_compute_pipelines_DEPRICATED[ compute_pipeline_info ] = pipeline;
	return pipeline.GetVulkanPipeline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipelineLayout vk2d::vk2d_internal::InstanceImpl::GetGraphicsPrimaryRenderPipelineLayout_MOVE() const
{
	return vk_graphics_primary_render_pipeline_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipelineLayout vk2d::vk2d_internal::InstanceImpl::GetGraphicsBlurPipelineLayout_DEPRICATED() const
{
	return vk_graphics_blur_pipeline_layout_DEPRICATED;
}

/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipelineLayout vk2d::vk2d_internal::InstanceImpl::GetComputeBlurPipelineLayout() const
{
	return vk_compute_blur_pipeline_layout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetComputeBasicSamplerDescriptorSetLayout() const
{
	return *compute_basic_sampler_descriptor_set_layout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetComputeBlurTexturesDescriptorSetLayout() const
{
	return *compute_blur_textures_descriptor_set_layout;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::vulkan::DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetGraphicsSamplerDescriptorSetLayout_MOVE() const
{
	return *graphics_sampler_descriptor_set_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::vulkan::DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetGraphicsTextureDescriptorSetLayout_MOVE() const
{
	return *graphics_texture_descriptor_set_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::vulkan::DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetGraphicsRenderTargetBlurTextureDescriptorSetLayout_DEPRICATED() const
{
	return *graphics_render_target_blur_texture_descriptor_set_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::vulkan::DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetGraphicsUniformBufferDescriptorSetLayout_MOVE() const
{
	return *graphics_uniform_buffer_descriptor_set_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::vulkan::DescriptorSetLayout & vk2d::vk2d_internal::InstanceImpl::GetGraphicsStorageBufferDescriptorSetLayout_MOVE() const
{
	return *graphics_storage_buffer_descriptor_set_layout_MOVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Texture * vk2d::vk2d_internal::InstanceImpl::GetDefaultTexture()
{
	return &*default_texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Sampler * vk2d::vk2d_internal::InstanceImpl::GetDefaultSampler()
{
	return default_sampler.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkDescriptorSet vk2d::vk2d_internal::InstanceImpl::GetBlurSamplerDescriptorSet_DEPRICATED() const
{
	return blur_sampler_descriptor_set_DEPRICATED.descriptorSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::thread::id vk2d::vk2d_internal::InstanceImpl::GetCreatorThreadID() const
{
	return creator_thread_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::IsThisThreadCreatorThread() const
{
	return creator_thread_id == std::this_thread::get_id();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::VkFun_vkCmdPushDescriptorSetKHR(
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::IsGood() const
{
	return is_good;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateInstance()
{
	vulkan_instance.emplace(
		*this
	);
	if( !vulkan_instance->IsGood() ) {
		Report( ReportSeverity::CRITICAL_ERROR, "Cannot create Vulkan instance." );
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateDeviceAndQueues()
{
	vulkan_device.emplace(
		*this,
		vulkan_instance->PickBestVulkanPhysicalDevice() // TODO: Make physical device user selectable.
	);
	if( !vulkan_device->IsGood() ) {
		Report( ReportSeverity::CRITICAL_ERROR, "Cannot create Vulkan device." );
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateDescriptorPool()
{
	descriptor_pool = std::make_unique<vulkan::DescriptorAutoPool>(
		this,
		*vulkan_device
	);

	if( descriptor_pool ) {
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateDefaultSampler()
{
	SamplerCreateInfo sampler_create_info {};
	sampler_create_info.minification_filter				= SamplerFilter::LINEAR;
	sampler_create_info.magnification_filter			= SamplerFilter::LINEAR;
	sampler_create_info.mipmap_mode						= SamplerMipmapMode::LINEAR;
	sampler_create_info.address_mode_u					= SamplerAddressMode::REPEAT;
	sampler_create_info.address_mode_v					= SamplerAddressMode::REPEAT;
	sampler_create_info.border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
	sampler_create_info.anisotropy_enable				= true;
	sampler_create_info.mipmap_max_anisotropy			= 16.0f;
	sampler_create_info.mipmap_level_of_detail_bias		= 0.0f;
	sampler_create_info.mipmap_min_level_of_detail		= 0.0f;
	sampler_create_info.mipmap_max_level_of_detail		= 128.0f;
	default_sampler = std::unique_ptr<Sampler>(
		new Sampler(
			*this,
			sampler_create_info
		)
	);
	if( default_sampler && default_sampler->IsGood() ) {
		return true;
	} else {
		default_sampler = nullptr;
		Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create default sampler!" );
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateBlurSampler()
{
	{
		SamplerCreateInfo sampler_create_info {};
		sampler_create_info.minification_filter				= SamplerFilter::LINEAR;
		sampler_create_info.magnification_filter			= SamplerFilter::LINEAR;
		sampler_create_info.mipmap_mode						= SamplerMipmapMode::NEAREST;
		sampler_create_info.address_mode_u					= SamplerAddressMode::CLAMP_TO_EDGE;
		sampler_create_info.address_mode_v					= SamplerAddressMode::CLAMP_TO_EDGE;
		sampler_create_info.border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
		sampler_create_info.anisotropy_enable				= false;
		sampler_create_info.mipmap_max_anisotropy			= 1.0f;
		sampler_create_info.mipmap_level_of_detail_bias		= 0.0f;
		sampler_create_info.mipmap_min_level_of_detail		= 0.0f;
		sampler_create_info.mipmap_max_level_of_detail		= 128.0f;
		blur_Sampler = std::unique_ptr<Sampler>(
			new Sampler(
				*this,
				sampler_create_info
			)
		);
		if( !blur_Sampler || !blur_Sampler->IsGood() ) {
			blur_Sampler		= nullptr;
			Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create blur sampler!" );
			return false;
		}
	}

	{
		blur_sampler_descriptor_set_DEPRICATED = AllocateDescriptorSet_DEPRICATED( *graphics_simple_sampler_descriptor_set_layout_MOVE );
		if( blur_sampler_descriptor_set_DEPRICATED != VK_SUCCESS ) {
			FreeDescriptorSet_DEPRICATED( blur_sampler_descriptor_set_DEPRICATED );
			Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create blur sampler descriptor set!" );
			return false;
		}
	}

	{
		VkDescriptorImageInfo image_info {};
		image_info.sampler		= blur_Sampler->impl->GetVulkanSampler();
		image_info.imageView	= VK_NULL_HANDLE;
		image_info.imageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;

		VkWriteDescriptorSet descriptor_write {};
		descriptor_write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.pNext				= nullptr;
		descriptor_write.dstSet				= blur_sampler_descriptor_set_DEPRICATED.descriptorSet;
		descriptor_write.dstBinding			= 0;
		descriptor_write.dstArrayElement	= 0;
		descriptor_write.descriptorCount	= 1;
		descriptor_write.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_write.pImageInfo			= &image_info;
		descriptor_write.pBufferInfo		= nullptr;
		descriptor_write.pTexelBufferView	= nullptr;
		vkUpdateDescriptorSets(
			*vulkan_device,
			1, &descriptor_write,
			0, nullptr
		);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateShaderModules_DEPRICATED()
{
	auto CreateModule = [this](
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
			*vulkan_device,
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



		// Create individual shader modules for render target texture blur passes.
		auto render_target_texture_blur_vertex								= CreateModule(
			RenderTargetTextureBlurVertex_vert_shader_data.data(),
			RenderTargetTextureBlurVertex_vert_shader_data.size()
		);

		auto render_target_texture_fragment_box_blur_horisontal				= CreateModule(
			RenderTargetTexture_BoxBlur_Horisontal_frag_shader_data.data(),
			RenderTargetTexture_BoxBlur_Horisontal_frag_shader_data.size()
		);
		auto render_target_texture_fragment_box_blur_vertical				= CreateModule(
			RenderTargetTexture_BoxBlur_Vertical_frag_shader_data.data(),
			RenderTargetTexture_BoxBlur_Vertical_frag_shader_data.size()
		);

		auto render_target_texture_fragment_gaussian_blur_horisontal		= CreateModule(
			RenderTargetTexture_GaussianBlur_Horisontal_frag_shader_data.data(),
			RenderTargetTexture_GaussianBlur_Horisontal_frag_shader_data.size()
		);
		auto render_target_texture_fragment_gaussian_blur_vertical			= CreateModule(
			RenderTargetTexture_GaussianBlur_Vertical_frag_shader_data.data(),
			RenderTargetTexture_GaussianBlur_Vertical_frag_shader_data.size()
		);



		// List all individual shader modules into a vector
		vk_graphics_shader_modules_DEPRICATED.push_back( single_textured_vertex );
		vk_graphics_shader_modules_DEPRICATED.push_back( single_textured_fragment );
		vk_graphics_shader_modules_DEPRICATED.push_back( single_textured_fragment_uv_border_color );

		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_vertex );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_triangle );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_line );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_point );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_triangle_uv_border_color );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_line_uv_border_color );
		vk_graphics_shader_modules_DEPRICATED.push_back( multitextured_fragment_point_uv_border_color );

		vk_graphics_shader_modules_DEPRICATED.push_back( render_target_texture_blur_vertex );
		vk_graphics_shader_modules_DEPRICATED.push_back( render_target_texture_fragment_box_blur_horisontal );
		vk_graphics_shader_modules_DEPRICATED.push_back( render_target_texture_fragment_box_blur_vertical );
		vk_graphics_shader_modules_DEPRICATED.push_back( render_target_texture_fragment_gaussian_blur_horisontal );
		vk_graphics_shader_modules_DEPRICATED.push_back( render_target_texture_fragment_gaussian_blur_vertical );

		// Collect a listing of shader units, which is a collection of shader modules needed to create a pipeline.
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::SINGLE_TEXTURED ]						= vulkan::GraphicsShaderList( single_textured_vertex, single_textured_fragment );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::SINGLE_TEXTURED_UV_BORDER_COLOR ]		= vulkan::GraphicsShaderList( single_textured_vertex, single_textured_fragment_uv_border_color );

		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_TRIANGLE ]				= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_triangle );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_LINE ]					= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_line );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_POINT ]					= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_point );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_TRIANGLE_UV_BORDER_COLOR ]= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_triangle_uv_border_color );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_LINE_UV_BORDER_COLOR ]	= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_line_uv_border_color );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::MULTITEXTURED_POINT_UV_BORDER_COLOR ]	= vulkan::GraphicsShaderList( multitextured_vertex, multitextured_fragment_point_uv_border_color );

		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::RENDER_TARGET_BOX_BLUR_HORISONTAL ]		= vulkan::GraphicsShaderList( render_target_texture_blur_vertex, render_target_texture_fragment_box_blur_horisontal );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::RENDER_TARGET_BOX_BLUR_VERTICAL ]		= vulkan::GraphicsShaderList( render_target_texture_blur_vertex, render_target_texture_fragment_box_blur_vertical );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::RENDER_TARGET_GAUSSIAN_BLUR_HORISONTAL ]= vulkan::GraphicsShaderList( render_target_texture_blur_vertex, render_target_texture_fragment_gaussian_blur_horisontal );
		graphics_shader_programs_DEPRICATED[ vulkan::GraphicsShaderListID_DEPRICATED::RENDER_TARGET_GAUSSIAN_BLUR_VERTICAL ]	= vulkan::GraphicsShaderList( render_target_texture_blur_vertex, render_target_texture_fragment_gaussian_blur_vertical );
	}

	////////////////////////////////
	// Compute shaders
	////////////////////////////////

	{
		// Nothing yet.
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateDescriptorSetLayouts_MOVE()
{
	// TODO: Creating descriptor sets could be streamlined some, consider creating descriptor set layouts on the fly as needed for each shader.
	// Checking, changing and making sure descriptor set layouts match the shaders is a bit tedious and prone to errors. So we could use a system
	// that would automate this based on parameters when creating/choosing the pipeline for example, though this could eventually introduce some
	// microstutter if we create everything on the fly as needed, so this will require some thinking first. One possible solution is to create
	// some pipelines and descriptor set layouts ahead of time based on some commonly used parameters, another is to just loop through each and
	// every possible settings iteration and create everything ahead of time but this could make opening the application slower (maybe create them
	// asyncronously on another thread?) and could potentially create a lot of unneeded pipelines wasting memory.
	// For now we're just going to leave it as is, however we could make the code a bit shorter as we don't need most of the CreateInfo structure
	// options.

	// These must match shader set types.

	struct Binding
	{
		VkDescriptorType		descriptor_type;
		VkShaderStageFlags		shader_stage_flags;
	};
	auto CreateLocalDescriptorSetLayout			= [this](
		const std::vector<Binding>			&	bindings,
		VkDescriptorSetLayoutCreateFlags		flags			= 0
		) -> std::unique_ptr<vulkan::DescriptorSetLayout>
	{
		std::vector<VkDescriptorSetLayoutBinding> set_bindings( std::size( bindings ) );
		for( size_t i = 0; i < std::size( bindings ); ++i ) {

			auto & src = bindings[ i ];
			auto & dst = set_bindings[ i ];

			dst.binding				= uint32_t( i );
			dst.descriptorType		= src.descriptor_type;
			dst.descriptorCount		= 1;
			dst.stageFlags			= src.shader_stage_flags;
			dst.pImmutableSamplers	= nullptr;
		}

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= flags;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( std::size( set_bindings ) );
		descriptor_set_layout_create_info.pBindings		= set_bindings.data();

		auto descriptor_set_layout = std::make_unique<vulkan::DescriptorSetLayout>(
			this,
			*vulkan_device,
			&descriptor_set_layout_create_info
		);
		if( !descriptor_set_layout ) {
			Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create sampler descriptor set layout!" );
			return {};
		}

		return descriptor_set_layout;
	};

	// Graphics: Descriptor set layout for simple sampler.
	// Binding 0 = Sampler
	{
		graphics_simple_sampler_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		);
		if( !graphics_simple_sampler_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	// Graphics: Descriptor set layout for sampler.
	// Binding 0 = Sampler
	// Binding 1 = Uniform buffer for sampler data
	{
		graphics_sampler_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		);
		if( !graphics_sampler_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	// Graphics: Descriptor set layout for texture.
	// Binding 0 = Sampled image
	{
		graphics_texture_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		);
		if( !graphics_texture_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	// Graphics: Descriptor set layout for texture meant for render target texture blur shader.
	// Binding 0 = Sampled image
	{
		graphics_render_target_blur_texture_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT }
			},
			VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR
		);
		if( !graphics_render_target_blur_texture_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	// Graphics: General uniform buffer descriptor set layout
	{
		graphics_uniform_buffer_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		);
		if( !graphics_uniform_buffer_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	// Graphics: General storage buffer descriptor set layout
	{
		graphics_storage_buffer_descriptor_set_layout_MOVE = CreateLocalDescriptorSetLayout(
			{
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		);
		if( !graphics_storage_buffer_descriptor_set_layout_MOVE ) {
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreatePipelineLayouts_MOVE()
{
	// Graphics primary render pipeline layout.
	{
		// TODO: We're currently using a lot of individual descriptor sets for primary render, should combine some of these.
		// Might look more into automating the descriptor set layout creation on the fly.

		// This must match shader layout.
		std::vector<VkDescriptorSetLayout> set_layouts {
			graphics_uniform_buffer_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),	// Pipeline set 0 is FrameData.
			graphics_storage_buffer_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),	// Pipeline set 1 is Transformation data.
			graphics_storage_buffer_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),	// Pipeline set 2 is index buffer as storage buffer.
			graphics_storage_buffer_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),	// Pipeline set 3 is vertex buffer as storage buffer.
			graphics_sampler_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),			// Pipeline set 4 is sampler and it's data uniform.
			graphics_texture_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),			// Pipeline set 5 is texture.
		};

		std::array<VkPushConstantRange, 1> push_constant_ranges {};
		push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_ranges[ 0 ].offset		= 0;
		push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vulkan::GraphicsPrimaryRenderPushConstants ) );

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext					= nullptr;
		pipeline_layout_create_info.flags					= 0;
		pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
		pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
		pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
		pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

		auto result = vkCreatePipelineLayout(
			*vulkan_device,
			&pipeline_layout_create_info,
			nullptr,
			&vk_graphics_primary_render_pipeline_layout_MOVE
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create Vulkan pipeline layout!" );
			return false;
		}
	}

	// Graphics blur pipeline layout.
	{
		// This must match shader layout.
		std::vector<VkDescriptorSetLayout> set_layouts {
			graphics_simple_sampler_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),				// Pipeline set 0 is a simple sampler.
			graphics_render_target_blur_texture_descriptor_set_layout_MOVE->GetVulkanDescriptorSetLayout(),	// Pipeline set 1 is texture.
		};

		std::array<VkPushConstantRange, 1> push_constant_ranges {};
		push_constant_ranges[ 0 ].stageFlags	= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_ranges[ 0 ].offset		= 0;
		push_constant_ranges[ 0 ].size			= uint32_t( sizeof( vulkan::GraphicsBlurPushConstants ) );

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext					= nullptr;
		pipeline_layout_create_info.flags					= 0;
		pipeline_layout_create_info.setLayoutCount			= uint32_t( set_layouts.size() );
		pipeline_layout_create_info.pSetLayouts				= set_layouts.data();
		pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
		pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

		auto result = vkCreatePipelineLayout(
			*vulkan_device,
			&pipeline_layout_create_info,
			nullptr,
			&vk_graphics_blur_pipeline_layout_DEPRICATED
		);
		if( result != VK_SUCCESS ) {
			Report( result, "Internal error: Cannot create Vulkan pipeline layout!" );
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateThreadPool()
{
	uint32_t thread_count					= uint32_t( std::thread::hardware_concurrency() );
	if( thread_count == 0 ) thread_count	= 8;
	thread_count -= 1;

	uint32_t loader_thread_count = create_info_copy.resource_loader_thread_count;
	if( loader_thread_count == UINT32_MAX )		loader_thread_count		= thread_count / 2;
	if( loader_thread_count > thread_count )	loader_thread_count		= thread_count;
	if( loader_thread_count == 0 )				loader_thread_count		= 1;

	uint32_t general_thread_count = 1;

	std::vector<std::unique_ptr<LocalThreadData>> thread_resources;
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<ThreadLoaderResource>( *this ) );
	}
	thread_resources.push_back( std::make_unique<ThreadMaterialLoaderResource>( *this ) );
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<ThreadGeneralResource>() );
	}

	loader_threads.resize( loader_thread_count );
	general_threads.resize( general_thread_count );
	uint32_t thread_src_index = 0;
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		loader_threads[ i ] = thread_src_index++;
	}
	material_loader_thread = thread_src_index++;
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		general_threads[ i ] = thread_src_index++;
	}

	thread_pool.emplace( std::move( thread_resources ) );
	if( thread_pool && thread_pool->IsGood() ) {
		return true;
	} else {
		Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create thread pool!" );
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateResourceManager()
{
	resource_manager = std::unique_ptr<ResourceManager>(
		new ResourceManager(
			*this
		)
	);
	if( resource_manager && resource_manager->IsGood() ) {
		return true;
	} else {
		Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create resource manager!" );
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::CreateDefaultTexture()
{
	default_texture		= resource_manager->CreateTextureResource(
		glm::uvec2( 1, 1 ),
		{ Color8( 255, 255, 255, 255 ) }
	);
	default_texture->WaitUntilLoaded();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::InstanceImpl::PopulateNonStaticallyExposedVulkanFunctions()
{
	vk_fun_vkCmdPushDescriptorSetKHR		= (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr( *vulkan_device, "vkCmdPushDescriptorSetKHR" );
	if( !vk_fun_vkCmdPushDescriptorSetKHR ) {
		Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Instance, cannot get vulkan function: vkCmdPushDescriptorSetKHR()!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyInstance()
{
	vulkan_instance.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyDevice()
{
	vulkan_device.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyDescriptorPool()
{
	descriptor_pool			= nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyDefaultSampler()
{
	default_sampler			= {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyBlurSampler()
{
	FreeDescriptorSet_DEPRICATED( blur_sampler_descriptor_set_DEPRICATED );
	blur_Sampler			= {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyShaderModules_DEPRICATED()
{
	for( auto s : vk_graphics_shader_modules_DEPRICATED ) {
		vkDestroyShaderModule(
			*vulkan_device,
			s,
			nullptr
		);
	}
	vk_graphics_shader_modules_DEPRICATED.clear();
	graphics_shader_programs_DEPRICATED.clear();

	for( auto s : vk_compute_shader_modules_DEPRICATED ) {
		vkDestroyShaderModule(
			*vulkan_device,
			s,
			nullptr
		);
	}
	vk_compute_shader_modules_DEPRICATED.clear();
	compute_shader_programs_DEPRICATED.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyDescriptorSetLayouts()
{
	graphics_simple_sampler_descriptor_set_layout_MOVE				= nullptr;
	graphics_sampler_descriptor_set_layout_MOVE						= nullptr;
	graphics_texture_descriptor_set_layout_MOVE						= nullptr;
	graphics_render_target_blur_texture_descriptor_set_layout_MOVE	= nullptr;
	graphics_uniform_buffer_descriptor_set_layout_MOVE				= nullptr;
	graphics_storage_buffer_descriptor_set_layout_MOVE				= nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyPipelineLayouts_MOVE()
{
	vkDestroyPipelineLayout(
		*vulkan_device,
		vk_graphics_primary_render_pipeline_layout_MOVE,
		nullptr
	);
	vk_graphics_primary_render_pipeline_layout_MOVE = {};

	vkDestroyPipelineLayout(
		*vulkan_device,
		vk_graphics_blur_pipeline_layout_DEPRICATED,
		nullptr
	);
	vk_graphics_blur_pipeline_layout_DEPRICATED = {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyThreadPool()
{
	thread_pool.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyResourceManager()
{
	resource_manager		= {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::InstanceImpl::DestroyDefaultTexture()
{
	default_texture			= {};
}



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VK2D_default_ReportFunction(
	ReportSeverity			severity,
	std::string_view		message
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	auto text_color			= ConsoleColor::DEFAULT;
	auto background_color	= ConsoleColor::DEFAULT;

	switch( severity ) {
	case ReportSeverity::NONE:
		break;
	case ReportSeverity::VERBOSE:
		text_color = ConsoleColor::DARK_GRAY;
		break;
	case ReportSeverity::INFO:
		text_color = ConsoleColor::DARK_GREEN;
		break;
	case ReportSeverity::PERFORMANCE_WARNING:
		text_color = ConsoleColor::BLUE;
		break;
	case ReportSeverity::WARNING:
		text_color = ConsoleColor::DARK_YELLOW;
		break;
	case ReportSeverity::NON_CRITICAL_ERROR:
		text_color = ConsoleColor::RED;
		break;
	case ReportSeverity::CRITICAL_ERROR:
	case ReportSeverity::DEVICE_LOST:
		text_color			= ConsoleColor::WHITE;
		background_color	= ConsoleColor::DARK_RED;
		break;
	case ReportSeverity::DEBUG:
		text_color			= ConsoleColor::CYAN;
		break;
	default:
		break;
	}

	auto editable_message = std::string();
	editable_message.reserve( size_t( message.size() * 1.2f + 10 ) );
	{
		auto tab_size = size_t( 4 );
		auto cursor_pos = size_t();
		for( auto c : message ) {
			if( c == '\t' )
			{
				auto add_spaces = ( cursor_pos / tab_size + 1 ) * tab_size - cursor_pos;
				editable_message.append( add_spaces, ' ' );
				cursor_pos += add_spaces;
			}
			else if( c == '\n' )
			{
				editable_message.push_back( c );
				cursor_pos = 0;
			}
			else
			{
				editable_message.push_back( c );
				++cursor_pos;
			}
		}
	}
	editable_message += "\n";
	ConsolePrint( editable_message, text_color, background_color );

	#if VK2D_BUILD_OPTION_VULKAN_COMMAND_BUFFER_CHECKMARKS && VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	if( severity == ReportSeverity::DEVICE_LOST ) {

		auto checkpoints = GetCommandBufferCheckpoints();
		if( checkpoints.size() ) {
			{
				std::stringstream ss;
				ss << "\n\nLatest command buffer checkpoint marker stages: " << checkpoints.size() << "\n";
				ConsolePrint( ss.str() );
			}
			for( auto & c : checkpoints ) {
				auto data = reinterpret_cast<CommandBufferCheckpointData*>( c.pCheckpointMarker );
				std::stringstream ss;
				ss << CommandBufferCheckpointTypeToString( data->type )
					<< " : " << data->name
					<< VkPipelineStageFlagBitsToString( c.stage )
					<< "\n";
				ConsolePrint( ss.str() );
			}
			ConsolePrint( "\nCommand buffer checkmark trail:\n" );
			auto c = reinterpret_cast<CommandBufferCheckpointData*>( checkpoints[ 0 ].pCheckpointMarker )->previous;
			while( c ) {
				auto current = c;
				auto previous= current->previous;
				c = previous;

				std::stringstream ss;
				ss << CommandBufferCheckpointTypeToString( current->type )
					<< " : " << current->name << "\n";
				ConsolePrint( ss.str() );

				delete current;
			}
			delete reinterpret_cast<CommandBufferCheckpointData*>( checkpoints[ 0 ].pCheckpointMarker );
		}
	}
	#endif

	if( severity == ReportSeverity::NON_CRITICAL_ERROR ||
		severity == ReportSeverity::CRITICAL_ERROR ||
		severity == ReportSeverity::DEVICE_LOST )
	{
		#ifdef _WIN32
		MessageBox( NULL, editable_message.c_str(), "Critical error!", MB_OK | MB_ICONERROR );
		#endif
		if( severity == ReportSeverity::CRITICAL_ERROR || severity == ReportSeverity::DEVICE_LOST ) {
			std::exit( -1 );
		}
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::UpdateMonitorLists(
	bool		globals_locked
)
{
	VK2D_ASSERT_SINGLE_THREAD_ACCESS_SCOPE();

	// Lock globals if not already locked.
	std::unique_lock<std::mutex> unique_lock;
	if( !globals_locked ) {
		unique_lock = std::unique_lock<std::mutex>( instance_globals_mutex );
	}

	auto BuildMonitorData = [](
		GLFWmonitor		*	monitor
		) -> std::unique_ptr<MonitorImpl>
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
		MonitorVideoMode		current_video_mode		= {};
		{
			auto glfw_current_video_mode		= glfwGetVideoMode( monitor );
			current_video_mode.resolution		= { uint32_t( glfw_current_video_mode->width ), uint32_t( glfw_current_video_mode->height ) };
			current_video_mode.red_bit_count	= uint32_t( glfw_current_video_mode->redBits );
			current_video_mode.green_bit_count	= uint32_t( glfw_current_video_mode->greenBits );
			current_video_mode.blue_bit_count	= uint32_t( glfw_current_video_mode->blueBits );
			current_video_mode.refresh_rate		= uint32_t( glfw_current_video_mode->refreshRate );
		}

		std::vector<MonitorVideoMode> video_modes		= {};
		{
			int vidModeCount = 0;
			auto glfw_video_modes = glfwGetVideoModes( monitor, &vidModeCount );
			video_modes.resize( vidModeCount );
			for( int i = 0; i < vidModeCount; ++i ) {
				video_modes[ i ].resolution			= { uint32_t( glfw_video_modes[ i ].width ), uint32_t( glfw_video_modes[ i ].height ) };
				video_modes[ i ].red_bit_count		= glfw_video_modes[ i ].redBits;
				video_modes[ i ].green_bit_count	= glfw_video_modes[ i ].greenBits;
				video_modes[ i ].blue_bit_count		= glfw_video_modes[ i ].blueBits;
				video_modes[ i ].refresh_rate		= glfw_video_modes[ i ].refreshRate;
			};
		}

		return std::make_unique<MonitorImpl>(
			monitor,
			position,
			physical_size,
			name,
			current_video_mode,
			video_modes
			);
	};

	// All monitors
	all_monitors.clear();
	int monitorCount = 0;
	auto monitors = glfwGetMonitors( &monitorCount );
	all_monitors.reserve( monitorCount );
	for( int i = 0; i < monitorCount; ++i ) {

		GLFWmonitor * monitor		= monitors[ i ];
		all_monitors.push_back( std::move( Monitor( BuildMonitorData( monitor ) ) ) );
	}

	// primary monitor
	primary_monitor = std::move( Monitor( BuildMonitorData( glfwGetPrimaryMonitor() ) ) );

	// callback
	for( auto l : instance_listeners ) {
		if( l->monitor_update_callback ) {
			l->monitor_update_callback();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwMonitorCallback(
	GLFWmonitor		*	monitor,
	int					event
)
{
	UpdateMonitorLists();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwJoystickEventCallback( int joystick, int event )
{
	std::lock_guard<std::mutex> lock_guard( instance_globals_mutex );

	if( event == GLFW_CONNECTED ) {
		std::string joystic_name = glfwGetJoystickName( joystick );

		for( auto l : instance_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( Gamepad( joystick ), GamepadConnectionEvent::CONNECTED, joystic_name );
			}
		}
	} else {
		for( auto l : instance_listeners ) {
			if( l->GetGamepadEventCallback() ) {
				l->GetGamepadEventCallback()( Gamepad( joystick ), GamepadConnectionEvent::DISCONNECTED, std::string( "" ) );
			}
		}
	}
}

} // vk2d_internal

} // vk2d
