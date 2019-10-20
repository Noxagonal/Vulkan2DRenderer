#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/Renderer.h"

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

	std::list<std::unique_ptr<Window>>				windows						= {};
};


} // _internal

} // vk2d
