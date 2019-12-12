#pragma once

#include "../Core/SourceCommon.h"

#include "../../../Include/Interface/Renderer.h"
#include "../../../Include/Interface/Window.h"

#include "../../Header/Core/MeshBuffer.h"
#include "../Core/QueueResolver.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

namespace vk2d {

class Window;
class Cursor;
class TextureResource;
class Mesh;

namespace _internal {

class RendererImpl;
class WindowImpl;
class CursorImpl;
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

	friend void glfwWindowPosCallback( GLFWwindow * glfwWindow, int x, int y );
	friend void glfwWindowSizeCallback( GLFWwindow * glfwWindow, int x, int y );
	friend void glfwWindowCloseCallback( GLFWwindow * glfwWindow );
	friend void glfwWindowRefreshCallback( GLFWwindow * glfwWindow );
	friend void glfwWindowFocusCallback( GLFWwindow * glfwWindow, int focus );
	friend void glfwWindowIconifyCallback( GLFWwindow * glfwWindow, int iconify );
	friend void glfwFramebufferSizeCallback( GLFWwindow * glfwWindow, int x, int y );
	friend void glfwMouseButtonCallback( GLFWwindow * glfwWindow, int button, int action, int mods );
	friend void glfwCursorPosCallback( GLFWwindow * glfwWindow, double x, double y );
	friend void glfwCursorEnterCallback( GLFWwindow * glfwWindow, int enter );
	friend void glfwScrollCallback( GLFWwindow * glfwWindow, double x, double y );
	friend void glfwKeyCallback( GLFWwindow * glfwWindow, int key, int scancode, int action, int mods );
	friend void glfwCharModsCallback( GLFWwindow * glfwWindow, unsigned int codepoint, int mods );
	friend void glfwFileDropCallback( GLFWwindow * glfwWindow, int fileCount, const char ** filePaths );

public:
	WindowImpl(
		vk2d::Window							*	window,
		vk2d::_internal::RendererImpl			*	renderer,
		const vk2d::WindowCreateInfo			&	window_create_info
	);
	~WindowImpl();

	void											CloseWindow();
	bool											ShouldClose();

	bool											BeginRender();
	bool											EndRender();

	void											UpdateEvents();

	void											TakeScreenshot(
		std::filesystem::path						save_path );

	void											Focus();

	void											SetOpacity(
		float										opacity );

	float											GetOpacity();

	void											Hide(
		bool										hidden );

	bool											IsHidden();

	void											DisableEvents(
		bool										disable_events );

	bool											IsEventsDisabled();

	void											SetFullscreen(
		vk2d::Monitor							*	monitor,
		uint32_t									frequency );

	bool											IsFullscreen();

	std::array<double, 2>							GetCursorPosition();

	void											SetCursorPosition(
		double										x,
		double										y );

	void											SetCursor(
		vk2d::Cursor							*	cursor );

	std::string										GetClipboardString();

	void											SetClipboardString(
		const std::string						&	str );

	void											SetTitle(
		const std::string						&	title );

	std::string										GetTitle();

	void											SetIcon(
		const std::vector<std::filesystem::path>&	image_paths );

	void											SetPosition(
		int32_t										x,
		int32_t										y );

	std::array<int32_t, 2>							GetPosition();

	void											Iconify(
		bool										minimized );

	bool											IsIconified();

	void											SetMaximized(
		bool										maximized );

	bool											GetMaximized();

	void											SetCursorState(
		vk2d::CursorState							new_state );

	vk2d::CursorState								GetCursorState();

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

	vk2d::Window								*	window_parent							= {};
	vk2d::_internal::RendererImpl				*	renderer_parent							= {};
	vk2d::WindowCreateInfo							create_info_copy						= {};

	vk2d::WindowEventHandler					*	event_handler							= {};
	VkOffset2D										position								= {};
	bool											is_minimized							= {};

	struct IconData {
		std::vector<uint8_t>						image_data								= {};
		GLFWimage									glfw_image								= {};
	};
	std::vector<IconData>							icon_data								= {};

	VkInstance										instance								= {};
	VkPhysicalDevice								physical_device							= {};
	VkDevice										device									= {};

	vk2d::_internal::ResolvedQueue					primary_render_queue					= {};

	vk2d::PFN_VK2D_ReportFunction					report_function							= {};

