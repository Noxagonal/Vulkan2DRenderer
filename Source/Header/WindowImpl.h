#pragma once

#include "SourceCommon.h"

#include "../../Include/VK2D/Renderer.h"
#include "../../Include/VK2D/Window.h"

#include "MeshBuffer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace vk2d {

class Window;
class MeshBuffer;



namespace _internal {

class RendererImpl;
class WindowImpl;

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

class WindowImpl {
	friend class Window;
	friend bool								AquireImage(
		WindowImpl						*	data,
		VkPhysicalDevice					physical_device,
		VkDevice							device,
		ResolvedQueue					&	primary_render_queue,
		uint32_t							nested_counter );

public:
	WindowImpl(
		RendererImpl					*	renderer,
		const WindowCreateInfo			&	window_create_info
	);
	~WindowImpl();

	bool									BeginRender();
	bool									EndRender();

	void									Draw_TriangleList(
		bool								filled,
		std::vector<Vertex>				&	vertices,
		std::vector<VertexIndex_3>		&	indices );

	void									Draw_LineList(
		std::vector<Vertex>				&	vertices,
		std::vector<VertexIndex_2>		&	indices );

	void									Draw_PointList(
		std::vector<Vertex>					vertices );

	void									Draw_Line(
		Vertex								point_1,
		Vertex								point_2 );

	void									Draw_Box(
		bool								filled,
		Coords								top_left,
		Coords								bottom_right );

	void									Draw_Circle(
		bool								filled,
		float								edge_count,
		Coords								location,
		float								radius );

	void									Draw_Pie(
		bool								filled,
		float								edge_count,
		Coords								location,
		float								radius,
		float								begin_angle_radians,
		float								end_angle_radians );

	bool									SynchronizeFrame();

private:
	bool									RecreateResourcesAfterResizing();
	bool									CreateGLFWWindow();
	bool									CreateSurface();
	bool									CreateRenderPass();
	bool									CreateGraphicsPipelines();
	bool									CreateCommandPool();
	bool									AllocateCommandBuffers();

	// If old swapchain is still active, this function instead re-creates
	// the swapchain recycling old resources whenever possible.
	bool									CreateSwapchain();
	bool									CreateFramebuffers();
	bool									CreateWindowSynchronizationPrimitives();
	bool									CreateFrameSynchronizationPrimitives();



	_internal::RendererImpl				*	renderer_parent							= {};
	WindowCreateInfo						create_info_copy						= {};

	VkInstance								instance								= {};
	VkPhysicalDevice						physical_device							= {};
	VkDevice								device									= {};

	ResolvedQueue							primary_render_queue					= {};

	PFN_VK2D_ReportFunction					report_function							= {};

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

	std::unique_ptr<MeshBuffer>				mesh_buffer								= {};

	bool									is_good									= {};
};



} // _internal

} // vk2d
