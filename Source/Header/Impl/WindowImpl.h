#pragma once

#include "../Core/SourceCommon.h"

#include "../../../Include/Interface/Renderer.h"
#include "../../../Include/Interface/Window.h"

#include "../../Header/Core/MeshBuffer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace vk2d {

class Window;
class TextureResource;
class Mesh;

namespace _internal {

class RendererImpl;
class WindowImpl;
class MeshBuffer;
class TextureResourceImpl;

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

	NONE						= UINT32_MAX,
};

class WindowImpl {
	friend class Window;
	friend bool										AquireImage(
		vk2d::_internal::WindowImpl				*	data,
		VkPhysicalDevice							physical_device,
		VkDevice									device,
		vk2d::_internal::ResolvedQueue			&	primary_render_queue,
		uint32_t									nested_counter );

public:
	WindowImpl(
		RendererImpl							*	renderer,
		const WindowCreateInfo					&	window_create_info
	);
	~WindowImpl();

	bool											BeginRender();
	bool											EndRender();

	void											DrawTriangleList(
		const std::vector<Vertex>				&	vertices,
		const std::vector<VertexIndex_3>		&	indices,
		bool										filled						= true,
		vk2d::TextureResource					*	texture						= nullptr );

	void											DrawTriangleList(
		const std::vector<Vertex>				&	vertices,
		const std::vector<uint32_t>				&	raw_indices,
		bool										filled						= true,
		vk2d::TextureResource					*	texture						= nullptr );

	void											DrawLineList(
		const std::vector<Vertex>				&	vertices,
		const std::vector<VertexIndex_2>		&	indices,
		vk2d::TextureResource					*	texture						= nullptr,
		float										line_width					= 1.0f );

	void											DrawLineList(
		const std::vector<Vertex>				&	vertices,
		const std::vector<uint32_t>				&	raw_indices,
		vk2d::TextureResource					*	texture						= nullptr,
		float										line_width					= 1.0f );

	void											DrawPointList(
		const std::vector<Vertex>				&	vertices,
		vk2d::TextureResource					*	texture						= nullptr );

	void											DrawLine(
		Vector2d									point_1,
		Vector2d									point_2,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawBox(
		Vector2d									top_left,
		Vector2d									bottom_right,
		bool										filled						= true,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawCircle(
		Vector2d									top_left,
		Vector2d									bottom_right,
		bool										filled						= true,
		float										edge_count					= 64.0f,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawPie(
		Vector2d									top_left,
		Vector2d									bottom_right,
		float										begin_angle_radians,
		float										coverage,
		bool										filled						= true,
		float										edge_count					= 64.0f,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawPieBox(
		Vector2d									top_left,
		Vector2d									bottom_right,
		float										begin_angle_radians,
		float										coverage,
		bool										filled						= true,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawTexture(
		Vector2d									top_left,
		Vector2d									bottom_right,
		vk2d::TextureResource					*	texture,
		Color										color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	void											DrawMesh(
		const vk2d::Mesh						&	mesh );

	bool											SynchronizeFrame();

private:
	bool											RecreateResourcesAfterResizing();
	bool											CreateGLFWWindow();
	bool											CreateSurface();
	bool											CreateRenderPass();
	bool											CreateGraphicsPipelines();
	bool											CreateCommandPool();
	bool											AllocateCommandBuffers();

	// If old swapchain is still active, this function instead re-creates
	// the swapchain recycling old resources whenever possible.
	bool											CreateSwapchain();
	bool											CreateFramebuffers();
	bool											CreateWindowSynchronizationPrimitives();
	bool											CreateFrameSynchronizationPrimitives();

	void											CmdBindPipelineIfDifferent(
		VkCommandBuffer								command_buffer,
		vk2d::_internal::PipelineType				pipeline_type );

	void											CmdBindTextureIfDifferent(
		VkCommandBuffer								command_buffer,
		vk2d::TextureResource					*	texture );

	void											CmdSetLineWidthIfDifferent(
		VkCommandBuffer								command_buffer,
		float										line_width );


	vk2d::_internal::RendererImpl				*	renderer_parent							= {};
	vk2d::WindowCreateInfo							create_info_copy						= {};

	VkInstance										instance								= {};
	VkPhysicalDevice								physical_device							= {};
	VkDevice										device									= {};

	vk2d::_internal::ResolvedQueue					primary_render_queue					= {};

	vk2d::PFN_VK2D_ReportFunction					report_function							= {};

	GLFWwindow									*	glfw_window								= {};

	VkSurfaceKHR									surface									= {};
	VkSwapchainKHR									swapchain								= {};
	std::vector<VkImage>							swapchain_images						= {};
	std::vector<VkImageView>						swapchain_image_views					= {};
	VkRenderPass									render_pass								= {};

	VkCommandPool									command_pool							= {};
	std::vector<VkCommandBuffer>					render_command_buffers					= {};	// For more overlapped execution multiple command buffers are needed.
	VkCommandBuffer									mesh_transfer_command_buffer			= {};	// For data transfer each frame, this is small command buffer and can be re-recorded just before submitting the work.
	VkSemaphore										mesh_transfer_semaphore					= {};

	VkExtent2D										min_extent								= {};
	VkExtent2D										max_extent								= {};
	VkExtent2D										extent									= {};
	uint32_t										swapchain_image_count					= {};
	VkSurfaceFormatKHR								surface_format							= {};
	VkPresentModeKHR								present_mode							= {};
	VkSurfaceCapabilitiesKHR						surface_capabilities					= {};
	std::vector<VkFramebuffer>						framebuffers							= {};

	uint32_t										next_image								= {};
	uint32_t										previous_image							= {};
	VkFence											aquire_image_fence						= {};
	std::vector<VkSemaphore>						submit_to_present_semaphores			= {};
	std::vector<VkFence>							gpu_to_cpu_frame_fences					= {};
	bool											previous_frame_need_synchronization		= {};

	std::vector<VkPipeline>							pipelines								= {};

	vk2d::_internal::NextRenderCallFunction			next_render_call_function				= vk2d::_internal::NextRenderCallFunction::BEGIN;
	bool											recreate_swapchain						= {};

	vk2d::_internal::PipelineType					previous_pipeline_type					= vk2d::_internal::PipelineType::NONE;
	VkDescriptorSet									previous_texture_descriptor_set			= {};
	float											previous_line_width						= {};

	std::unique_ptr<vk2d::_internal::MeshBuffer>	mesh_buffer								= {};

	bool											is_good									= {};
};



} // _internal

} // vk2d
