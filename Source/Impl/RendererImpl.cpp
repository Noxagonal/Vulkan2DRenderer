
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/RendererImpl.h"
#include "../../Include/Interface/ResourceManager.h"
#include "../Header/Impl/ResourceManagerImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Core/QueueResolver.h"
#include "../Header/Core/ThreadPool.h"
#include "../Header/Core/ThreadPrivateResources.h"
#include "../Header/Core/DescriptorSet.h"

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
	if( !CreateDescriptorSetLayouts() ) return;
	if( !CreateSamplers() ) return;
	if( !CreatePipelineCache() ) return;
	if( !CreateShaderModules() ) return;
	if( !CreatePipelineLayout() ) return;
	if( !CreateDeviceMemoryPool() ) return;
	if( !CreateThreadPool() ) return;
	if( !CreateResourceManager() ) return;
	if( !CreateDefaultTexture() ) return;

	is_good		= true;
}

RendererImpl::~RendererImpl()
{
	vkDeviceWaitIdle( device );

	windows.clear();

	DestroyDefaultTexture();
	DestroyResourceManager();
	DestroyThreadPool();
	DestroyDeviceMemoryPool();
	DestroyPipelineLayout();
	DestroyShaderModules();
	DestroyPipelineCache();
	DestroySamplers();
	DestroyDescriptorSetLayouts();
	DestroyDevice();
	DestroyInstance();

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









PFN_VK2D_ReportFunction RendererImpl::GetReportFunction() const
{
	return report_function;
}

ThreadPool * RendererImpl::GetThreadPool() const
{
	return thread_pool.get();
}

const std::vector<uint32_t> & RendererImpl::GetLoaderThreads() const
{
	return loader_threads;
}

const std::vector<uint32_t> & RendererImpl::GetGeneralThreads() const
{
	return general_threads;
}

ResourceManager * RendererImpl::GetResourceManager() const
{
	return resource_manager.get();
}

VkInstance RendererImpl::GetVulkanInstance() const
{
	return instance;
}

VkPhysicalDevice RendererImpl::GetVulkanPhysicalDevice() const
{
	return physical_device;
}

VkDevice RendererImpl::GetVulkanDevice() const
{
	return device;
}

ResolvedQueue RendererImpl::GetPrimaryRenderQueue() const
{
	return primary_render_queue;
}

ResolvedQueue RendererImpl::GetSecondaryRenderQueue() const
{
	return secondary_render_queue;
}

ResolvedQueue RendererImpl::GetPrimaryComputeQueue() const
{
	return primary_compute_queue;
}

ResolvedQueue RendererImpl::GetPrimaryTransferQueue() const
{
	return primary_transfer_queue;
}

const VkPhysicalDeviceProperties & RendererImpl::GetPhysicalDeviceProperties() const
{
	return physical_device_properties;
}

const VkPhysicalDeviceMemoryProperties & RendererImpl::GetPhysicalDeviceMemoryProperties() const
{
	return physical_device_memory_properties;
}

const VkPhysicalDeviceFeatures & RendererImpl::GetPhysicalDeviceFeatures() const
{
	return physical_device_features;
}

VkShaderModule RendererImpl::GetVertexShaderModule() const
{
	return vertex_shader_module;
}

VkShaderModule RendererImpl::GetFragmentShaderModule() const
{
	return fragment_shader_module;
}

VkPipelineCache RendererImpl::GetPipelineCache() const
{
	return pipeline_cache;
}

VkPipelineLayout RendererImpl::GetPipelineLayout() const
{
	return pipeline_layout;
}

const DescriptorSetLayout & RendererImpl::GetSamplerDescriptorSetLayout() const
{
	return *sampler_descriptor_set_layout;
}

const DescriptorSetLayout & RendererImpl::GetTextureDescriptorSetLayout() const
{
	return *texture_descriptor_set_layout;
}

VkDescriptorSet RendererImpl::GetDefaultTextureDescriptorSet() const
{
	return default_texture_descriptor_set;
}

VkDescriptorSet RendererImpl::GetSamplerDescriptorSet( SamplerType sampler_type ) const
{
	return samplers[ uint32_t( sampler_type ) ].descriptor_set;
}

VkSampler RendererImpl::GetSampler( SamplerType sampler_type ) const
{
	return samplers[ uint32_t( sampler_type ) ].sampler;
}

DeviceMemoryPool * RendererImpl::GetDeviceMemoryPool() const
{
	return device_memory_pool.get();
}

bool RendererImpl::IsGood() const
{
	return is_good;
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
	secondary_render_queue		= resolved_queues[ 1 ];
	primary_compute_queue		= resolved_queues[ 2 ];
	primary_transfer_queue		= resolved_queues[ 3 ];

	return true;
}









bool RendererImpl::CreateSamplers()
{
	// Sampler 0
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
			&samplers[ 0 ].sampler
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create sampler!" );
			}
			return false;
		}
	}

	// Create descriptor pool and allocate descriptor sets.
	std::vector<VkDescriptorSet> descriptor_sets( samplers.size() );
	{
		VkDescriptorPoolSize pool_size {};
		pool_size.type								= VK_DESCRIPTOR_TYPE_SAMPLER;
		pool_size.descriptorCount					= uint32_t( samplers.size() );
		VkDescriptorPoolCreateInfo descriptor_pool_create_info {};
		descriptor_pool_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.pNext			= nullptr;
		descriptor_pool_create_info.flags			= 0;
		descriptor_pool_create_info.maxSets			= uint32_t( samplers.size() );
		descriptor_pool_create_info.poolSizeCount	= 1;
		descriptor_pool_create_info.pPoolSizes		= &pool_size;
		if( vkCreateDescriptorPool(
			device,
			&descriptor_pool_create_info,
			nullptr,
			&sampler_descriptor_pool
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create samplers!" );
			}
			return false;
		}

		VkDescriptorSetLayout set_layout					= sampler_descriptor_set_layout->GetVulkanDescriptorSetLayout();
		VkDescriptorSetAllocateInfo descriptor_set_allocate_info {};
		descriptor_set_allocate_info.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_info.pNext					= nullptr;
		descriptor_set_allocate_info.descriptorPool			= sampler_descriptor_pool;
		descriptor_set_allocate_info.descriptorSetCount		= uint32_t( samplers.size() );
		descriptor_set_allocate_info.pSetLayouts			= &set_layout;
		if( vkAllocateDescriptorSets(
			device,
			&descriptor_set_allocate_info,
			descriptor_sets.data()
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create sampler!" );
			}
			return false;
		}
	}

	// Update descriptor sets.
	for( size_t i = 0; i < samplers.size(); ++i ) {
		samplers[ i ].descriptor_set			= descriptor_sets[ i ];

		VkDescriptorImageInfo descriptor_image_info {};
		descriptor_image_info.sampler			= samplers[ i ].sampler;
		descriptor_image_info.imageView			= VK_NULL_HANDLE;
		descriptor_image_info.imageLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
		VkWriteDescriptorSet descriptor_set_write {};
		descriptor_set_write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_set_write.pNext				= nullptr;
		descriptor_set_write.dstSet				= samplers[ i ].descriptor_set;
		descriptor_set_write.dstBinding			= 0;
		descriptor_set_write.dstArrayElement	= 0;
		descriptor_set_write.descriptorCount	= 1;
		descriptor_set_write.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_write.pImageInfo			= &descriptor_image_info;
		descriptor_set_write.pBufferInfo		= nullptr;
		descriptor_set_write.pTexelBufferView	= nullptr;
		vkUpdateDescriptorSets(
			device,
			1, &descriptor_set_write,
			0, nullptr
		);
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

	// Set 0 layout, sampler
	{
		std::array<VkDescriptorSetLayoutBinding, 1> descriptor_set_layout_bindings {};

		descriptor_set_layout_bindings[ 0 ].binding				= 0;
		descriptor_set_layout_bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_layout_bindings[ 0 ].descriptorCount		= 1;
		descriptor_set_layout_bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[ 0 ].pImmutableSamplers	= nullptr;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.pNext			= nullptr;
		descriptor_set_layout_create_info.flags			= 0;
		descriptor_set_layout_create_info.bindingCount	= uint32_t( descriptor_set_layout_bindings.size() );
		descriptor_set_layout_create_info.pBindings		= descriptor_set_layout_bindings.data();

		sampler_descriptor_set_layout		= _internal::CreateDescriptorSetLayout(
			device,
			&descriptor_set_layout_create_info
		);
		if( !sampler_descriptor_set_layout ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create descriptor set layout!" );
			}
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

		texture_descriptor_set_layout		= _internal::CreateDescriptorSetLayout(
			device,
			&descriptor_set_layout_create_info
		);
		if( !texture_descriptor_set_layout ) {
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
		sampler_descriptor_set_layout->GetVulkanDescriptorSetLayout(),
		texture_descriptor_set_layout->GetVulkanDescriptorSetLayout()
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

bool RendererImpl::CreateDeviceMemoryPool()
{
	device_memory_pool		= MakeDeviceMemoryPool(
		physical_device,
		device
	);
	if( !device_memory_pool ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create Vulkan Device memory pool!" );
		}
		return false;
	}
	return true;
}

bool RendererImpl::CreateThreadPool()
{
	uint32_t thread_count			= std::thread::hardware_concurrency();
	if( thread_count == 0 ) thread_count = 8;
	--thread_count;

	uint32_t loader_thread_count	= thread_count / 5;
	uint32_t general_thread_count	= thread_count - loader_thread_count;
	if( loader_thread_count == 0 )	loader_thread_count = 1;
	if( general_thread_count == 0 )	general_thread_count = 1;

	std::vector<std::unique_ptr<ThreadPrivateResource>> thread_resources;
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<ThreadLoaderResource>( this ) );
	}
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		thread_resources.push_back( std::make_unique<ThreadGeneralResource>() );
	}

	loader_threads.resize( loader_thread_count );
	general_threads.resize( general_thread_count );
	for( uint32_t i = 0; i < loader_thread_count; ++i ) {
		loader_threads[ i ]		= i;
	}
	for( uint32_t i = 0; i < general_thread_count; ++i ) {
		general_threads[ i ]	= loader_thread_count + i;
	}

	thread_pool				= std::make_unique<ThreadPool>( std::move( thread_resources ) );
	if( thread_pool && thread_pool->IsGood() ) {
		return true;
	} else {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create thread pool!" );
		}
		return false;
	}
}

