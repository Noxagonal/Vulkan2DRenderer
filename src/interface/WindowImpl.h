#pragma once

#include "core/SourceCommon.h"

#include "types/Synchronization.hpp"

#include "system/MeshBuffer.h"
#include "system/QueueResolver.h"
#include "system/VulkanMemoryManagement.h"
#include "system/DescriptorSet.h"
#include "system/ShaderInterface.h"
#include "system/RenderTargetTextureDependecyGraphInfo.hpp"

#include "interface/Instance.h"
#include "interface/InstanceImpl.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <atomic>
#include <map>

namespace vk2d {

class Window;
class Monitor;
class Cursor;
class TextureResource;
class Mesh;

namespace vk2d_internal {

class InstanceImpl;
class WindowImpl;
class MonitorImpl;
class CursorImpl;
class MeshBuffer;
class TextureResourceImpl;
class ScreenshotSaverTask;
class GraphicsShaderProgram;

enum class NextRenderCallFunction : uint32_t {
	BEGIN		= 0,
	END			= 1,
};

class WindowImpl {
	friend class Window;
	friend class ScreenshotSaverTask;

	friend void glfwWindowPosCallback( GLFWwindow * glfwWindow, int x, int y );
	friend void glfwWindowSizeCallback( GLFWwindow * glfwWindow, int x, int y );
	friend void glfwWindowCloseCallback( GLFWwindow * glfwWindow );
	friend void glfwWindowRefreshCallback( GLFWwindow * glfwWindow );
	friend void glfwWindowFocusCallback( GLFWwindow * glfwWindow, int focus );
	friend void glfwWindowIconifyCallback( GLFWwindow * glfwWindow, int iconify );
	friend void glfwWindowMaximizeCallback( GLFWwindow * glfwWindow, int maximized );
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
		Window												*	window,
		InstanceImpl										*	instance,
		const WindowCreateInfo								&	window_create_info
	);
	~WindowImpl();

	void														CloseWindow();
	bool														ShouldClose();

	bool														BeginRender();
	bool														EndRender();

	void														TakeScreenshotToFile(
		const std::filesystem::path							&	save_path,
		bool													include_alpha );

	void														TakeScreenshotToData(
		bool													include_alpha);

	void														Focus();

	void														SetOpacity(
		float													opacity );

	float														GetOpacity();

	void														Hide(
		bool													hidden );

	bool														IsHidden();

	void														DisableEvents(
		bool													disable_events );

	bool														AreEventsDisabled();

	void														SetFullscreen(
		Monitor												*	monitor,
		uint32_t												frequency );

	bool														IsFullscreen();

	glm::dvec2													GetCursorPosition();

	void														SetCursorPosition(
		glm::dvec2												new_position );

	void														SetCursor(
		Cursor												*	cursor );

	std::string													GetClipboardString();

	void														SetClipboardString(
		const std::string									&	str );

	void														SetTitle(
		const std::string									&	title );

	std::string													GetTitle();

	void														SetIcon(
		const std::vector<std::filesystem::path>			&	image_paths );

	void														SetPosition(
		glm::ivec2												new_position );

	glm::ivec2													GetPosition();

	void														SetSize(
		glm::uvec2												new_size );

	glm::uvec2													GetSize();

	void														Iconify(
		bool													minimized );

	bool														IsIconified();

	void														SetMaximized(
		bool													maximized );

	bool														GetMaximized();

	void														SetCursorState(
		CursorState												new_state );

	CursorState													GetCursorState();

	void														SetRenderCoordinateSpace(
		RenderCoordinateSpace									coordinate_space );

	void														DrawTriangleList(
		const std::vector<VertexIndex_3>					&	indices,
		const std::vector<Vertex>							&	vertices,
		const std::vector<float>							&	texture_layer_weights,
		const std::vector<glm::mat4>						&	transformations,
		bool													solid,
		Texture												*	texture,
		Sampler												*	sampler );

	void														DrawTriangleList(
		const std::vector<uint32_t>							&	raw_indices,
		const std::vector<Vertex>							&	vertices,
		const std::vector<float>							&	texture_layer_weights,
		const std::vector<glm::mat4>						&	transformations,
		bool													solid,
		Texture												*	texture,
		Sampler												*	sampler);

	void														DrawLineList(
		const std::vector<VertexIndex_2>					&	indices,
		const std::vector<Vertex>							&	vertices,
		const std::vector<float>							&	texture_layer_weights,
		const std::vector<glm::mat4>						&	transformations,
		Texture												*	texture,
		Sampler												*	sampler,
		float													line_width );

	void														DrawLineList(
		const std::vector<uint32_t>							&	raw_indices,
		const std::vector<Vertex>							&	vertices,
		const std::vector<float>							&	texture_layer_weights,
		const std::vector<glm::mat4>						&	transformations,
		Texture												*	texture,
		Sampler												*	sampler,
		float													line_width );

	void														DrawPointList(
		const std::vector<Vertex>							&	vertices,
		const std::vector<float>							&	texture_layer_weights,
		const std::vector<glm::mat4>						&	transformations,
		Texture												*	texture,
		Sampler												*	sampler );

