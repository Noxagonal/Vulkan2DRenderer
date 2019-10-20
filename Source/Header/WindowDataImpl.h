#pragma once

#include "SourceCommon.h"
#include "../../Include/VK2D/Window.h"
#include "../../Include/VK2D/Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>

namespace vk2d {

class Renderer;



namespace _internal {



enum class NextRenderCallFunction : uint32_t {
	BEGIN		= 0,
	END			= 1,
};

enum class PipelineType : uint32_t {
	FILLED_POLYGON_LIST,
	WIREFRAME_POLYGON_LIST,
	LINE_LIST,
	POINT_LIST,

	PIPELINE_TYPE_COUNT,
};

struct WindowDataImpl {
	PFN_VK2D_ReportFunction					report_function							= {};

	Renderer							*	renderer								= {};
	WindowCreateInfo						create_info_copy						= {};

	GLFWwindow							*	glfw_window								= {};

	VkSurfaceKHR							surface									= {};
	VkSwapchainKHR							swapchain								= {};
	std::vector<VkImage>					swapchain_images						= {};
	std::vector<VkImageView>				swapchain_image_views					= {};
	VkRenderPass							render_pass								= {};

	VkCommandPool							command_pool							= {};
	std::vector<VkCommandBuffer>			command_buffers							= {};

	VkExtent2D								min_extent								= {};
	VkExtent2D								max_extent								= {};
	VkExtent2D								extent									= {};
	uint32_t								swapchain_image_count					= {};
	VkSurfaceFormatKHR						surface_format							= {};
	VkPresentModeKHR						present_mode							= {};
	VkSurfaceCapabilitiesKHR				surface_capabilities					= {};
	std::vector<VkFramebuffer>				framebuffers							= {};

	uint32_t								next_image								= {};
	uint32_t								previous_image							= {};
	VkFence									aquire_image_fence						= {};
	std::vector<VkSemaphore>				submit_to_present_semaphores			= {};
	std::vector<VkFence>					gpu_to_cpu_frame_fences					= {};
	bool									previous_frame_need_synchronization		= {};

	std::vector<VkPipeline>					pipelines								= {};

	NextRenderCallFunction					next_render_call_function				= NextRenderCallFunction::BEGIN;
	bool									recreate_swapchain						= {};

	std::vector<std::vector<Vertex>>		frame_vertices							= {};
	std::vector<std::vector<uint32_t>>		frame_indices							= {};
};



} // _internal

} // vk2d
