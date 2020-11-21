#pragma once

#include "../Core/SourceCommon.h"

#include "../Types/Synchronization.hpp"

#include "../System/MeshBuffer.h"
#include "../System/QueueResolver.h"
#include "../System/VulkanMemoryManagement.h"
#include "../System/DescriptorSet.h"
#include "../System/ShaderInterface.h"
#include "../System/RenderTargetTextureDependecyGraphInfo.hpp"

#include "../../../Include/Interface/Instance.h"
#include "InstanceImpl.h"

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

namespace _internal {

class InstanceImpl;
class WindowImpl;
class MonitorImpl;
class CursorImpl;
class MeshBuffer;
class TextureResourceImpl;
class ScreenshotSaverTask;
class GraphicsShaderProgram;

bool											AquireImage(
	vk2d::_internal::WindowImpl				*	data,
	VkPhysicalDevice							physical_device,
	VkDevice									device,
	vk2d::_internal::ResolvedQueue			&	primary_render_queue,
	uint32_t									nested_counter );

enum class NextRenderCallFunction : uint32_t {
	BEGIN		= 0,
	END			= 1,
};

class WindowImpl {
	friend class vk2d::Window;
	friend class vk2d::_internal::ScreenshotSaverTask;
	friend bool										vk2d::_internal::AquireImage(
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
		vk2d::Window										*	window,
		vk2d::_internal::InstanceImpl						*	instance,
		const vk2d::WindowCreateInfo						&	window_create_info
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
		vk2d::Monitor										*	monitor,
		uint32_t												frequency );

	bool														IsFullscreen();

	vk2d::Vector2d												GetCursorPosition();

	void														SetCursorPosition(
		vk2d::Vector2d											new_position );

	void														SetCursor(
		vk2d::Cursor										*	cursor );

	std::string													GetClipboardString();

	void														SetClipboardString(
		const std::string									&	str );

	void														SetTitle(
		const std::string									&	title );

	std::string													GetTitle();

	void														SetIcon(
		const std::vector<std::filesystem::path>			&	image_paths );

	void														SetPosition(
		vk2d::Vector2i											new_position );

	vk2d::Vector2i												GetPosition();

	void														SetSize(
		vk2d::Vector2u											new_size );

	vk2d::Vector2u												GetSize();

	void														Iconify(
		bool													minimized );

	bool														IsIconified();

	void														SetMaximized(
		bool													maximized );

	bool														GetMaximized();

	void														SetCursorState(
		vk2d::CursorState										new_state );

	vk2d::CursorState											GetCursorState();