	void														DrawMesh(
		const Mesh											&	mesh,
		const std::vector<glm::mat4>						&	transformations );

	bool														SynchronizeFrame();

	bool														IsGood();

private:
	bool														RecreateWindowSizeDependantResources();
	bool														CreateGLFWWindow();
	bool														CreateSurface();
	bool														CreateRenderPass();
	bool														CreateCommandPool();
	bool														AllocateCommandBuffers();

	// If old swapchain is still active, this function instead re-creates
	// the swapchain recycling old resources whenever possible.
	bool														ReCreateSwapchain();
	bool														ReCreateScreenshotResources();
	bool														CreateFramebuffers();
	bool														CreateWindowSynchronizationPrimitives();
	bool														CreateFrameSynchronizationPrimitives();
	bool														CreateWindowFrameDataBuffer();

	// Should be called once render is definitely going to happen. When this is called,
	// SynchronizeFrame() will start blocking until the the contents of the
	// RenderTargerTexture have been fully rendered. BeginRender() can be called however,
	// it will swap the buffers so 2 renders can be queued, however third call to
	// BeginRender() will be blocked until the first BeginRender() call has been rendered.
	bool														CommitRenderTargetTextureRender(
		RenderTargetTextureRenderCollector					&	collector );

	void														ConfirmRenderTargetTextureRenderSubmission();
	void														ConfirmRenderTargetTextureRenderFinished(
		uint32_t												for_frame_image_index );

	// In case something goes wrong, allows cancelling render commitment.
	void														AbortRenderTargetTextureRender();

	void														CheckAndAddRenderTargetTextureDependency(
		Texture												*	texture );

	void														HandleScreenshotEvent();

	void														CmdBindGraphicsPipelineIfDifferent(
		VkCommandBuffer											command_buffer,
		const GraphicsPipelineSettings						&	pipeline_settings );

	void														CmdBindSamplerIfDifferent(
		VkCommandBuffer											command_buffer,
		Sampler												*	sampler );

	void														CmdBindTextureIfDifferent(
		VkCommandBuffer											command_buffer,
		Texture												*	texture );

	void														CmdSetLineWidthIfDifferent(
		VkCommandBuffer											command_buffer,
		float													line_width );

	bool														CmdUpdateFrameData(
		VkCommandBuffer											command_buffer );

	Window													*	my_interface								= {};
	InstanceImpl											*	instance									= {};
	WindowCreateInfo											create_info_copy							= {};

	WindowEventHandler										*	event_handler								= {};
	VkOffset2D													position									= {};
	bool														is_iconified								= {};
	bool														is_maximized								= {};

	struct IconData {
		std::vector<uint8_t>									image_data									= {};
		GLFWimage												glfw_image									= {};
	};
	std::vector<WindowImpl::IconData>							icon_data									= {};

	VkInstance													vk_instance									= {};
	VkPhysicalDevice											vk_physical_device							= {};
	VkDevice													vk_device									= {};

	ResolvedQueue												primary_render_queue						= {};
	ResolvedQueue												primary_compute_queue						= {};

	PFN_VK2D_ReportFunction										report_function								= {};

	GLFWwindow												*	glfw_window									= {};
	std::string													window_title								= {};

	RenderCoordinateSpace										coordinate_space							= {};
	Multisamples												samples										= {};

	VkSurfaceKHR												vk_surface									= {};
	VkSwapchainKHR												vk_swapchain								= {};
	std::vector<VkImage>										vk_swapchain_images							= {};
	std::vector<VkImageView>									vk_swapchain_image_views					= {};
	VkRenderPass												vk_render_pass								= {};
	std::vector<CompleteImageResource>							multisample_render_targets					= {};

	VkCommandPool												vk_command_pool								= {};
	std::vector<VkCommandBuffer>								vk_render_command_buffers					= {};	// For more overlapped execution multiple command buffers are needed.
	VkCommandBuffer												vk_transfer_command_buffer					= {};	// For data transfer each frame, this is small command buffer and can be re-recorded just before submitting the work.
	VkSemaphore													vk_transfer_semaphore						= {};

	VkExtent2D													min_extent									= {};
	VkExtent2D													max_extent									= {};
	VkExtent2D													extent										= {};
	uint32_t													swapchain_image_count						= {};
	VkSurfaceFormatKHR											surface_format								= {};
	VkPresentModeKHR											present_mode								= {};
	VkSurfaceCapabilitiesKHR									surface_capabilities						= {};
	std::vector<VkFramebuffer>									vk_framebuffers								= {};

	uint32_t													next_image									= {};
	uint32_t													previous_image								= {};
	VkFence														vk_acquire_image_fence						= {};
	std::vector<VkSemaphore>									vk_submit_to_present_semaphores				= {};
	std::vector<VkFence>										vk_gpu_to_cpu_frame_fences					= {};
	bool														previous_frame_need_synchronization			= {};

	CompleteBufferResource										frame_data_staging_buffer					= {};
	CompleteBufferResource										frame_data_device_buffer					= {};
	PoolDescriptorSet											frame_data_descriptor_set					= {};

