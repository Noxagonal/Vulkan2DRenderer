
#include "../Header/SourceCommon.h"
#include "../Header/RendererImpl.h"

#include "../../Include/VK2D/Window.h"

#include "../Header/QueueResolver.h"

#include "../Shaders/shader.vert.spv.h"
#include "../Shaders/shader.frag.spv.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include <sstream>
#include <iostream>


namespace vk2d {



namespace _internal {









uint64_t RendererImpl::renderer_count				= 0;

RendererImpl::RendererImpl( const RendererCreateInfo & renderer_create_info )
{
	// Initialize glfw if this is the first renderer.
	if( renderer_count == 0 ) {
		glfwInit();
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	}
	++renderer_count;

	// Check if vulkan is supported
	if( glfwVulkanSupported() == GLFW_FALSE ) {
		if( renderer_create_info.report_function ) {
			renderer_create_info.report_function( ReportSeverity::NON_CRITICAL_ERROR, "Vulkan is not supported on this machine!\nUpdating your graphics drivers might fix this issue!" );
		}
		return;
	}

	create_info_copy	= renderer_create_info;
	report_function		= create_info_copy.report_function;

	// Introduce layers and extensions here
//	instance_layers;
//	instance_extensions;
	device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
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
	if( !CreateSampler() ) return;
	if( !CreatePipelineCache() ) return;
	if( !CreateShaderModules() ) return;
	if( !CreateDescriptorSetLayouts() ) return;
	if( !CreatePipelineLayout() ) return;

	device_memory_pool		= MakeDeviceMemoryPool(
		physical_device,
		device
	);

	if( !device_memory_pool ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create Vulkan Device memory pool!" );
		}
		return;
	}

	is_good		= true;
}

RendererImpl::~RendererImpl()
{
	vkDeviceWaitIdle( device );

	windows.clear();

	device_memory_pool	= nullptr;

	vkDestroyPipelineLayout(
		device,
		pipeline_layout,
		nullptr
	);

	vkDestroyDescriptorSetLayout(
		device,
		descriptor_set_layout,
		nullptr
	);

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

	vkDestroyPipelineCache(
		device,
		pipeline_cache,
		nullptr
	);

	vkDestroySampler(
		device,
		sampler,
		nullptr
	);

	vkDestroyDevice( device, nullptr );

	if( debug_utils_messenger ) {
		auto destroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if( destroyDebugUtilsMessenger ) {
			destroyDebugUtilsMessenger( instance, debug_utils_messenger, nullptr );
		}
	}

	vkDestroyInstance( instance, nullptr );

	// If this is the last renderer exiting, also terminate glfw
	--renderer_count;
	if( renderer_count == 0 ) {
		glfwTerminate();
	}
}









Window * RendererImpl::CreateWindowOutput(
	WindowCreateInfo	&	window_create_info
)
{
	auto new_window = std::unique_ptr<Window>( new Window( this, window_create_info ) );

	if( new_window->is_good ) {
		windows.push_back( std::move( new_window ) );
		return windows.back().get();
	}
	return {};
}









void RendererImpl::CloseWindowOutput(
	Window				*	window
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









PFN_VK2D_ReportFunction RendererImpl::GetReportFunction()
{
	return report_function;
}

VkInstance RendererImpl::GetVulkanInstance()
{
	return instance;
}

VkPhysicalDevice RendererImpl::GetVulkanPhysicalDevice()
{
	return physical_device;
}

VkDevice RendererImpl::GetVulkanDevice()
{
	return device;
}

ResolvedQueue RendererImpl::GetPrimaryRenderQueue()
{
	return primary_render_queue;
}

ResolvedQueue RendererImpl::GetSecondaryRenderQueue()
{
	return secondary_render_queue;
}

ResolvedQueue RendererImpl::GetPrimaryComputeQueue()
{
	return primary_compute_queue;
}

ResolvedQueue RendererImpl::GetPrimaryTransferQueue()
{
	return primary_transfer_queue;
}

const VkPhysicalDeviceProperties & RendererImpl::GetPhysicalDeviceProperties()
{
	return physical_device_properties;
}

const VkPhysicalDeviceMemoryProperties & RendererImpl::GetPhysicalDeviceMemoryProperties()
{
	return physical_device_memory_properties;
}

const VkPhysicalDeviceFeatures & RendererImpl::GetPhysicalDeviceFeatures()
{
	return physical_device_features;
}

VkShaderModule RendererImpl::GetVertexShaderModule()
{
	return vertex_shader_module;
}

VkShaderModule RendererImpl::GetFragmentShaderModule()
{
	return fragment_shader_module;
}

VkPipelineCache RendererImpl::GetPipelineCache()
{
	return pipeline_cache;
}

VkPipelineLayout RendererImpl::GetPipelineLayout()
{
	return pipeline_layout;
}

VkDescriptorSetLayout RendererImpl::GetDescriptorSetLayout()
{
	return descriptor_set_layout;
}

DeviceMemoryPool * RendererImpl::GetDeviceMemoryPool()
{
	return device_memory_pool.get();
}









#if VK2D_BUILD_OPTION_VULKAN_VALIDATION

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

#endif









bool RendererImpl::CreateInstance()
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
	debug_utils_create_info.pfnUserCallback		= DebugMessenger;
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
			if( create_info_copy.report_function ) {
				create_info_copy.report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan instance!" );
			}
			return false;
		}
	}

