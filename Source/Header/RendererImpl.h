#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/Renderer.h"
#include "../Header/VulkanMemoryManagement.h"

#include <list>
#include <vector>
#include <array>
#include <memory>



namespace vk2d {

class Window;

namespace _internal {



struct RendererDataImpl {
	RendererCreateInfo								create_info_copy			= {};

	PFN_VK2D_ReportFunction							report_function				= {};

	VkDebugUtilsMessengerEXT						debug_utils_messenger		= {};

	VkInstance										instance					= {};
	VkPhysicalDevice								physical_device				= {};
	VkDevice										device						= {};

	VkPhysicalDeviceProperties						physical_device_properties	= {};

	VkShaderModule									vertex_shader_module		= {};
	VkShaderModule									fragment_shader_module		= {};
	VkPipelineCache									pipeline_cache				= {};
	VkPipelineLayout								pipeline_layout				= {};

	VkSampler										sampler						= {};

	VkDescriptorSetLayout							descriptor_set_layout		= {};

	ResolvedQueue									primary_render_queue		= {};
	ResolvedQueue									secondary_render_queue		= {};
	ResolvedQueue									primary_compute_queue		= {};
	ResolvedQueue									primary_transfer			= {};

	std::unique_ptr<DeviceMemoryPool>				device_memory_pool			= {};

	std::list<std::unique_ptr<Window>>				windows						= {};
};



} // _internal


bool												CreateInstance(
	_internal::RendererDataImpl					*	data,
	std::vector<const char*>					&	instance_layers,
	std::vector<const char*>					&	instance_extensions );

bool												PickPhysicalDevice(
	_internal::RendererDataImpl					*	data );

bool												CreateDeviceAndQueues(
	_internal::RendererDataImpl					*	data,
	VkPhysicalDevice								physical_device,
	std::vector<const char*>					&	device_extensions );

bool												CreateSampler(
	_internal::RendererDataImpl					*	data,
	VkDevice										device );

bool												CreatePipelineCache(
	_internal::RendererDataImpl					*	data,
	VkDevice										device );

bool												CreateShaderModules(
	_internal::RendererDataImpl					*	data,
	VkDevice										device );

bool												CreateDescriptorSetLayouts(
	_internal::RendererDataImpl					*	data,
	VkDevice										device );

bool												CreatePipelineLayout(
	_internal::RendererDataImpl					*	data,
	VkDevice										device );

std::vector<VkPhysicalDevice>						EnumeratePhysicalDevices(
	VkInstance										instance );

VkPhysicalDevice									PickBestPhysicalDevice(
	VkInstance										instance,
	std::vector<VkPhysicalDevice>				&	physicalDevices );




} // vk2d
