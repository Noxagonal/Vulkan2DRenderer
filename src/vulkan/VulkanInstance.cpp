
#include "VulkanInstance.hpp"

#include <interface/InstanceImpl.hpp>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE

namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkBool32 VKAPI_PTR DebugMessenger(
	VkDebugUtilsMessageSeverityFlagBitsEXT			messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT					messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT*		pCallbackData,
	void*											pUserData )
{
	std::string str_severity;
	ReportSeverity vk2d_severity;
	switch( messageSeverity ) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		vk2d_severity = ReportSeverity::VERBOSE;
		str_severity = "VERBOSE";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		vk2d_severity = ReportSeverity::INFO;
		str_severity = "INFO";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		vk2d_severity = ReportSeverity::WARNING;
		str_severity = "WARNING";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		vk2d_severity = ReportSeverity::NON_CRITICAL_ERROR;
		str_severity = "ERROR";
		break;
	default:
		vk2d_severity = ReportSeverity::INFO;
		str_severity = "<UNKNOWN>";
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

	auto instance = reinterpret_cast<InstanceImpl*>( pUserData );
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



} // vk2d_internal
} // vk2d

#endif



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanInstance::VulkanInstance(
	InstanceImpl & instance
) :
	instance( instance )
{
	// glfw vulkan instance extensions.
	{
		uint32_t glfw_instance_extension_count = 0;
		auto glfw_instance_extensions_ptr = glfwGetRequiredInstanceExtensions( &glfw_instance_extension_count );
		for( uint32_t i = 0; i < glfw_instance_extension_count; ++i ) {
			instance_extensions.push_back( glfw_instance_extensions_ptr[ i ] );
		}
	}

	#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE

	instance_layers.push_back( "VK_LAYER_KHRONOS_validation" );
	// instance_layers.push_back( "VK_LAYER_LUNARG_device_simulation" );
	instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	#if VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP
	instance_layers.push_back( "VK_LAYER_LUNARG_api_dump" );

	std::stringstream dump_file;
	std::stringstream dump_extension;

	dump_file << "VK_APIDUMP_LOG_FILENAME="
		<< VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP_FILE_PATH
		<< "."
		<< VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP_FILE_EXTENSION;

	dump_extension << "VK_APIDUMP_OUTPUT_FORMAT="
		<< VK2D_BUILD_OPTION_VULKAN_VALIDATION_API_DUMP_FILE_EXTENSION;

	putenv( dump_file.str().c_str() );
	putenv( dump_extension.str().c_str() );
	#endif

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
	debug_utils_create_info.pUserData			= &instance;

	VkValidationFeaturesEXT validation_features {};
	validation_features.sType							= VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validation_features.pNext							= &debug_utils_create_info;
	validation_features.enabledValidationFeatureCount	= uint32_t( enabled_validation_features.size() );
	validation_features.pEnabledValidationFeatures		= enabled_validation_features.data();
	validation_features.disabledValidationFeatureCount	= uint32_t( disabled_validation_features.size() );
	validation_features.pDisabledValidationFeatures		= disabled_validation_features.data();

	#endif

	{
		assert( VK_HEADER_VERSION_COMPLETE >= VK2D_BUILD_OPTION_VULKAN_MINIMUM_REQUIRED_VERSION );

		VkApplicationInfo application_info {};
		application_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pNext				= nullptr;
		application_info.pApplicationName	= instance.GetCreateInfo().application_name.c_str();
		application_info.applicationVersion	= instance.GetCreateInfo().application_version.ToVulkanVersion();
		application_info.pEngineName		= instance.GetCreateInfo().engine_name.c_str();
		application_info.engineVersion		= instance.GetCreateInfo().engine_version.ToVulkanVersion();
		application_info.apiVersion			= VK2D_BUILD_OPTION_VULKAN_MINIMUM_REQUIRED_VERSION;

		VkInstanceCreateInfo instance_create_info {};
		instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
		instance_create_info.pNext						= &validation_features;
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
			instance.Report( result, "Internal error: Cannot create vulkan instance!" );
			return;
		}
	}

	#if VK2D_BUILD_OPTION_VULKAN_VALIDATION && VK2D_DEBUG_ENABLE
	{
		auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( vk_instance, "vkCreateDebugUtilsMessengerEXT" );
		if( !createDebugUtilsMessenger ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create vulkan debug object!" );
			return;
		}
		auto result = createDebugUtilsMessenger(
			vk_instance,
			&debug_utils_create_info,
			nullptr,
			&vk_debug_utils_messenger
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create vulkan debug object!" );
			return;
		}
	}
	#endif

	is_good = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanInstance::~VulkanInstance()
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

		vk_debug_utils_messenger = {};
	}

	vkDestroyInstance(
		vk_instance,
		nullptr
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkInstance vk2d::vk2d_internal::VulkanInstance::GetVulkanInstance()
{
	return vk_instance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<VkPhysicalDevice> vk2d::vk2d_internal::VulkanInstance::EnumeratePhysicalDevices()
{
	auto result = VK_SUCCESS;

	uint32_t physical_device_count = UINT32_MAX;
	result = vkEnumeratePhysicalDevices(
		vk_instance,
		&physical_device_count,
		nullptr
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot enumerate physical devices!" );
		return {};
	}

	std::vector<VkPhysicalDevice> physical_devices( physical_device_count );
	result = vkEnumeratePhysicalDevices(
		vk_instance,
		&physical_device_count,
		physical_devices.data()
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot enumerate physical devices!" );
		return {};
	}

	return physical_devices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPhysicalDevice vk2d::vk2d_internal::VulkanInstance::PickBestVulkanPhysicalDevice()
{
	auto physicalDevices = EnumeratePhysicalDevices();

	std::vector<uint64_t> scores( physicalDevices.size() );
	for( size_t i = 0; i < physicalDevices.size(); ++i ) {
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
		bool		physicalDeviceCanPresent = false;
		uint32_t	queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( pd, &queueFamilyCount, nullptr );
		for( uint32_t i = 0; i < queueFamilyCount; ++i ) {
			if( glfwGetPhysicalDevicePresentationSupport( vk_instance, pd, i ) ) {
				physicalDeviceCanPresent = true;
				break;
			}
		}
		// If the physical device cannot present anything we won't even consider it
		if( !physicalDeviceCanPresent ) {
			s = 0;
		}
	}

	VkPhysicalDevice best_physical_device = VK_NULL_HANDLE;
	uint64_t best_score_so_far = 0;
	for( size_t i = 0; i < physicalDevices.size(); ++i ) {
		if( scores[ i ] > best_score_so_far ) {
			best_score_so_far = scores[ i ];
			best_physical_device = physicalDevices[ i ];
		}
	}

	if( !best_physical_device ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Cannot find physical device capable of presenting to any surface!" );
	}

	return best_physical_device;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::VulkanInstance::IsGood()
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::VulkanInstance::operator VkInstance()
{
	return GetVulkanInstance();
}