	void														DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		bool													solid						= true,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	void														DrawTriangleList(
		const std::vector<uint32_t>							&	raw_indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		bool													solid						= true,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	void														DrawLineList(
		const std::vector<vk2d::VertexIndex_2>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr,
		float													line_width					= 1.0f );

	void														DrawLineList(
		const std::vector<uint32_t>							&	raw_indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr,
		float													line_width					= 1.0f );

	void														DrawPointList(
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	void														DrawMesh(
		const vk2d::Mesh									&	mesh );

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
		vk2d::_internal::RenderTargetTextureRenderCollector	&	collector );

	void														ConfirmRenderTargetTextureRenderSubmission();
	void														ConfirmRenderTargetTextureRenderFinished(
		uint32_t												for_frame_image_index );

	// In case something goes wrong, allows cancelling render commitment.
	void														AbortRenderTargetTextureRender();

//	void														ClearRenderTargetTextureDepencies();
	void														CheckAndAddRenderTargetTextureDependency(
		vk2d::Texture										*	texture );

	void														HandleScreenshotEvent();

	void														CmdBindGraphicsPipelineIfDifferent(
		VkCommandBuffer											command_buffer,
		const vk2d::_internal::GraphicsPipelineSettings		&	pipeline_settings );

	/*
	void														CmdBindTextureSamplerIfDifferent(
		VkCommandBuffer											command_buffer,
		vk2d::Sampler										*	sampler,
		vk2d::Texture										*	texture );
	*/

	void														CmdBindSamplerIfDifferent(
		VkCommandBuffer											command_buffer,
		vk2d::Sampler										*	sampler );

	void														CmdBindTextureIfDifferent(
		VkCommandBuffer											command_buffer,
		vk2d::Texture										*	texture );

	void														CmdSetLineWidthIfDifferent(
		VkCommandBuffer											command_buffer,
		float													line_width );

	bool														CmdUpdateFrameData(
		VkCommandBuffer											command_buffer );

	vk2d::Window											*	my_interface								= {};
	vk2d::_internal::InstanceImpl							*	instance									= {};
	vk2d::WindowCreateInfo										create_info_copy							= {};

	vk2d::WindowEventHandler								*	event_handler								= {};
	VkOffset2D													position									= {};
	bool														is_minimized								= {};

	struct IconData {
		std::vector<uint8_t>									image_data									= {};
		GLFWimage												glfw_image									= {};
	};
	std::vector<vk2d::_internal::WindowImpl::IconData>			icon_data									= {};

	VkInstance													vk_instance									= {};
	VkPhysicalDevice											vk_physical_device							= {};
	VkDevice													vk_device									= {};

	vk2d::_internal::ResolvedQueue								primary_render_queue						= {};
	vk2d::_internal::ResolvedQueue								primary_compute_queue						= {};

	vk2d::PFN_VK2D_ReportFunction								report_function								= {};

	GLFWwindow												*	glfw_window									= {};
	std::string													window_title								= {};

	vk2d::Multisamples											samples										= {};

	VkSurfaceKHR												vk_surface									= {};
	VkSwapchainKHR												vk_swapchain								= {};
	std::vector<VkImage>										vk_swapchain_images							= {};
	std::vector<VkImageView>									vk_swapchain_image_views					= {};
	VkRenderPass												vk_render_pass								= {};
	std::vector<vk2d::_internal::CompleteImageResource>			multisample_render_targets					= {};

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
	VkFence														vk_aquire_image_fence						= {};
	std::vector<VkSemaphore>									vk_submit_to_present_semaphores				= {};
	std::vector<VkFence>										vk_gpu_to_cpu_frame_fences					= {};
	bool														previous_frame_need_synchronization			= {};

	vk2d::_internal::CompleteBufferResource						frame_data_staging_buffer					= {};
	vk2d::_internal::CompleteBufferResource						frame_data_device_buffer					= {};
	vk2d::_internal::PoolDescriptorSet							frame_data_descriptor_set					= {};

	vk2d::_internal::NextRenderCallFunction						next_render_call_function					= vk2d::_internal::NextRenderCallFunction::BEGIN;
	bool														should_reconstruct							= {};
	bool														should_close								= {};

	vk2d::_internal::GraphicsPipelineSettings					previous_pipeline_settings					= {};
	vk2d::Texture											*	previous_texture							= {};
	vk2d::Sampler											*	previous_sampler							= {};
	float														previous_line_width							= {};

	std::map<vk2d::Sampler*, vk2d::_internal::TimedDescriptorPoolData>
																sampler_descriptor_sets						= {};

	std::map<vk2d::Texture*, vk2d::_internal::TimedDescriptorPoolData>
																texture_descriptor_sets						= {};

	std::unique_ptr<vk2d::_internal::MeshBuffer>				mesh_buffer									= {};

	std::vector<std::vector<vk2d::_internal::RenderTargetTextureDependencyInfo>>
																render_target_texture_dependencies			= {};

	enum class ScreenshotState : uint32_t {
		IDLE					= 0,	// doing nothing
		REQUESTED,						// screenshot requested but nothing done yet
		WAITING_RENDER,					// waiting for rendering to happen
		WAITING_FILE_WRITE,				// waiting for file to be written
		WAITING_EVENT_REPORT,			// waiting for system to report event in main thread after saving
		IDLE_ERROR				= UINT32_MAX,	// error state, screenshots disabled
	};
	std::atomic<vk2d::_internal::WindowImpl::ScreenshotState>	screenshot_state						= {};
	std::filesystem::path										screenshot_save_path					= {};
	vk2d::ImageData												screenshot_save_data					= {};
	bool														screenshot_alpha						= {};
	vk2d::_internal::CompleteImageResource						screenshot_image						= {};
	vk2d::_internal::CompleteBufferResource						screenshot_buffer						= {};
	uint32_t													screenshot_swapchain_id					= {};
	std::atomic_bool											screenshot_being_saved					= {};
	bool														screenshot_event_error					= {};
	std::string													screenshot_event_message				= {};

	bool														is_good									= {};
};







class CursorImpl {
	friend class vk2d::_internal::WindowImpl;

public:
												CursorImpl(
		vk2d::_internal::InstanceImpl		*	instance,
		const std::filesystem::path			&	image_path,
		vk2d::Vector2i							hot_spot );

	// Cursor constructor, raw data version.
	// Image data needs to be in format RGBA, 8 bits per channel, 32 bits per pixel,
	// in order left to right - top to bottom.
	// [in] image_size: size of the image in pixels, x dimension.
	// [in] image_size: size of the image in pixels, y dimension.
	// [in] image_data: raw image data.
	// [in] hot_spot_x: where the active location of the cursor is, x location.
	// [in] hot_spot_y: where the active location of the cursor is, y location.
												CursorImpl(
		vk2d::_internal::InstanceImpl		*	instance,
		vk2d::Vector2u							image_size,
		const std::vector<vk2d::Color8>		&	image_data,
		vk2d::Vector2i							hot_spot );

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

	vk2d::_internal::InstanceImpl			*	GetInstance();
	const std::vector<vk2d::Color8>			&	GetPixelData();
	GLFWcursor								*	GetGLFWcursor();
	vk2d::Vector2u								GetSize();
	vk2d::Vector2i								GetHotSpot();

private:
	vk2d::_internal::InstanceImpl			*	instance					= {};
	std::vector<vk2d::Color8>					pixel_data						= {};
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
	GLFWmonitor										*	monitor								= {};
	VkOffset2D											position							= {};
	VkExtent2D											physical_size						= {};
	std::string											name								= {};
	MonitorVideoMode									current_video_mode					= {};
	std::vector<MonitorVideoMode>						video_modes;

	bool												is_good								= {};
};







} // _internal

} // vk2d