#if VK2D_BUILD_OPTION_VULKAN_VALIDATION
	{
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		if( !createDebugUtilsMessenger ) {
			if( create_info_copy.report_function ) {
				create_info_copy.report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan debug object!" );
			}
			return false;
		}
		auto result = createDebugUtilsMessenger(
			instance,
			&debug_utils_create_info,
			nullptr,
			&debug_utils_messenger
		);
		if( result != VK_SUCCESS ) {
			if( create_info_copy.report_function ) {
				create_info_copy.report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan debug object!" );
			}
			return false;
		}
	}
#endif

	return true;
}









bool RendererImpl::PickPhysicalDevice()
{
	physical_device	= PickBestPhysicalDevice();

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









bool RendererImpl::CreateDeviceAndQueues()
{
	std::vector<std::pair<VkQueueFlags, float>> queue_requests {
		{ VK_QUEUE_GRAPHICS_BIT, 1.0f },
		{ VK_QUEUE_GRAPHICS_BIT, 0.2f },
		{ VK_QUEUE_COMPUTE_BIT, 1.0f },
		{ VK_QUEUE_TRANSFER_BIT, 1.0f }
	};
	DeviceQueueResolver queue_resolver( instance, physical_device, queue_requests );
	if( !queue_resolver.IsGood() ) {
		if( create_info_copy.report_function ) {
			create_info_copy.report_function( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return false;
	}
	auto queue_create_infos = queue_resolver.GetDeviceQueueCreateInfos();

	VkPhysicalDeviceFeatures features {};
	features.samplerAnisotropy							= VK_TRUE;
	features.fillModeNonSolid							= VK_TRUE;

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
		if( create_info_copy.report_function ) {
			create_info_copy.report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan device!" );
		}
		return false;
	}

	auto resolved_queues = queue_resolver.GetQueues( device );
	if( resolved_queues.size() != queue_requests.size() ) {
		if( create_info_copy.report_function ) {
			create_info_copy.report_function( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return false;
	}
	primary_render_queue		= resolved_queues[ 0 ];
	secondary_render_queue	= resolved_queues[ 1 ];
	primary_compute_queue		= resolved_queues[ 2 ];
	primary_transfer_queue			= resolved_queues[ 3 ];

	return true;
}









bool RendererImpl::CreateSampler()
{
	VkSamplerCreateInfo sampler_create_info {};
	sampler_create_info.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.pNext						= nullptr;
	sampler_create_info.flags						= 0;
	sampler_create_info.magFilter					= VK_FILTER_LINEAR;
	sampler_create_info.minFilter					= VK_FILTER_LINEAR;
	sampler_create_info.mipmapMode					= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.mipLodBias					= 0.0f;
	sampler_create_info.anisotropyEnable			= VK_TRUE;
	sampler_create_info.maxAnisotropy				= physical_device_properties.limits.maxSamplerAnisotropy;
	sampler_create_info.compareEnable				= VK_FALSE;
	sampler_create_info.compareOp					= VK_COMPARE_OP_NEVER;
	sampler_create_info.minLod						= 0.0f;
	sampler_create_info.maxLod						= 32.0f;
	sampler_create_info.borderColor					= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	sampler_create_info.unnormalizedCoordinates		= VK_FALSE;

	if( vkCreateSampler(
		device,
		&sampler_create_info,
		nullptr,
		&sampler
	) != VK_SUCCESS ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create sampler!" );
		}
		return false;
	}

	return true;
}









bool RendererImpl::CreatePipelineCache()
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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create pipeline cache!" );
		}
		return false;
	}

	return true;
}









bool RendererImpl::CreateShaderModules()
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vertex shader module!" );
			}
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create fragment shader module!" );
			}
			return false;
		}
	}

	return true;
}









bool RendererImpl::CreateDescriptorSetLayouts()
{
	// These must match shader layout.

	// Set 0 layout
	{
		std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings {};
		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		// If using more than one sampler this will need to be set to nullptr and update manually
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= &sampler;

		descriptor_set_layout_bindings[ 1 ].binding				= 1;
		descriptor_set_layout_bindings[ 1 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptor_set_layout_bindings[ 1 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 1 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 1 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		// TODO: check result
		if( vkCreateDescriptorSetLayout(
			device,
			&descriptor_set_layout_create_info,
			nullptr,
			&descriptor_set_layout
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create descriptor set layout!" );
			}
			return false;
		}
	}

	return true;
}









bool RendererImpl::CreatePipelineLayout()
{
	// This must match shader layout.

	std::vector<VkDescriptorSetLayout> set_layouts {
		descriptor_set_layout
	};

	std::array<VkPushConstantRange, 0> push_constant_ranges {};

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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create pipeline layout!" );
		}
		return false;
	}

	return true;
}









std::vector<VkPhysicalDevice> RendererImpl::EnumeratePhysicalDevices()
{
	uint32_t physical_device_count		= UINT32_MAX;
	vkEnumeratePhysicalDevices( instance, &physical_device_count, nullptr );
	std::vector<VkPhysicalDevice> physical_devices( physical_device_count );
	vkEnumeratePhysicalDevices( instance, &physical_device_count, physical_devices.data() );
	return physical_devices;
}









VkPhysicalDevice RendererImpl::PickBestPhysicalDevice()
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

	return best_physical_device;
}









} // _internal

} // vk2d