	NextRenderCallFunction										next_render_call_function					= NextRenderCallFunction::BEGIN;
	bool														should_reconstruct							= {};
	bool														should_close								= {};

	GraphicsPipelineSettings									previous_pipeline_settings					= {};
	Texture													*	previous_texture							= {};
	Sampler													*	previous_sampler							= {};
	float														previous_line_width							= {};

	std::map<Sampler*, TimedDescriptorPoolData>
																sampler_descriptor_sets						= {};

	std::map<Texture*, TimedDescriptorPoolData>
																texture_descriptor_sets						= {};

	std::unique_ptr<MeshBuffer>									mesh_buffer									= {};

	std::vector<std::vector<RenderTargetTextureDependencyInfo>>	render_target_texture_dependencies			= {};

	enum class ScreenshotState : uint32_t {
		IDLE					= 0,			// doing nothing
		REQUESTED,								// screenshot requested but nothing done yet
		WAITING_RENDER,							// waiting for rendering to happen
		WAITING_FILE_WRITE,						// waiting for file to be written
		WAITING_EVENT_REPORT,					// waiting for system to report event in main thread after saving
		IDLE_ERROR				= UINT32_MAX,	// error state, screenshots disabled
	};

	std::atomic<WindowImpl::ScreenshotState>					screenshot_state							= {};
	std::filesystem::path										screenshot_save_path						= {};
	ImageData													screenshot_save_data						= {};
	bool														screenshot_alpha							= {};
	CompleteImageResource										screenshot_image							= {};
	CompleteBufferResource										screenshot_buffer							= {};
	uint32_t													screenshot_swapchain_id						= {};
	bool														screenshot_event_error						= {};
	std::string													screenshot_event_message					= {};

	bool														is_good										= {};
};







class CursorImpl {
	friend class WindowImpl;

public:
												CursorImpl(
		InstanceImpl						*	instance,
		const std::filesystem::path			&	image_path,
		glm::ivec2								hot_spot );

	// Cursor constructor, raw data version.
	// Image data needs to be in format RGBA, 8 bits per channel, 32 bits per pixel,
	// in order left to right - top to bottom.
	// [in] image_size: size of the image in pixels, x dimension.
	// [in] image_size: size of the image in pixels, y dimension.
	// [in] image_data: raw image data.
	// [in] hot_spot_x: where the active location of the cursor is, x location.
	// [in] hot_spot_y: where the active location of the cursor is, y location.
												CursorImpl(
		InstanceImpl						*	instance,
		glm::uvec2								image_size,
		const std::vector<Color8>			&	image_data,
		glm::ivec2								hot_spot );

	// Copy constructor from another cursor.
												CursorImpl(
		const CursorImpl					&	other );

	// Move constructor from another cursor.
												CursorImpl(
		CursorImpl							&&	other )							= default;

	// Destructor for cursor.
												~CursorImpl();

	// Copy operator from another cursor.
	CursorImpl								&	operator=(
		CursorImpl							&	other );

	// Move operator from another cursor.
	CursorImpl								&	operator=(
		CursorImpl							&&	other )							= default;

	bool										IsGood();

	InstanceImpl							*	GetInstance();
	const std::vector<Color8>				&	GetTexelData();
	GLFWcursor								*	GetGLFWcursor();
	glm::uvec2									GetSize();
	glm::ivec2									GetHotSpot();

private:
	InstanceImpl							*	instance						= {};
	std::vector<Color8>							pixel_data						= {};
	GLFWcursor								*	cursor							= nullptr;
	VkExtent2D									extent							= {};
	VkOffset2D									hotSpot							= {};

	bool										is_good							= {};
};







// Monitor object holds information about the physical monitor
class MonitorImpl {
	friend class Window;
	friend class WindowImpl;
	friend class Monitor;

public:
	MonitorImpl(
		GLFWmonitor									*	monitor,
		VkOffset2D										position,
		VkExtent2D										physical_size,
		std::string										name,
		MonitorVideoMode							current_video_mode,
		const std::vector<MonitorVideoMode>	&	video_modes );

														MonitorImpl()						= delete;

														MonitorImpl(
		const MonitorImpl			&	other )								= default;

														MonitorImpl(
		MonitorImpl				&&	other )								= default;

														~MonitorImpl()						= default;

	const MonitorVideoMode					&	GetCurrentVideoMode() const;

	const std::vector<MonitorVideoMode>		&	GetVideoModes() const;

	void												SetGamma(
		float											gamma );

	std::vector<GammaRampNode>					GetGammaRamp();

	void												SetGammaRamp(
		const std::vector<GammaRampNode>		&	ramp );

	MonitorImpl					&	operator=(
		const MonitorImpl			&	other )								= default;

	MonitorImpl					&	operator=(
		MonitorImpl				&&	other )								= default;

	bool												IsGood();

private:
	GLFWmonitor										*	monitor								= {};
	VkOffset2D											position							= {};
	VkExtent2D											physical_size						= {};
	std::string											name								= {};
	MonitorVideoMode									current_video_mode					= {};
	std::vector<MonitorVideoMode>						video_modes;

	bool												is_good								= {};
};







} // vk2d_internal

} // vk2d
