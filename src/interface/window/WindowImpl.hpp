#pragma once

#include <core/SourceCommon.hpp>

#include <interface/Window.hpp>

#include <types/Synchronization.hpp>

#include <system/MeshBuffer.hpp>
#include <vulkan/utils/QueueResolver.hpp>
#include <vulkan/utils/VulkanMemoryManagement.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>
#include <vulkan/shaders/ShaderInterface.hpp>
#include <system/RenderTargetTextureDependecyGraphInfo.hpp>
#include <mesh/vertex/RawVertexData.hpp>

#include <interface/Instance.hpp>
#include <interface/instance/InstanceImpl.hpp>

#include <vector>
#include <memory>
#include <atomic>
#include <map>
#include <span>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace vk2d {

class Window;
class Monitor;
class Cursor;
class TextureResource;
class MeshBase;

namespace vulkan {
class GraphicsShaderList;
} // vulkan

namespace vk2d_internal {

class InstanceImpl;
class WindowImpl;
class MonitorImpl;
class CursorImpl;
class MeshBuffer;
class TextureResourceImpl;
class ScreenshotSaverTask;



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
		Window												&	window,
		InstanceImpl										&	instance,
		const WindowCreateInfo								&	window_create_info
	);
	~WindowImpl();

	void														CloseWindow();
	bool														ShouldClose();

	bool														BeginRender();
	bool														EndRender();

	void														TakeScreenshotToFile(
		const std::filesystem::path							&	save_path,
		bool													include_alpha
	);

	void														TakeScreenshotToData(
		bool													include_alpha
	);

	void														Focus();

	void														SetOpacity(
		float													opacity
	);

	float														GetOpacity();

	void														Hide(
		bool													hidden
	);

	bool														IsHidden();

	void														DisableEvents(
		bool													disable_events
	);

	bool														AreEventsDisabled();

	void														SetFullscreen(
		Monitor												*	monitor,
		uint32_t												frequency
	);

	bool														IsFullscreen();

	glm::dvec2													GetCursorPosition();

	void														SetCursorPosition(
		glm::dvec2												new_position
	);

	void														SetCursor(
		Cursor												*	cursor
	);

	std::string													GetClipboardString();

	void														SetClipboardString(
		const std::string									&	str
	);

	void														SetTitle(
		const std::string									&	title );

	std::string													GetTitle();

	void														SetIcon(
		const std::vector<std::filesystem::path>			&	image_paths
	);

	void														SetPosition(
		glm::ivec2												new_position
	);

	glm::ivec2													GetPosition();

	void														SetSize(
		glm::uvec2												new_size
	);

	glm::uvec2													GetSize();

	void														Iconify(
		bool													minimized
	);

	bool														IsIconified();

	void														SetMaximized(
		bool													maximized
	);

	bool														GetMaximized();

	void														SetCursorState(
		CursorState												new_state
	);

	CursorState													GetCursorState();

	void														SetRenderCoordinateSpace(
		RenderCoordinateSpace									coordinate_space
	);

	void														DrawPointList(
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		Texture												*	texture,
		Sampler												*	sampler
	);

	void														DrawLineList(
		std::span<const uint32_t>								raw_indices,
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		Texture												*	texture,
		Sampler												*	sampler,
		float													line_width
	);

	void														DrawTriangleList(
		std::span<const uint32_t>								raw_indices,
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		bool													solid,
		Texture												*	texture,
		Sampler												*	sampler
	);

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
	// SynchronizeFrame() will start blocking until the contents of the
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
		const vulkan::GraphicsPipelineSettings				&	pipeline_settings );

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

	InstanceImpl											&	instance;
	Window													&	my_interface;
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

	vulkan::DeviceMemoryPool								&	device_memory_pool;

	vulkan::Queue										primary_render_queue						= {};
	vulkan::Queue										primary_compute_queue						= {};

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
	std::vector<vulkan::CompleteImageResource>					multisample_render_targets					= {};

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

	vulkan::CompleteBufferResource								frame_data_staging_buffer					= {};
	vulkan::CompleteBufferResource								frame_data_device_buffer					= {};
	vulkan::PoolDescriptorSet									frame_data_descriptor_set					= {};

	NextRenderCallFunction										next_render_call_function					= NextRenderCallFunction::BEGIN;
	bool														should_reconstruct							= {};
	bool														should_close								= {};

	vulkan::GraphicsPipelineSettings							previous_pipeline_settings					= {};
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
	vulkan::CompleteImageResource								screenshot_image							= {};
	vulkan::CompleteBufferResource								screenshot_buffer							= {};
	uint32_t													screenshot_swapchain_id						= {};
	bool														screenshot_event_error						= {};
	std::string													screenshot_event_message					= {};

	bool														is_good										= {};
};



} // vk2d_internal

} // vk2d