bool RendererImpl::CreateResourceManager()
{
	resource_manager		= std::make_unique<ResourceManager>(
		this
		);
	if( resource_manager && resource_manager->IsGood() ) {
		return true;
	} else {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create resource manager!" );
		}
		return false;
	}
}

bool RendererImpl::CreateDefaultTexture()
{
	// Create simple descriptor pool
	{
		std::array<VkDescriptorPoolSize, 1> pool_sizes {};
		pool_sizes[ 0 ].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		pool_sizes[ 0 ].descriptorCount		= 1;

		VkDescriptorPoolCreateInfo descriptor_pool_create_info {};
		descriptor_pool_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.pNext			= nullptr;
		descriptor_pool_create_info.flags			= 0;
		descriptor_pool_create_info.maxSets			= 1;
		descriptor_pool_create_info.poolSizeCount	= uint32_t( pool_sizes.size() );
		descriptor_pool_create_info.pPoolSizes		= pool_sizes.data();

		if( vkCreateDescriptorPool(
			device,
			&descriptor_pool_create_info,
			nullptr,
			&default_texture_descriptor_pool
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
			}
			return false;
		}
	}
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
				if( report_function ) {
					report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
				}
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
				pool_create_info.queueFamilyIndex	= primary_render_queue.queueFamilyIndex;
				if( vkCreateCommandPool(
					device,
					&pool_create_info,
					nullptr,
					&cpool
				) != VK_SUCCESS ) {
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
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
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
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
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
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
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
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
						if( report_function ) {
							report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
						}
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
				if( vkQueueSubmit(
					primary_render_queue.queue,
					1, &submit_info,
					fence
				) != VK_SUCCESS ) {
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
					return false;
				}

				if( vkWaitForFences(
					device,
					1, &fence,
					VK_TRUE,
					UINT64_MAX
				) != VK_SUCCESS ) {
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
					}
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
		auto set_layout = texture_descriptor_set_layout->GetVulkanDescriptorSetLayout();

		VkDescriptorSetAllocateInfo descriptor_set_allocate_info {};
		descriptor_set_allocate_info.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_info.pNext					= nullptr;
		descriptor_set_allocate_info.descriptorPool			= default_texture_descriptor_pool;
		descriptor_set_allocate_info.descriptorSetCount		= 1;
		descriptor_set_allocate_info.pSetLayouts			= &set_layout;

		if( vkAllocateDescriptorSets(
			device,
			&descriptor_set_allocate_info,
			&default_texture_descriptor_set
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create default texture!" );
			}
			return false;
		}

		VkDescriptorImageInfo descriptor_image_info {};
		descriptor_image_info.sampler			= VK_NULL_HANDLE;
		descriptor_image_info.imageView			= default_texture.view;
		descriptor_image_info.imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkWriteDescriptorSet, 1> descriptor_writes {};
		descriptor_writes[ 0 ].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[ 0 ].pNext			= nullptr;
		descriptor_writes[ 0 ].dstSet			= default_texture_descriptor_set;
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



void RendererImpl::DestroyInstance()
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

void RendererImpl::DestroyDevice()
{
	vkDestroyDevice(
		device,
		nullptr
	);

	device					= {};
}

void RendererImpl::DestroySamplers()
{
	for( auto & s : samplers ) {
		vkDestroySampler(
			device,
			s.sampler,
			nullptr
		);
	}

	vkDestroyDescriptorPool(
		device,
		sampler_descriptor_pool,
		nullptr
	);

	samplers				= {};
	sampler_descriptor_pool	= {};
}

void RendererImpl::DestroyPipelineCache()
{
	vkDestroyPipelineCache(
		device,
		pipeline_cache,
		nullptr
	);

	pipeline_cache			= {};
}

void RendererImpl::DestroyShaderModules()
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

void RendererImpl::DestroyDescriptorSetLayouts()
{
	sampler_descriptor_set_layout	= nullptr;
	texture_descriptor_set_layout	= nullptr;
}

void RendererImpl::DestroyPipelineLayout()
{
	vkDestroyPipelineLayout(
		device,
		pipeline_layout,
		nullptr
	);

	pipeline_layout			= {};
}

void RendererImpl::DestroyDeviceMemoryPool()
{
	device_memory_pool		= {};
}

void RendererImpl::DestroyThreadPool()
{
	thread_pool				= {};
}

void RendererImpl::DestroyResourceManager()
{
	resource_manager		= {};
}

void RendererImpl::DestroyDefaultTexture()
{
	vkDestroyDescriptorPool(
		device,
		default_texture_descriptor_pool,
		nullptr
	);

	device_memory_pool->FreeCompleteResource( default_texture );

	default_texture						= {};
	default_texture_descriptor_pool		= {};
	default_texture_descriptor_set		= {};
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