	GLFWwindow									*	glfw_window								= {};
	std::string										window_title							= {};

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
	bool											should_reconstruct						= {};
	bool											should_close							= {};

	vk2d::_internal::PipelineType					previous_pipeline_type					= vk2d::_internal::PipelineType::NONE;
	VkDescriptorSet									previous_texture_descriptor_set			= {};
	float											previous_line_width						= {};

	std::unique_ptr<vk2d::_internal::MeshBuffer>	mesh_buffer								= {};

	bool											is_good									= {};
};







class CursorImpl {
	friend class vk2d::_internal::WindowImpl;

public:
												CursorImpl(
		const std::filesystem::path			&	image_path,
		int32_t									hot_spot_x,
		int32_t									hot_spot_y );

	// Cursor constructor, raw data version.
	// Image data needs to be in format RGBA, 8 bits per channel, 32 bits per pixel,
	// in order left to right - top to bottom.
	// [in] image_size: size of the image in pixels, x dimension.
	// [in] image_size: size of the image in pixels, y dimension.
	// [in] image_data: raw image data.
	// [in] hot_spot_x: where the active location of the cursor is, x location.
	// [in] hot_spot_y: where the active location of the cursor is, y location.
												CursorImpl(
		uint32_t								image_size_x,
		uint32_t								image_size_y,
		const std::vector<vk2d::Color>		&	image_data,
		int32_t									hot_spot_x,
		int32_t									hot_spot_y );

	// Copy constructor from another cursor.
												CursorImpl(
		const vk2d::_internal::CursorImpl	&	other );

	// Move constructor from another cursor.
												CursorImpl(
		vk2d::_internal::CursorImpl			&&	other )							= default;

	// Destructor for cursor.
												~CursorImpl();

	// Copy operator from another cursor.
	vk2d::_internal::CursorImpl				&	operator=(
		vk2d::_internal::CursorImpl			&	other );

	// Move operator from another cursor.
	vk2d::_internal::CursorImpl				&	operator=(
		vk2d::_internal::CursorImpl			&&	other )							= default;

	bool										IsGood();

	const std::vector<vk2d::Color>			&	GetPixelData();
	GLFWcursor								*	GetGLFWcursor();
	std::array<uint32_t, 2>						GetExtent();
	std::array<int32_t, 2>						GetHotSpot();

private:
	std::vector<vk2d::Color>					pixel_data						= {};
	GLFWcursor								*	cursor							= nullptr;
	VkExtent2D									extent							= {};
	VkOffset2D									hotSpot							= {};

	bool										is_good							= {};
};







// Monitor object holds information about the physical monitor
class MonitorImpl {
	friend class vk2d::Window;
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::Monitor;

public:
	MonitorImpl(
		GLFWmonitor									*	monitor,
		VkOffset2D										position,
		VkExtent2D										physical_size,
		std::string										name,
		vk2d::MonitorVideoMode							current_video_mode,
		const std::vector<vk2d::MonitorVideoMode>	&	video_modes );

														MonitorImpl()						= delete;

														MonitorImpl(
		const vk2d::_internal::MonitorImpl			&	other )								= default;

														MonitorImpl(
		vk2d::_internal::MonitorImpl				&&	other )								= default;

														~MonitorImpl()						= default;

	const vk2d::MonitorVideoMode					&	GetCurrentVideoMode() const;

	const std::vector<vk2d::MonitorVideoMode>		&	GetVideoModes() const;

	void												SetGamma(
		float											gamma );

	vk2d::GammaRamp										GetGammaRamp();

	void												SetGammaRamp(
		const vk2d::GammaRamp						&	ramp );

	vk2d::_internal::MonitorImpl					&	operator=(
		const vk2d::_internal::MonitorImpl			&	other )								= default;

	vk2d::_internal::MonitorImpl					&	operator=(
		vk2d::_internal::MonitorImpl				&&	other )								= default;

	bool												IsGood();

private:
	GLFWmonitor										*	monitor								= nullptr;
	VkOffset2D											position							= {};
	VkExtent2D											physical_size						= {};
	std::string											name								= "";
	MonitorVideoMode									current_video_mode					= {};
	std::vector<MonitorVideoMode>						video_modes;

	bool												is_good								= {};
};







} // _internal

} // vk2d
