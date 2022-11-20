
#include <core/SourceCommon.hpp>

#include "../window/WindowImpl.hpp"
#include "../monitor/MonitorImpl.hpp"
#include "../cursor/CursorImpl.hpp"
#include "../render_target_texture/RenderTargetTextureImpl.hpp"
#include "../sampler/SamplerImpl.hpp"

#include <containers/Rect2.hpp>
#include <containers/Color.hpp>
#include <mesh/Mesh.hpp>

#include <system/MeshBuffer.hpp>
#include <system/ThreadPool.hpp>

#include <interface/instance/InstanceImpl.hpp>
#include <interface/resources/texture/TextureResourceImpl.hpp>


#include <mesh/AlignedMeshBufferOffsets.hpp>

#include <stb_image.h>
#include <stb_image_write.h>



namespace vk2d {
namespace vk2d_internal {



// Function declarations
void glfwJoystickEventCallback( int joystick, int event );
void glfwWindowPosCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwWindowSizeCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwWindowCloseCallback( GLFWwindow * glfwWindow );
void glfwWindowRefreshCallback( GLFWwindow * glfwWindow );
void glfwWindowFocusCallback( GLFWwindow * glfwWindow, int focus );
void glfwWindowIconifyCallback( GLFWwindow * glfwWindow, int iconify );
void glfwWindowMaximizeCallback( GLFWwindow * glfwWindow, int maximized );
void glfwFramebufferSizeCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwMouseButtonCallback( GLFWwindow * glfwWindow, int button, int action, int mods );
void glfwCursorPosCallback( GLFWwindow * glfwWindow, double x, double y );
void glfwCursorEnterCallback( GLFWwindow * glfwWindow, int enter );
void glfwScrollCallback( GLFWwindow * glfwWindow, double x, double y );
void glfwKeyCallback( GLFWwindow * glfwWindow, int key, int scancode, int action, int mods );
void glfwCharModsCallback( GLFWwindow * glfwWindow, unsigned int codepoint, int mods );
void glfwFileDropCallback( GLFWwindow * glfwWindow, int fileCount, const char ** filePaths );



} // vk2d_internal
} // vk2d



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal callback functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowPosCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	impl->position = { int32_t( x ), int32_t( y ) };
	if( impl->event_handler ) {
		impl->event_handler->EventWindowPosition( impl->my_interface, { int32_t( x ), int32_t( y ) } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowSizeCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->extent					= { uint32_t( x ), uint32_t( y ) };
	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowSize( impl->my_interface, { uint32_t( x ), uint32_t( y ) } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowCloseCallback(
	GLFWwindow		*	glfwWindow
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->should_close			= true;
//	glfwHideWindow( impl->glfw_window );
	if( impl->event_handler ) {
		impl->event_handler->EventWindowClose( impl->my_interface );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowRefreshCallback(
	GLFWwindow		*	glfwWindow
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowRefresh( impl->my_interface );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowFocusCallback(
	GLFWwindow		*	glfwWindow,
	int					focus
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	if( impl->event_handler ) {
		impl->event_handler->EventWindowFocus( impl->my_interface, bool( focus ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowIconifyCallback(
	GLFWwindow		*	glfwWindow,
	int					iconify
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	if( iconify ) {
		impl->is_iconified			= true;
	} else {
		impl->is_iconified			= false;
		impl->should_reconstruct	= true;
	}
	if( impl->event_handler ) {
		impl->event_handler->EventWindowIconify( impl->my_interface, bool( iconify ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwWindowMaximizeCallback(
	GLFWwindow		*	glfwWindow,
	int					maximized
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	if( maximized ) {
		impl->is_maximized			= true;
	} else {
		impl->is_maximized			= false;
		impl->should_reconstruct	= true;
	}
	if( impl->event_handler ) {
		impl->event_handler->EventWindowMaximize( impl->my_interface, bool( maximized ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwFramebufferSizeCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->extent					= { uint32_t( x ), uint32_t( y ) };
	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowSize( impl->my_interface, { uint32_t( x ), uint32_t( y ) } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwMouseButtonCallback(
	GLFWwindow		*	glfwWindow,
	int					button,
	int					action,
	int					mods
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventMouseButton( impl->my_interface, MouseButton( button ), ButtonAction( action ), ModifierKeyFlags( mods ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwCursorPosCallback(
	GLFWwindow		*	glfwWindow,
	double				x,
	double				y
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCursorPosition( impl->my_interface, { x, y } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwCursorEnterCallback(
	GLFWwindow		*	glfwWindow,
	int					enter
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCursorEnter( impl->my_interface, bool( enter ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwScrollCallback(
	GLFWwindow		*	glfwWindow,
	double				x,
	double				y
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventScroll( impl->my_interface, { x, y } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwKeyCallback(
	GLFWwindow		*	glfwWindow,
	int					key,
	int					scancode,
	int					action,
	int					mods
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventKeyboard( impl->my_interface, KeyboardButton( key ), scancode, ButtonAction( action ), ModifierKeyFlags( mods ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwCharModsCallback(
	GLFWwindow		*	glfwWindow,
	unsigned int		codepoint,
	int					mods
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCharacter( impl->my_interface, codepoint, ModifierKeyFlags( mods ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::glfwFileDropCallback(
	GLFWwindow		*	glfwWindow,
	int					fileCount,
	const char		**	filePaths
)
{
	auto impl = reinterpret_cast<WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		std::vector<std::filesystem::path> files( fileCount );
		for( int i = 0; i < fileCount; ++i ) {
			files[ i ]		= filePaths[ i ];
		}
		impl->event_handler->EventFileDrop( impl->my_interface, files );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::WindowImpl::WindowImpl(
	Window					&	window,
	InstanceImpl			&	instance,
	const WindowCreateInfo	&	window_create_info
) :
	my_interface( window ),
	instance( instance ),
	create_info_copy( window_create_info ),
	report_function( instance.GetReportFunction() ),
	window_title( window_create_info.title ),
	event_handler( window_create_info.event_handler ),
	coordinate_space( window_create_info.coordinate_space ),
	vk_instance( instance.GetVulkanInstance() ),
	vk_physical_device( instance.GetVulkanDevice().GetVulkanPhysicalDevice() ),
	vk_device( instance.GetVulkanDevice() ),
	primary_render_queue( instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_RENDER ) ),
	primary_compute_queue( instance.GetVulkanDevice().GetQueue( VulkanQueueType::PRIMARY_COMPUTE ) ),
	device_memory_pool( *instance.GetVulkanDevice().GetDeviceMemoryPool() )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	samples = CheckSupportedMultisampleCount( instance, create_info_copy.samples );

	if( !CreateGLFWWindow() ) return;
	if( !CreateSurface() ) return;
	if( !CreateRenderPass() ) return;
	if( !ReCreateSwapchain() ) return;
	ReCreateScreenshotResources();
	if( !CreateFramebuffers() ) return;
	if( !CreateCommandPool() ) return;
	if( !AllocateCommandBuffers() ) return;
	if( !CreateWindowSynchronizationPrimitives() ) return;
	if( !CreateFrameSynchronizationPrimitives() ) return;
	if( !CreateWindowFrameDataBuffer() ) return;

	mesh_buffer = std::make_unique<MeshBuffer>(
		instance
	);

	render_target_texture_dependencies.resize( swapchain_image_count );

	if( !this->mesh_buffer ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create MeshBuffer object!" );
		return;
	}

	{
		int32_t x = 0, y = 0;
		glfwGetWindowPos( glfw_window, &x, &y );
		this->position	= { x, y };

		glfwGetWindowSize( glfw_window, &x, &y );
		this->extent	= { uint32_t( x ), uint32_t( y ) };
	}
	{
		glfwSetWindowUserPointer( glfw_window, this );

		glfwSetWindowPosCallback( glfw_window, glfwWindowPosCallback );
		glfwSetWindowSizeCallback( glfw_window, glfwWindowSizeCallback );
		glfwSetWindowCloseCallback( glfw_window, glfwWindowCloseCallback );
		glfwSetWindowRefreshCallback( glfw_window, glfwWindowRefreshCallback );
		glfwSetWindowFocusCallback( glfw_window, glfwWindowFocusCallback );
		glfwSetWindowIconifyCallback( glfw_window, glfwWindowIconifyCallback );
		glfwSetWindowMaximizeCallback( glfw_window, glfwWindowMaximizeCallback );
		glfwSetFramebufferSizeCallback( glfw_window, glfwFramebufferSizeCallback );

		glfwSetMouseButtonCallback( glfw_window, glfwMouseButtonCallback );
		glfwSetCursorPosCallback( glfw_window, glfwCursorPosCallback );
		glfwSetCursorEnterCallback( glfw_window, glfwCursorEnterCallback );
		glfwSetScrollCallback( glfw_window, glfwScrollCallback );
		glfwSetKeyCallback( glfw_window, glfwKeyCallback );
		glfwSetCharModsCallback( glfw_window, glfwCharModsCallback );
		glfwSetDropCallback( glfw_window, glfwFileDropCallback );
	}

	this->is_good		= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::WindowImpl::~WindowImpl()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	vkDeviceWaitIdle( vk_device );

	while( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE ) {
		std::this_thread::sleep_for( std::chrono::microseconds( 500 ) );
	};
	if( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT ) {
		HandleScreenshotEvent();
	}

	device_memory_pool.FreeCompleteResource( screenshot_image );
	device_memory_pool.FreeCompleteResource( screenshot_buffer );

	mesh_buffer		= nullptr;

	instance.FreeDescriptorSet( frame_data_descriptor_set );
	device_memory_pool.FreeCompleteResource( frame_data_device_buffer );
	device_memory_pool.FreeCompleteResource( frame_data_staging_buffer );

	for( auto f : vk_gpu_to_cpu_frame_fences ) {
		vkDestroyFence(
			vk_device,
			f,
			nullptr
		);
	}

	for( auto s : vk_submit_to_present_semaphores ) {
		vkDestroySemaphore(
			vk_device,
			s,
			nullptr
		);
	}

	vkDestroySemaphore(
		vk_device,
		vk_transfer_semaphore,
		nullptr
	);

	vkDestroyFence(
		vk_device,
		vk_acquire_image_fence,
		nullptr
	);

	vkDestroyCommandPool(
		vk_device,
		vk_command_pool,
		nullptr
	);

	for( auto f : vk_framebuffers ) {
		vkDestroyFramebuffer(
			vk_device,
			f,
			nullptr
		);
	}

	for( auto m : multisample_render_targets ) {
		device_memory_pool.FreeCompleteResource( m );
	}

	for( auto v : vk_swapchain_image_views ) {
		vkDestroyImageView(
			vk_device,
			v,
			nullptr
		);
	}

	vkDestroySwapchainKHR(
		vk_device,
		vk_swapchain,
		nullptr
	);

	vkDestroyRenderPass(
		vk_device,
		vk_render_pass,
		nullptr
	);

	vkDestroySurfaceKHR(
		vk_instance,
		vk_surface,
		nullptr
	);

	glfwDestroyWindow( glfw_window );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CloseWindow()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	should_close = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::ShouldClose()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return should_close;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::BeginRender()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	auto AcquireImage = [this](
		vk2d::vk2d_internal::WindowImpl		&	impl,
		VkPhysicalDevice						physical_device,
		VkDevice								device,
		vk2d::vk2d_internal::ResolvedQueue	&	primary_render_queue
		)
	{
		auto & instance = impl.instance;

		struct TryAcquireResult
		{
			uint32_t		new_image;
			VkResult		vk_result;
		};

		auto TryAcquireAndWaitAvailable = [&]() -> TryAcquireResult
		{
			uint32_t new_image_index = UINT32_MAX;
			auto acquire_result = vkAcquireNextImageKHR(
				device,
				impl.vk_swapchain,
				UINT64_MAX,
				VK_NULL_HANDLE,
				impl.vk_acquire_image_fence,
				&new_image_index
			);

			// Only wait for image availability on success or partial success.
			// This should cover all cases as VkResult positive values are
			// partial success and negative values are errors.
			if( acquire_result >= VK_SUCCESS ) {
				{
					auto sync_result = vkWaitForFences(
						device,
						1, &impl.vk_acquire_image_fence,
						VK_TRUE,
						UINT64_MAX
					);
					if( sync_result != VK_SUCCESS ) {
						instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot acquire next swapchain image, error waiting for fence!" );
						return { UINT32_MAX, acquire_result };
					}
				}
				{
					auto sync_result = vkResetFences(
						device,
						1, &impl.vk_acquire_image_fence
					);
					if( sync_result != VK_SUCCESS ) {
						instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot acquire next swapchain image, error resetting fence!" );
						return { UINT32_MAX, acquire_result };
					}
				}
			}
			return { new_image_index, acquire_result };
		};

		auto result = TryAcquireAndWaitAvailable();
		switch( result.vk_result ) {
		case VK_SUCCESS:
		{
			impl.next_image = result.new_image;
			return true;
		}

		case VK_SUBOPTIMAL_KHR:
		{
			// Image acquired but is not optimal, continue but recreate swapchain next time we begin the render again.
			impl.instance.Report( result.vk_result, "Acquired suboptimal swapchain image, continuing and recreating swapchain next frame." );

			impl.next_image = result.new_image;
			impl.should_reconstruct = true;
			return true;
		}

		case VK_ERROR_OUT_OF_DATE_KHR:
		{
			// Image was not acquired so we cannot present anything until we recreate the swapchain.
			impl.instance.Report( result.vk_result, "Could not acquire swapchain image, out of date swapchain, trying to recreate swapchain now." );

			if( !impl.RecreateWindowSizeDependantResources() ) {
				impl.instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot recreate window resources after resizing window!" );
				return false;
			}

			// Retry getting next swapchain image.
			auto retry_result = TryAcquireAndWaitAvailable();
			if( retry_result.vk_result == VK_SUCCESS ) {
				impl.instance.Report( retry_result.vk_result, "Successfully recreated swapchain and aquired swapchain image after recreating swapchain." );
				impl.next_image = retry_result.new_image;
				return true;
			} else {
				impl.instance.Report( ReportSeverity::CRITICAL_ERROR, "Error trying to get swapchain image after swapchain recreation. Aborting now!" );
				return false;
			}
		}
		default:
			assert( 0 && "Unhandled case." );
			break;
		}

		return false;
	};

	// Skip if the window is iconified, swapchain images might not be available.
	if( is_iconified ) return true;

	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != NextRenderCallFunction::BEGIN ) {
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "'Window::BeginRender()' Called twice in a row!" );
		return false;
	} else {
		next_render_call_function = NextRenderCallFunction::END;
	}

	if( should_reconstruct ) {
		if( !RecreateWindowSizeDependantResources() ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot recreate window resources after resizing window!" );
			return false;
		}
	}

	// Acquire a new image from the presentation engine. This
	// determines which "swap" we're going to write to.
	// Everything is double or multi-buffered, eg. command buffers,
	// framebuffers...
	{
		if( !AcquireImage(
			*this,
			vk_physical_device,
			vk_device,
			primary_render_queue
		) ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot acquire next swapchain image!" );
			return false;
		}
	}

	// If next image index happens to same as the previous, presentation has propably already succeeded but
	// since we're using the image index as an index to our command buffers and framebuffers we'll have to
	// make sure that we don't start overwriting a command buffer until it's execution has completely
	// finished, so we'll have to count for that and synchronize the frame early in here.
	if( next_image == previous_image ) {
		if( !SynchronizeFrame() ) {
			instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot synchronize frame, cannot output to window!" );
			return false;
		}
	}

	// Begin command buffer
	{
		VkCommandBuffer		command_buffer			= vk_render_command_buffers[ next_image ];

		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext				= nullptr;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo	= nullptr;

		if( vkBeginCommandBuffer(
			command_buffer,
			&command_buffer_begin_info
		) != VK_SUCCESS ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record primary render command buffer!" );
			return false;
		}
		CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"WindowImpl",
			CommandBufferCheckpointType::BEGIN_COMMAND_BUFFER
		);

		// Set viewport, scissor and initial line width
		{
			VkViewport viewport {};
			viewport.x			= 0;
			viewport.y			= 0;
			viewport.width		= float( extent.width );
			viewport.height		= float( extent.height );
			viewport.minDepth	= 0.0f;
			viewport.maxDepth	= 1.0f;
			vkCmdSetViewport(
				command_buffer,
				0, 1, &viewport
			);

			VkRect2D scissor {
				{ 0, 0 },
				extent
			};
			vkCmdSetScissor(
				command_buffer,
				0, 1, &scissor
			);

			vkCmdSetLineWidth(
				command_buffer,
				1.0f
			);
			previous_line_width		= 1.0f;

			// Window frame data.
			vkCmdBindDescriptorSets(
				command_buffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				instance.GetGraphicsPrimaryRenderPipelineLayout(),
				GRAPHICS_DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA,
				1, &frame_data_descriptor_set.descriptorSet,
				0, nullptr
			);
		}

		// Begin render pass
		{
			VkClearValue	clear_value {};
			clear_value.color.float32[ 0 ]		= 0.0f;
			clear_value.color.float32[ 1 ]		= 0.0f;
			clear_value.color.float32[ 2 ]		= 0.0f;
			clear_value.color.float32[ 3 ]		= 0.0f;

			VkRenderPassBeginInfo render_pass_begin_info {};
			render_pass_begin_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext			= nullptr;
			render_pass_begin_info.renderPass		= vk_render_pass;
			render_pass_begin_info.framebuffer		= vk_framebuffers[ next_image ];
			render_pass_begin_info.renderArea		= { { 0, 0 }, extent };
			render_pass_begin_info.clearValueCount	= 1;
			render_pass_begin_info.pClearValues		= &clear_value;

			CmdInsertCommandBufferCheckpoint(
				command_buffer,
				"WindowImpl",
				CommandBufferCheckpointType::BEGIN_RENDER_PASS
			);
			vkCmdBeginRenderPass(
				command_buffer,
				&render_pass_begin_info,
				VK_SUBPASS_CONTENTS_INLINE
			);
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::EndRender()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	// Skip if the window is iconified, swapchain images might not be available.
	if( is_iconified ) return true;

	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != NextRenderCallFunction::END ) {
		instance.Report( ReportSeverity::WARNING, "'Window::EndRender()' Called twice in a row!" );
		return false;
	} else {
		next_render_call_function = NextRenderCallFunction::BEGIN;
	}

	VkCommandBuffer		render_command_buffer	= vk_render_command_buffers[ next_image ];

	// End render pass
	{
		CmdInsertCommandBufferCheckpoint(
			render_command_buffer,
			"WindowImpl",
			CommandBufferCheckpointType::END_RENDER_PASS
		);
		vkCmdEndRenderPass( render_command_buffer );
	}

	// Save screenshot if it was requested
	{
		if( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::REQUESTED ) {

			VkImageSubresourceRange subresource_range {};
			subresource_range.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel		= 0;
			subresource_range.levelCount		= 1;
			subresource_range.baseArrayLayer	= 0;
			subresource_range.layerCount		= 1;
			{
				std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers {};

				// index 0: screenshot image
				// Prepare screenshot image memory barrier
				imageMemoryBarriers[ 0 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[ 0 ].pNext					= nullptr;
				imageMemoryBarriers[ 0 ].srcAccessMask			= 0;
				imageMemoryBarriers[ 0 ].dstAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarriers[ 0 ].oldLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarriers[ 0 ].newLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarriers[ 0 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 0 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 0 ].image					= screenshot_image.image;
				imageMemoryBarriers[ 0 ].subresourceRange		= subresource_range;

				// index 1: source image
				// Transfer from whatever layout the image is in to transfer source
				imageMemoryBarriers[ 1 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[ 1 ].pNext					= nullptr;
				imageMemoryBarriers[ 1 ].srcAccessMask			= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				imageMemoryBarriers[ 1 ].dstAccessMask			= VK_ACCESS_TRANSFER_READ_BIT;
				imageMemoryBarriers[ 1 ].oldLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				imageMemoryBarriers[ 1 ].newLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				imageMemoryBarriers[ 1 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 1 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 1 ].image					= vk_swapchain_images[ next_image ];
				imageMemoryBarriers[ 1 ].subresourceRange		= subresource_range;

				vkCmdPipelineBarrier( render_command_buffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					uint32_t( imageMemoryBarriers.size() ), imageMemoryBarriers.data() );
			}

			// Blitting transfers data, works outside render pass and can change formats so we'll use it to basically just copy the image.
			{
				VkImageBlit blitRegion {};
				blitRegion.srcSubresource.aspectMask		= VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
				blitRegion.srcSubresource.mipLevel			= 0;
				blitRegion.srcSubresource.baseArrayLayer	= 0;
				blitRegion.srcSubresource.layerCount		= 1;
				blitRegion.srcOffsets[ 0 ]					= { 0, 0, 0 };
				blitRegion.srcOffsets[ 1 ]					= { int32_t( extent.width ), int32_t( extent.height ), 1 };
				blitRegion.dstSubresource					= blitRegion.srcSubresource;
				blitRegion.dstOffsets[ 0 ]					= { 0, 0, 0 };
				blitRegion.dstOffsets[ 1 ]					= { int32_t( extent.width ), int32_t( extent.height ), 1 };
				vkCmdBlitImage( render_command_buffer,
					vk_swapchain_images[ next_image ],
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					screenshot_image.image,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blitRegion,
					VkFilter::VK_FILTER_LINEAR );
			}
			{
				std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers {};

				// index 0: screenshot image
				// Prepare screenshot image memory barrier
				imageMemoryBarriers[ 0 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[ 0 ].pNext					= nullptr;
				imageMemoryBarriers[ 0 ].srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarriers[ 0 ].dstAccessMask			= VK_ACCESS_TRANSFER_READ_BIT;
				imageMemoryBarriers[ 0 ].oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarriers[ 0 ].newLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				imageMemoryBarriers[ 0 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 0 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 0 ].image					= screenshot_image.image;
				imageMemoryBarriers[ 0 ].subresourceRange		= subresource_range;

				// index 1: source image
				// Transfer from whatever layout the image is in to transfer source
				imageMemoryBarriers[ 1 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[ 1 ].pNext					= nullptr;
				imageMemoryBarriers[ 1 ].srcAccessMask			= VK_ACCESS_TRANSFER_READ_BIT;
				imageMemoryBarriers[ 1 ].dstAccessMask			= VK_ACCESS_MEMORY_READ_BIT;
				imageMemoryBarriers[ 1 ].oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				imageMemoryBarriers[ 1 ].newLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				imageMemoryBarriers[ 1 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 1 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarriers[ 1 ].image					= vk_swapchain_images[ next_image ];
				imageMemoryBarriers[ 1 ].subresourceRange		= subresource_range;

				vkCmdPipelineBarrier( render_command_buffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					uint32_t( imageMemoryBarriers.size() ), imageMemoryBarriers.data() );
			}
			{
				// Copy to host visible buffer
				VkBufferImageCopy bufferImageCopyRegion {};
				bufferImageCopyRegion.bufferOffset						= 0;
				bufferImageCopyRegion.bufferRowLength					= 0;
				bufferImageCopyRegion.bufferImageHeight					= 0;
				bufferImageCopyRegion.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
				bufferImageCopyRegion.imageSubresource.mipLevel			= 0;
				bufferImageCopyRegion.imageSubresource.baseArrayLayer	= 0;
				bufferImageCopyRegion.imageSubresource.layerCount		= 1;
				bufferImageCopyRegion.imageOffset						= { 0, 0, 0 };
				bufferImageCopyRegion.imageExtent						= { extent.width, extent.height, 1 };
				vkCmdCopyImageToBuffer( render_command_buffer,
					screenshot_image.image,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					screenshot_buffer.buffer,
					1, &bufferImageCopyRegion );
			}
			{
				// Make sure writes to screenshot buffer have finished
				std::array<VkBufferMemoryBarrier, 1> bufferMemoryBarriers {};
				bufferMemoryBarriers[ 0 ].sType					= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				bufferMemoryBarriers[ 0 ].pNext					= nullptr;
				bufferMemoryBarriers[ 0 ].srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
				bufferMemoryBarriers[ 0 ].dstAccessMask			= VK_ACCESS_HOST_READ_BIT;
				bufferMemoryBarriers[ 0 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				bufferMemoryBarriers[ 0 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
				bufferMemoryBarriers[ 0 ].buffer				= screenshot_buffer.buffer;
				bufferMemoryBarriers[ 0 ].offset				= 0;
				bufferMemoryBarriers[ 0 ].size					= VK_WHOLE_SIZE;
				vkCmdPipelineBarrier( render_command_buffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_HOST_BIT,
					0,
					0, nullptr,
					uint32_t( bufferMemoryBarriers.size() ), bufferMemoryBarriers.data(),
					0, nullptr );
			}
			{
				screenshot_state			= vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_RENDER;
				screenshot_swapchain_id		= next_image;
			}
		}
	}
	// End command buffer
	CmdInsertCommandBufferCheckpoint(
		render_command_buffer,
		"WindowImpl",
		CommandBufferCheckpointType::END_COMMAND_BUFFER
	);

	auto result = vkEndCommandBuffer( render_command_buffer );
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot compile primary render command buffer!" );
		return false;
	}

	// SynchronizeFrame the previous frame here, this waits for the previous
	// frame to finish fully rendering before continuing execution.
	if( !SynchronizeFrame() ) {
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot synchronize frame, cannot output to window!" );
		return false;
	}
	// Record command buffer to upload complementary data to GPU
	{
		// Begin command buffer
		{
			VkCommandBufferBeginInfo transfer_command_buffer_begin_info {};
			transfer_command_buffer_begin_info.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			transfer_command_buffer_begin_info.pNext			= nullptr;
			transfer_command_buffer_begin_info.flags			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			transfer_command_buffer_begin_info.pInheritanceInfo	= nullptr;

			auto result = vkBeginCommandBuffer(
				vk_transfer_command_buffer,
				&transfer_command_buffer_begin_info
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot record mesh to GPU transfer command buffer!" );
				return false;
			}
		}

		// Record commands to upload frame data to gpu
		{
			if( !CmdUpdateFrameData(
				vk_transfer_command_buffer
			) ) {
				instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record commands to transfer FrameData to GPU!" );
				return false;
			}
		}

		// Record commands to upload mesh data to gpu
		{
			if( !mesh_buffer->CmdUploadMeshDataToGPU(
				vk_transfer_command_buffer
			) ) {
				instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record commands to transfer mesh data to GPU!" );
				return false;
			}
		}

		// End command buffer
		{
			auto result = vkEndCommandBuffer(
				vk_transfer_command_buffer
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot compile mesh to GPU transfer command buffer!" );
				return false;
			}
		}
	}

	// Submit renders
	{
		RenderTargetTextureRenderCollector collector;

		// Collect render target texture render submissions.
		if( !CommitRenderTargetTextureRender( collector ) ) {
			AbortRenderTargetTextureRender();
			instance.Report(
				ReportSeverity::NON_CRITICAL_ERROR,
				"Internal error: Cannot commit render target textures for rendering!"
			);
			return false;
		}

		std::vector<VkSubmitInfo> graphics_queue_submit_infos;
		graphics_queue_submit_infos.reserve( collector.size() * 2 + 2 );

		// TODO: During submission on the frame it could be more efficient to separate transfer and render further and just use a single semaphore between all transfers and renders.
		// Right now the transfer and render submissions are rather fragmented. Need to get a larger workload to measure impact on performance before modifying this.

		// Get all the submit infos from all render targets into one list.
		for( auto & c : collector ) {
			graphics_queue_submit_infos.push_back( *c.vk_transfer_submit_info );
			graphics_queue_submit_infos.push_back( *c.vk_render_submit_info );
		}

		// Collection of semaphores that the main window render needs to wait for, this considers timeline semaphores.
		std::vector<VkSemaphore>			render_wait_for_semaphores;
		std::vector<uint64_t>				render_wait_for_semaphore_timeline_values;
		std::vector<VkPipelineStageFlags>	render_wait_for_pipeline_stages;
		render_wait_for_semaphores.reserve( std::size( render_target_texture_dependencies[ next_image ] ) + 1 );
		render_wait_for_semaphore_timeline_values.reserve( std::size( render_target_texture_dependencies[ next_image ] ) + 1 );
		render_wait_for_pipeline_stages.reserve( std::size( render_target_texture_dependencies[ next_image ] ) + 1 );

		// First entry is the regular transfer semaphore which is a binary semaphore.
		render_wait_for_semaphores.push_back( vk_transfer_semaphore );
		render_wait_for_semaphore_timeline_values.push_back( 1 );
		render_wait_for_pipeline_stages.push_back( VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT );

		// Resolve immediate dependencies we need to wait for before the main render happens.
		for( auto & d : render_target_texture_dependencies[ next_image ] ) {
			render_wait_for_semaphores.push_back( d.render_target->GetAllCompleteSemaphore( d ) );
			render_wait_for_semaphore_timeline_values.push_back( d.render_target->GetRenderCounter( d ) );
			// TODO: Replace VK_PIPELINE_STAGE_ALL_COMMANDS_BIT with something that narrows down the potential pipeline bubble more.
			render_wait_for_pipeline_stages.push_back( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
		}

		assert( std::size( render_wait_for_semaphores ) == std::size( render_wait_for_semaphore_timeline_values ) &&
			std::size( render_wait_for_semaphores ) == std::size( render_wait_for_pipeline_stages ) );

	// Get submit infos for the window itself.
		VkSubmitInfo window_transfer_submit_info {};
		window_transfer_submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		window_transfer_submit_info.pNext					= nullptr;
		window_transfer_submit_info.waitSemaphoreCount		= 0;
		window_transfer_submit_info.pWaitSemaphores			= nullptr;
		window_transfer_submit_info.pWaitDstStageMask		= nullptr;
		window_transfer_submit_info.commandBufferCount		= 1;
		window_transfer_submit_info.pCommandBuffers			= &vk_transfer_command_buffer;
		window_transfer_submit_info.signalSemaphoreCount	= 1;
		window_transfer_submit_info.pSignalSemaphores		= &vk_transfer_semaphore;
		graphics_queue_submit_infos.push_back( window_transfer_submit_info );

		uint64_t signal_timeline_semaphore_value = 1;
		VkTimelineSemaphoreSubmitInfo window_render_timeline_submit_info {};
		window_render_timeline_submit_info.sType						= VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		window_render_timeline_submit_info.pNext						= nullptr;
		window_render_timeline_submit_info.waitSemaphoreValueCount		= uint32_t( std::size( render_wait_for_semaphore_timeline_values ) );
		window_render_timeline_submit_info.pWaitSemaphoreValues			= render_wait_for_semaphore_timeline_values.data();
		window_render_timeline_submit_info.signalSemaphoreValueCount	= 1;
		window_render_timeline_submit_info.pSignalSemaphoreValues		= &signal_timeline_semaphore_value;

		VkSubmitInfo window_render_submit_info {};
		window_render_submit_info.sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		window_render_submit_info.pNext						= &window_render_timeline_submit_info;
		window_render_submit_info.waitSemaphoreCount		= uint32_t( std::size( render_wait_for_semaphores ) );
		window_render_submit_info.pWaitSemaphores			= render_wait_for_semaphores.data();
		window_render_submit_info.pWaitDstStageMask			= render_wait_for_pipeline_stages.data();
		window_render_submit_info.commandBufferCount		= 1;
		window_render_submit_info.pCommandBuffers			= &render_command_buffer;
		window_render_submit_info.signalSemaphoreCount		= 1;
		window_render_submit_info.pSignalSemaphores			= &vk_submit_to_present_semaphores[ next_image ];
		graphics_queue_submit_infos.push_back( window_render_submit_info );

		auto result = primary_render_queue.Submit(
			graphics_queue_submit_infos,
			vk_gpu_to_cpu_frame_fences[ next_image ]
		);
		if( result != VK_SUCCESS ) {
			AbortRenderTargetTextureRender();
			instance.Report(
				result,
				"Internal error: Cannot submit frame end pre compute graphics command buffers!"
			);
			return false;
		}

		// Notify render targets about successful command buffer submission.
		ConfirmRenderTargetTextureRenderSubmission();
	}

	// Present swapchain image
	{
		VkResult present_result				= VK_SUCCESS;
		VkPresentInfoKHR present_info {};
		present_info.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext					= nullptr;
		present_info.waitSemaphoreCount		= 1;
		present_info.pWaitSemaphores		= &vk_submit_to_present_semaphores[ next_image ];
		present_info.swapchainCount			= 1;
		present_info.pSwapchains			= &vk_swapchain;
		present_info.pImageIndices			= &next_image;
		present_info.pResults				= &present_result;
		auto result = primary_render_queue.Present(
			present_info
		);
		if( result != VK_SUCCESS || present_result != VK_SUCCESS ) {
			if( result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_ERROR_OUT_OF_DATE_KHR ||
				result == VK_SUBOPTIMAL_KHR || present_result == VK_SUBOPTIMAL_KHR ) {
				should_reconstruct	= true;
			} else {
				instance.Report( result, "Internal error: Cannot present render results to window!" );
				return false;
			}
		}
	}

	//ClearRenderTargetTextureDepencies();

	previous_image						= next_image;
	previous_frame_need_synchronization	= true;
	previous_pipeline_settings			= {};
	previous_sampler					= {};
	previous_texture					= {};
	previous_line_width					= {};

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::TakeScreenshotToFile(
	const std::filesystem::path		&	save_path,
	bool								include_alpha
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::IDLE ) {
		screenshot_save_path	= save_path;
		screenshot_state		= vk2d::vk2d_internal::WindowImpl::ScreenshotState::REQUESTED;
		screenshot_alpha		= include_alpha;
	} else {
		instance.Report( ReportSeverity::INFO, "Screenshot request ignored: Previous screenshot has not yet been processed." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::TakeScreenshotToData(
	bool		include_alpha
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::IDLE ) {
		screenshot_save_path	= "";
		screenshot_state		= vk2d::vk2d_internal::WindowImpl::ScreenshotState::REQUESTED;
		screenshot_alpha		= include_alpha;
	} else {
		instance.Report( ReportSeverity::INFO, "Screenshot request ignored: Previous screenshot has not yet been processed." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::Focus()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	glfwFocusWindow( glfw_window );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetOpacity( float opacity )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	glfwSetWindowOpacity( glfw_window, opacity );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float vk2d::vk2d_internal::WindowImpl::GetOpacity()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return glfwGetWindowOpacity( glfw_window );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::Hide( bool hidden )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( hidden ) {
		glfwHideWindow( glfw_window );
	} else {
		glfwShowWindow( glfw_window );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::IsHidden()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return !glfwGetWindowAttrib( glfw_window, GLFW_VISIBLE );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::DisableEvents(
	bool		disable_events
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( disable_events ) {
		event_handler	= nullptr;
	} else {
		event_handler	= create_info_copy.event_handler;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::AreEventsDisabled()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return !!event_handler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetFullscreen(
	Monitor		*	monitor,
	uint32_t				frequency )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( monitor ) {
		glfwSetWindowMonitor( glfw_window, monitor->impl->monitor, 0, 0, extent.width, extent.height, frequency );
		if( !glfwGetWindowMonitor( glfw_window ) ) {
			glfwSetWindowMonitor( glfw_window, nullptr, position.x, position.y, extent.width, extent.height, INT32_MAX );
		}
	} else {
		glfwSetWindowMonitor( glfw_window, nullptr, position.x, position.y, extent.width, extent.height, INT32_MAX );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::IsFullscreen()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return !!glfwGetWindowMonitor( glfw_window );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::dvec2 vk2d::vk2d_internal::WindowImpl::GetCursorPosition()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	double x = 0, y = 0;
	glfwGetCursorPos( glfw_window, &x, &y );
	return { x, y };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetCursorPosition(
	glm::dvec2		new_position
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	glfwSetCursorPos( glfw_window, new_position.x, new_position.y );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetCursor( Cursor * cursor )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( cursor ) {
		glfwSetCursor( glfw_window, cursor->impl->cursor );
	} else {
		glfwSetCursor( glfw_window, nullptr );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vk2d_internal::WindowImpl::GetClipboardString()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return glfwGetClipboardString( glfw_window );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetClipboardString( const std::string & str )
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	glfwSetClipboardString( glfw_window, str.c_str() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetTitle(
	const std::string		&	title
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	window_title	= title;
	glfwSetWindowTitle( glfw_window, window_title.c_str() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string vk2d::vk2d_internal::WindowImpl::GetTitle()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return window_title;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetIcon(
	const std::vector<std::filesystem::path>	&	image_paths
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	icon_data.resize( image_paths.size() );
	for( size_t i = 0; i < image_paths.size(); ++i ) {
		auto & ic = icon_data[ i ];

		int x = 0, y = 0, channels = 0;
		auto stbi_image = stbi_load( image_paths[ i ].string().c_str(), &x, &y, &channels, 4 );

		if( stbi_image && channels == 4 ) {
			ic.image_data.resize( x * y * 4 );
			std::memcpy( ic.image_data.data(), stbi_image, x * y * 4 );
			ic.glfw_image.width		= x;
			ic.glfw_image.height	= y;
			stbi_image_free( stbi_image );
		} else {
			ic.image_data.resize( 4 );
			std::memset( ic.image_data.data(), 255, 4 );
			ic.glfw_image.width		= 1;
			ic.glfw_image.height	= 1;
		}

		ic.glfw_image.pixels	= ic.image_data.data();
	}

	std::vector<GLFWimage> images( icon_data.size() );
	for( size_t i = 0; i < icon_data.size(); ++i ) {
		images[ i ]		= icon_data[ i ].glfw_image;
	}
	glfwSetWindowIcon( glfw_window, int( icon_data.size() ), images.data() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetPosition(
	glm::ivec2		new_position
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	glfwSetWindowPos( glfw_window, new_position.x, new_position.y );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::ivec2 vk2d::vk2d_internal::WindowImpl::GetPosition()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	int x = 0, y = 0;
	glfwGetWindowPos( glfw_window, &x, &y );
	return { int32_t( x ), int32_t( y ) };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetSize(
	glm::uvec2			new_size
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	extent					= { uint32_t( new_size.x ), uint32_t( new_size.y ) };
	should_reconstruct		= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
glm::uvec2 vk2d::vk2d_internal::WindowImpl::GetSize()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return { extent.width, extent.height };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::Iconify(
	bool			iconified
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( iconified ) {
		glfwIconifyWindow( glfw_window );
	} else {
		glfwRestoreWindow( glfw_window );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::IsIconified()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return glfwGetWindowAttrib( glfw_window, GLFW_ICONIFIED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetMaximized(
	bool			maximized
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	if( maximized ) {
		glfwMaximizeWindow( glfw_window );
	} else {
		glfwRestoreWindow( glfw_window );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::GetMaximized()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	return glfwGetWindowAttrib( glfw_window, GLFW_MAXIMIZED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetCursorState(
	CursorState		new_state
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	switch( new_state ) {
		case CursorState::NORMAL:
			glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			break;
		case CursorState::HIDDEN:
			glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
			break;
		case CursorState::LOCKED:
			glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
			break;
		default:
			instance.Report(
				ReportSeverity::WARNING,
				"Parameter error: 'Window::SetCursorState()' -> parameter: 'new_state' was not any value in\n"
				"'CursorState', defaulting to 'CursorState::NORMAL'"
			);
			glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::CursorState vk2d::vk2d_internal::WindowImpl::GetCursorState()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	auto state = glfwGetInputMode( glfw_window, GLFW_CURSOR );
	switch( state ) {
		case GLFW_CURSOR_NORMAL:
			return CursorState::NORMAL;
		case GLFW_CURSOR_HIDDEN:
			return CursorState::HIDDEN;
		case GLFW_CURSOR_DISABLED:
			return CursorState::LOCKED;
		default:
			assert( 0 && "Should not happen" );
			return CursorState::NORMAL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::SetRenderCoordinateSpace(
	RenderCoordinateSpace coordinate_space
)
{
	this->coordinate_space = coordinate_space;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::DrawPointList(
	const RawVertexData					&	raw_vertex_data,
	std::span<const glm::mat4>				transformations,
	Texture								*	texture,
	Sampler								*	sampler
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	// Skip if the window is iconified, swapchain images might not be available.
	if( is_iconified ) return;

	auto command_buffer = vk_render_command_buffers[ next_image ];

	auto vertex_count = uint32_t( raw_vertex_data.vertex_count );

	if( !texture ) {
		texture = instance.GetDefaultTexture();
	}
	if( !texture->IsTextureDataReady() ) {
		texture = instance.GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance.GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency( texture );

	{
		auto graphics_shader_programs = instance.GetCompatibleGraphicsShaderModules(
			sampler->impl->IsAnyBorderColorEnabled(),
			1
		);

		GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout = instance.GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass = vk_render_pass;
		pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		pipeline_settings.polygon_mode = VK_POLYGON_MODE_POINT;
		pipeline_settings.shader_programs = graphics_shader_programs;
		pipeline_settings.samples = VkSampleCountFlags( samples );
		pipeline_settings.enable_blending = VK_TRUE;

		CmdBindGraphicsPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		{},
		raw_vertex_data,
		transformations
	);

	if( push_result.success )
	{
		auto aligned_buffer_offsets = AlignedMeshBufferOffsets( push_result, raw_vertex_data );

		{
			GraphicsPrimaryRenderPushConstants pc {};
			pc.transformation_offset = aligned_buffer_offsets.transformation_offset;
			pc.index_offset = aligned_buffer_offsets.index_offset;
			pc.index_count = 1;
			pc.vertex_offset = aligned_buffer_offsets.vertex_offset;

			vkCmdPushConstants(
				command_buffer,
				instance.GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			CommandBufferCheckpointType::DRAW
		);
		vkCmdDraw(
			command_buffer,
			vertex_count,
			uint32_t( std::size( transformations ) ),
			aligned_buffer_offsets.vertex_offset,
			0
		);
	}
	else {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::DrawLineList(
	std::span<const uint32_t>				raw_indices,
	const RawVertexData					&	raw_vertex_data,
	std::span<const glm::mat4>				transformations,
	Texture								*	texture,
	Sampler								*	sampler,
	float									line_width
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	// Skip if the window is iconified, swapchain images might not be available.
	if( is_iconified ) return;

	auto command_buffer					= vk_render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( raw_vertex_data.vertex_count );
	auto index_count	= uint32_t( raw_indices.size() );

	if( !texture ) {
		texture = instance.GetDefaultTexture();
	}
	if( !texture->IsTextureDataReady() ) {
		texture = instance.GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance.GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency( texture );

	{
		auto graphics_shader_programs = instance.GetCompatibleGraphicsShaderModules(
			sampler->impl->IsAnyBorderColorEnabled(),
			2
		);

		GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout	= instance.GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass		= vk_render_pass;
		pipeline_settings.primitive_topology	= VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		pipeline_settings.polygon_mode			= VK_POLYGON_MODE_LINE;
		pipeline_settings.shader_programs		= graphics_shader_programs;
		pipeline_settings.samples				= VkSampleCountFlags( samples );
		pipeline_settings.enable_blending		= VK_TRUE;

		CmdBindGraphicsPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdSetLineWidthIfDifferent(
		command_buffer,
		line_width
	);
	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		raw_vertex_data,
		transformations
	);

	if( push_result.success )
	{
		auto aligned_buffer_offsets = AlignedMeshBufferOffsets( push_result, raw_vertex_data );

		{
			GraphicsPrimaryRenderPushConstants pc {};
			pc.transformation_offset			= aligned_buffer_offsets.transformation_offset;
			pc.index_offset						= aligned_buffer_offsets.index_offset;
			pc.index_count						= 2;
			pc.vertex_offset					= aligned_buffer_offsets.vertex_offset;

			vkCmdPushConstants(
				command_buffer,
				instance.GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			CommandBufferCheckpointType::DRAW
		);
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			uint32_t( std::size( transformations ) ),
			aligned_buffer_offsets.index_offset,
			int32_t( aligned_buffer_offsets.vertex_offset ),
			0
		);
	} else {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::DrawTriangleList(
	std::span<const uint32_t>				raw_indices,
	const RawVertexData					&	raw_vertex_data,
	std::span<const glm::mat4>				transformations,
	bool									filled,
	Texture								*	texture,
	Sampler								*	sampler
)
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	// Skip if the window is iconified, swapchain images might not be available.
	if( is_iconified ) return;

	auto command_buffer = vk_render_command_buffers[ next_image ];

	auto vertex_count = uint32_t( raw_vertex_data.vertex_count );
	auto index_count = uint32_t( raw_indices.size() );

	if( !texture ) {
		texture = instance.GetDefaultTexture();
	}
	if( !texture->IsTextureDataReady() ) {
		texture = instance.GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance.GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency( texture );

	{
		auto graphics_shader_programs = instance.GetCompatibleGraphicsShaderModules(
			sampler->impl->IsAnyBorderColorEnabled(),
			3
		);

		GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout = instance.GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass = vk_render_pass;
		pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline_settings.polygon_mode = filled ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
		pipeline_settings.shader_programs = graphics_shader_programs;
		pipeline_settings.samples = VkSampleCountFlags( samples );
		pipeline_settings.enable_blending = VK_TRUE;

		CmdBindGraphicsPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		raw_vertex_data,
		transformations
	);

	if( push_result.success )
	{
		auto aligned_buffer_offsets = AlignedMeshBufferOffsets( push_result, raw_vertex_data );
		{
			GraphicsPrimaryRenderPushConstants pc {};
			pc.transformation_offset = aligned_buffer_offsets.transformation_offset;
			pc.index_offset = aligned_buffer_offsets.index_offset;
			pc.index_count = 3;
			pc.vertex_offset = aligned_buffer_offsets.vertex_offset;

			vkCmdPushConstants(
				command_buffer,
				instance.GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			CommandBufferCheckpointType::DRAW
		);
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			uint32_t( std::size( transformations ) ),
			aligned_buffer_offsets.index_offset,
			int32_t( aligned_buffer_offsets.vertex_offset ),
			0
		);
	}
	else
	{
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}

	#if VK2D_BUILD_OPTION_DEBUG_ALWAYS_DRAW_TRIANGLES_WIREFRAME
	if( filled ) {
		auto vertices_copy = vertices;
		for( auto & v : vertices_copy ) {
			v.color = Colorf( 0.2f, 1.0f, 0.4f, 0.25f );
		}
		DrawTriangleList(
			raw_indices,
			vertices_copy,
			{},
			false
		);
	}
	#endif
}







namespace vk2d {
namespace vk2d_internal {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ScreenshotSaverTask : public Task
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ScreenshotSaverTask(
		WindowImpl		*	window
	) :
		window( window )
	{}

	void											operator()(
		ThreadPrivateResource	*	thread_resource )
	{
		assert( window->screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE );

		auto path				= window->screenshot_save_path;
		auto extent				= window->screenshot_save_data.size;
		auto pixel_count		= VkDeviceSize( extent.x ) * VkDeviceSize( extent.y );

		auto screenshot_data	= reinterpret_cast<uint8_t*>( window->screenshot_save_data.data.data() );
		int pixel_channels		= 4;

		// Prepare for if we don't want to save the alpha channel.
		std::vector<uint8_t> pixel_rgb_data;
		if( !window->screenshot_alpha ) {
			pixel_rgb_data.resize( pixel_count * 3 );
			for( VkDeviceSize i = 0; i < pixel_count; ++i ) {
//				auto pixel_rgba_offset = 4 * i;
				auto pixel_rgb_offset = 3 * i;
				pixel_rgb_data[ pixel_rgb_offset + 0 ] = window->screenshot_save_data.data[ i ].r;
				pixel_rgb_data[ pixel_rgb_offset + 1 ] = window->screenshot_save_data.data[ i ].g;
				pixel_rgb_data[ pixel_rgb_offset + 2 ] = window->screenshot_save_data.data[ i ].b;
			}
			screenshot_data = pixel_rgb_data.data();
			pixel_channels = 3;
		}

		int stbi_write_success = 0;
		auto extension = path.extension();
		if( extension == ".png" ) {
			stbi_write_success = stbi_write_png(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data,
				0
			);
		} else if( extension == ".bmp" ) {
			stbi_write_success = stbi_write_bmp(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data
			);
		} else if( extension == ".tga" ) {
			stbi_write_success = stbi_write_tga(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data
			);
		} else if( extension == ".jpg" ) {
			stbi_write_success = stbi_write_jpg(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data,
				90
			);
		} else if( extension == ".jpeg" ) {
			stbi_write_success = stbi_write_jpg(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data,
				90
			);
		} else {
			window->instance.Report( ReportSeverity::INFO, "Screenshot extension was not known, saving screenshot as .png" );
			path += ".png";
			stbi_write_success = stbi_write_png(
				path.string().c_str(),
				int( extent.x ),
				int( extent.y ),
				pixel_channels,
				screenshot_data,
				0
			);
		}

		if( stbi_write_success ) {
			window->screenshot_event_error = false;
			window->screenshot_event_message = std::string( "Screenshot successfully saved at: " ) + path.string();
		} else {
			window->instance.Report( ReportSeverity::WARNING, std::string( "Cannot save screenshot: '" ) + path.string() + "'" );
			window->screenshot_event_error = true;
			window->screenshot_event_message = std::string( "Cannot save screenshot '" ) + path.string() + "'";
		}

		window->screenshot_state					= vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT;
	}

	WindowImpl			*	window			= {};
	std::filesystem::path					path			= {};
};

} // vk2d_internal
} // vk2d



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::SynchronizeFrame()
{
	VK2D_ASSERT_MAIN_THREAD( instance );

	auto result = VK_SUCCESS;

	if( previous_frame_need_synchronization ) {

		using namespace std::chrono_literals;

		result = vkWaitForFences(
			vk_device,
			1, &vk_gpu_to_cpu_frame_fences[ previous_image ],
			VK_TRUE,
			std::chrono::duration_cast<std::chrono::nanoseconds>( 5s ).count()
		);
		if( result == VK_TIMEOUT ) {
			instance.Report( result, "Internal error: Timeout synchronizing frame." );
			return false;
		}
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot properly synchronize frame." );
			return false;
		}

		ConfirmRenderTargetTextureRenderFinished( previous_image );

		result = vkResetFences(
			vk_device,
			1, &vk_gpu_to_cpu_frame_fences[ previous_image ]
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot properly synchronize frame." );
			return false;
		}

		{
			if( screenshot_state			== vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_RENDER &&
				screenshot_swapchain_id		== previous_image ) {
				// Can get the screenshot data now.
				{
					screenshot_save_data.size = { extent.width, extent.height };
					screenshot_save_data.data.resize( extent.width * extent.height );
					auto mapped_data = screenshot_buffer.memory.Map<Color8>();
					if( mapped_data ) {
						std::memcpy( screenshot_save_data.data.data(), mapped_data, screenshot_save_data.data.size() * sizeof( Color8 ) );
						screenshot_buffer.memory.Unmap();

						if( screenshot_save_path.empty() ) {
							screenshot_state		= vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT;
						} else {
							screenshot_state		= vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE;
							instance.GetThreadPool()->ScheduleTask(
								std::make_unique<ScreenshotSaverTask>( this ),
								instance.GetGeneralThreads() );
						}
					} else {
						instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot save screenshot, cannot map screenshot buffer memory!" );
						screenshot_event_error		= true;
						screenshot_event_message	= "Internal error: Cannot map buffer data.";
						screenshot_state			= vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT;
					}
				}
			}

			if( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT ) {
				HandleScreenshotEvent();
			}
		}

		// And we also don't need to synchronize later.
		previous_frame_need_synchronization	= false;
	}

	return true;
}

bool vk2d::vk2d_internal::WindowImpl::IsGood()
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::RecreateWindowSizeDependantResources()
{
	instance.Report( ReportSeverity::VERBOSE, "Begin recreating window resources." );

	if( !ReCreateSwapchain() ) return false;

	// Doing this here because it's dependant of the swapchain image count.
	render_target_texture_dependencies.resize( swapchain_image_count );

	ReCreateScreenshotResources();

	// Reallocate framebuffers
	{
		if( vk_framebuffers.size() ) {
			for( auto m : multisample_render_targets ) {
				device_memory_pool.FreeCompleteResource( m );
			}
			for( auto fb : vk_framebuffers ) {
				vkDestroyFramebuffer(
					vk_device,
					fb,
					nullptr
				);
			}
		}
		if( !CreateFramebuffers() ) return false;
	}

	// Reallocate command buffers
	if( vk_render_command_buffers.size() != swapchain_image_count ) {
		if( vk_render_command_buffers.size() ) {
			vkFreeCommandBuffers(
				vk_device,
				vk_command_pool,
				uint32_t( vk_render_command_buffers.size() ),
				vk_render_command_buffers.data()
			);
		}
		if( !AllocateCommandBuffers() ) return false;
	}

	if( vk_submit_to_present_semaphores.size() != swapchain_image_count ||
		vk_gpu_to_cpu_frame_fences.size() != swapchain_image_count ) {

		// Recreate synchronization semaphores
		if( vk_submit_to_present_semaphores.size() ) {
			for( auto s : vk_submit_to_present_semaphores ) {
				vkDestroySemaphore(
					vk_device,
					s,
					nullptr
				);
			}
		}

		// Recreate synchronization fences
		if( vk_gpu_to_cpu_frame_fences.size() ) {
			vkResetFences( vk_device, uint32_t( vk_gpu_to_cpu_frame_fences.size() ), vk_gpu_to_cpu_frame_fences.data() );
			for( auto s : vk_gpu_to_cpu_frame_fences ) {
				vkDestroyFence(
					vk_device,
					s,
					nullptr
				);
			}
		}

		if( !CreateFrameSynchronizationPrimitives() ) return false;
	}

	should_reconstruct		= false;

	instance.Report( ReportSeverity::VERBOSE, "Done recreating window resources." );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateGLFWWindow()
{
	glfwWindowHint( GLFW_RESIZABLE, create_info_copy.resizeable );
	glfwWindowHint( GLFW_VISIBLE, create_info_copy.visible );
	glfwWindowHint( GLFW_DECORATED, create_info_copy.decorated );
	glfwWindowHint( GLFW_FOCUSED, create_info_copy.focused );
	glfwWindowHint( GLFW_AUTO_ICONIFY, GLFW_FALSE );
	glfwWindowHint( GLFW_MAXIMIZED, create_info_copy.maximized );
	glfwWindowHint( GLFW_CENTER_CURSOR, GLFW_TRUE );
	glfwWindowHint( GLFW_TRANSPARENT_FRAMEBUFFER, create_info_copy.transparent_framebuffer );
	glfwWindowHint( GLFW_FOCUS_ON_SHOW, GLFW_TRUE );
	glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );

	GLFWmonitor * monitor = nullptr;
	if( create_info_copy.fullscreen_monitor ) {
		if( create_info_copy.fullscreen_monitor->impl ) {
			monitor = create_info_copy.fullscreen_monitor->impl->monitor;
		}
	}

	glfw_window = glfwCreateWindow(
		int( create_info_copy.size.x ),
		int( create_info_copy.size.y ),
		window_title.c_str(),
		monitor,
		nullptr );
	if( !glfw_window ) {
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create glfw window!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateSurface()
{
	auto result = VK_SUCCESS;

	{
		result = glfwCreateWindowSurface(
			vk_instance,
			glfw_window,
			nullptr,
			&vk_surface
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create Vulkan surface!" );
			return false;
		}

		VkBool32 surface_supported = VK_FALSE;
		result = vkGetPhysicalDeviceSurfaceSupportKHR(
			vk_physical_device,
			primary_render_queue.GetQueueFamilyIndex(),
			vk_surface,
			&surface_supported
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot get physical device surface support!" );
			return false;
		}
		if( !surface_supported ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Vulkan surface does not support presentation using primary render queue, cannot continue!" );
			return false;
		}
	}

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		vk_physical_device,
		vk_surface,
		&surface_capabilities
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Cannot get physical device surface capabilities, cannot continue!" );
		return false;
	}

	// Check if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is supported
	{
		VkImageUsageFlags required_image_support = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if( !( ( surface_capabilities.supportedUsageFlags & required_image_support ) == required_image_support ) ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Window Vulkan surface does not support required image capabilities, cannot continue!" );
			return false;
		}
	}

	// Figure out surface format
	{
		std::vector<VkSurfaceFormatKHR> surface_formats;
		{
			uint32_t surface_format_count = 0;
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(
				vk_physical_device,
				vk_surface,
				&surface_format_count,
				nullptr
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot query physical device surface formats, cannot continue!" );
				return false;
			}
			surface_formats.resize( surface_format_count );

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(
				vk_physical_device,
				vk_surface,
				&surface_format_count,
				surface_formats.data()
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot query physical device surface formats, cannot continue!" );
				return false;
			}
		}
		surface_format = surface_formats[ 0 ];
		if( surface_format.format == VK_FORMAT_UNDEFINED ) {
			surface_format.format			= VK_FORMAT_R8G8B8A8_UNORM;
			surface_format.colorSpace		= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateRenderPass()
{
	bool use_multisampling	= samples != Multisamples::SAMPLE_COUNT_1;

	std::vector<VkAttachmentDescription>					color_attachment_descriptions {};
	if( use_multisampling ) {
		color_attachment_descriptions.resize( 2 );
	} else {
		color_attachment_descriptions.resize( 1 );
	}

	color_attachment_descriptions[ 0 ].flags				= 0;
	color_attachment_descriptions[ 0 ].format				= surface_format.format;
	color_attachment_descriptions[ 0 ].samples				= VkSampleCountFlagBits( samples );
	color_attachment_descriptions[ 0 ].loadOp				= VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_descriptions[ 0 ].storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_descriptions[ 0 ].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_descriptions[ 0 ].finalLayout			= use_multisampling ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	if( use_multisampling ) {
		color_attachment_descriptions[ 1 ].flags			= 0;
		color_attachment_descriptions[ 1 ].format			= surface_format.format;
		color_attachment_descriptions[ 1 ].samples			= VK_SAMPLE_COUNT_1_BIT;
		color_attachment_descriptions[ 1 ].loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_descriptions[ 1 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_descriptions[ 1 ].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_descriptions[ 1 ].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_descriptions[ 1 ].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_descriptions[ 1 ].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	std::array<VkAttachmentReference, 0>					input_attachment_references {};

	std::array<VkAttachmentReference, 1>					color_attachment_references {};
	color_attachment_references[ 0 ].attachment				= 0;	// points to color_attachment_descriptions[ 0 ]
	color_attachment_references[ 0 ].layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 1>					resolve_attachment_references {};
	resolve_attachment_references[ 0 ].attachment			= 1;	// points to color_attachment_descriptions[ 1 ]
	resolve_attachment_references[ 0 ].layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference						depth_stencil_attachment {};
	depth_stencil_attachment.attachment			= VK_ATTACHMENT_UNUSED;
	depth_stencil_attachment.layout				= VK_IMAGE_LAYOUT_UNDEFINED;

	std::array<uint32_t, 0>						preserve_attachments {};

	std::array<VkSubpassDescription, 1>			subpasses {};
	subpasses[ 0 ].flags						= 0;
	subpasses[ 0 ].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[ 0 ].inputAttachmentCount			= uint32_t( input_attachment_references.size() );
	subpasses[ 0 ].pInputAttachments			= input_attachment_references.data();
	subpasses[ 0 ].colorAttachmentCount			= uint32_t( color_attachment_references.size() );
	subpasses[ 0 ].pColorAttachments			= color_attachment_references.data();
	subpasses[ 0 ].pResolveAttachments			= use_multisampling ? resolve_attachment_references.data() : nullptr;
	subpasses[ 0 ].pDepthStencilAttachment		= &depth_stencil_attachment;
	subpasses[ 0 ].preserveAttachmentCount		= uint32_t( preserve_attachments.size() );
	subpasses[ 0 ].pPreserveAttachments			= preserve_attachments.data();

	// OPTIMIZATION: Look here to see if we need to narrow the scope of synchronization to possibly gain performance
	std::array<VkSubpassDependency, 2>			subpass_dependencies {};
	subpass_dependencies[ 0 ].srcSubpass		= VK_SUBPASS_EXTERNAL;
	subpass_dependencies[ 0 ].dstSubpass		= 0;
	subpass_dependencies[ 0 ].srcStageMask		= VK_PIPELINE_STAGE_HOST_BIT;
	subpass_dependencies[ 0 ].dstStageMask		= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	subpass_dependencies[ 0 ].srcAccessMask		= 0;
	subpass_dependencies[ 0 ].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	subpass_dependencies[ 0 ].dependencyFlags	= 0;

	subpass_dependencies[ 1 ].srcSubpass		= 0;
	subpass_dependencies[ 1 ].dstSubpass		= VK_SUBPASS_EXTERNAL;
	subpass_dependencies[ 1 ].srcStageMask		= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	subpass_dependencies[ 1 ].dstStageMask		= VK_PIPELINE_STAGE_HOST_BIT;
	subpass_dependencies[ 1 ].srcAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	subpass_dependencies[ 1 ].dstAccessMask		= 0;
	subpass_dependencies[ 1 ].dependencyFlags	= 0;

	VkRenderPassCreateInfo render_pass_create_info {};
	render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.pNext				= nullptr;
	render_pass_create_info.flags				= 0;
	render_pass_create_info.attachmentCount		= uint32_t( color_attachment_descriptions.size() );
	render_pass_create_info.pAttachments		= color_attachment_descriptions.data();
	render_pass_create_info.subpassCount		= uint32_t( subpasses.size() );
	render_pass_create_info.pSubpasses			= subpasses.data();
	render_pass_create_info.dependencyCount		= uint32_t( subpass_dependencies.size() );
	render_pass_create_info.pDependencies		= subpass_dependencies.data();

	auto result = vkCreateRenderPass(
		vk_device,
		&render_pass_create_info,
		nullptr,
		&vk_render_pass
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan render pass!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateCommandPool()
{

	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext				= nullptr;
	command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex	= primary_render_queue.GetQueueFamilyIndex();

	auto result = vkCreateCommandPool(
		vk_device,
		&command_pool_create_info,
		nullptr,
		&vk_command_pool
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create window Vulkan command pool!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::AllocateCommandBuffers()
{
	vk_render_command_buffers.resize( swapchain_image_count );
	std::vector<VkCommandBuffer> temp( swapchain_image_count + 1 );

	VkCommandBufferAllocateInfo command_buffer_allocate_info {};
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext				= nullptr;
	command_buffer_allocate_info.commandPool		= vk_command_pool;
	command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount	= uint32_t( temp.size() );

	auto result = vkAllocateCommandBuffers(
		vk_device,
		&command_buffer_allocate_info,
		temp.data()
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot allocate window Vulkan command buffers!" );
		return false;
	}

	for( size_t i = 0; i < swapchain_image_count; ++i ) {
		vk_render_command_buffers[ i ]		= temp[ i ];
	}
	vk_transfer_command_buffer		= temp[ swapchain_image_count ];

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::ReCreateSwapchain()
{
	auto result = VK_SUCCESS;

	if( !SynchronizeFrame() ) return false;
	vkQueueWaitIdle( primary_render_queue.GetQueue() );

	auto old_vk_swapchain		= vk_swapchain;

	// Create swapchain
	{
		// Figure out minimum image count
		uint32_t swapchain_minimum_image_count = 0;
		{
			if( create_info_copy.vsync ) {
				swapchain_minimum_image_count = 2;	// Vsync enabled, we only need 2 swapchain images
			} else {
				swapchain_minimum_image_count = 3;	// Vsync disabled, we should use at least 3 images
			}
			if( surface_capabilities.maxImageCount != 0 ) {
				if( swapchain_minimum_image_count > surface_capabilities.maxImageCount ) {
					swapchain_minimum_image_count = surface_capabilities.maxImageCount;
				}
			}
			if( swapchain_minimum_image_count < surface_capabilities.minImageCount ) {
				swapchain_minimum_image_count = surface_capabilities.minImageCount;
			}
		}
		assert( swapchain_minimum_image_count > 0 );

		// Figure out image dimensions and set window minimum and maximum sizes
		{
			min_extent		= {
				create_info_copy.min_size.x,
				create_info_copy.min_size.y
			};
			max_extent		= {
				create_info_copy.max_size.x,
				create_info_copy.max_size.y
			};

			// Set window size limits
			glfwSetWindowSizeLimits(
				glfw_window,
				int( min_extent.width ),
				int( min_extent.height ),
				int( max_extent.width ),
				int( max_extent.height )
			);

			// Get new surface capabilities as window extent might have changed
			auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				vk_physical_device,
				vk_surface,
				&surface_capabilities
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot query physical device surface capabilities, cannot continue!" );
				return false;
			}

			extent	= surface_capabilities.currentExtent;
		}

		// Figure out present mode
		{
			bool present_mode_found		= false;
			if( create_info_copy.vsync ) {
				// Using VSync we should use FIFO, this mode is required to be supported so we can rely on that and just use it without checking
				present_mode			= VK_PRESENT_MODE_FIFO_KHR;
				present_mode_found		= true;
			} else {
				// Not using VSync, we should try mailbox first and immediate second, fall back to FIFO if neither is supported
				std::vector<VkPresentModeKHR> surface_present_modes;
				{
					uint32_t present_mode_count = 0;
					result = vkGetPhysicalDeviceSurfacePresentModesKHR(
						vk_physical_device,
						vk_surface,
						&present_mode_count,
						nullptr
					);
					if( result != VK_SUCCESS ) {
						instance.Report( result, "Internal error: Cannot query physical device surface present modes!" );
						return false;
					}
					surface_present_modes.resize( present_mode_count );
					result = vkGetPhysicalDeviceSurfacePresentModesKHR(
						vk_physical_device,
						vk_surface,
						&present_mode_count,
						surface_present_modes.data()
					);
					if( result != VK_SUCCESS ) {
						instance.Report( result, "Internal error: Cannot query physical device surface present modes!" );
						return false;
					}
				}
				// Since there are only 2 things we're interested in finding we'll do a simple
				// check if we could find either, if we found the better one, break out so we
				// don't pick the worse option later.
				for( auto p : surface_present_modes ) {
					if( p == VK_PRESENT_MODE_MAILBOX_KHR ) {
						present_mode	= VK_PRESENT_MODE_MAILBOX_KHR;
						present_mode_found	= true;
						break;	// found the best, break out
					} else if( p == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
						present_mode	= VK_PRESENT_MODE_IMMEDIATE_KHR;
						present_mode_found	= true;
					}
				}
			}
			if( !present_mode_found ) {
				// Present mode not found, use FIFO as a fallback
				present_mode		= VK_PRESENT_MODE_FIFO_KHR;
			}

			VkSwapchainCreateInfoKHR swapchain_create_info {};
			swapchain_create_info.sType						= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_create_info.pNext						= nullptr;
			swapchain_create_info.flags						= 0;
			swapchain_create_info.surface					= vk_surface;
			swapchain_create_info.minImageCount				= swapchain_minimum_image_count;
			swapchain_create_info.imageFormat				= surface_format.format;
			swapchain_create_info.imageColorSpace			= surface_format.colorSpace;
			swapchain_create_info.imageExtent				= extent;
			swapchain_create_info.imageArrayLayers			= 1;
			swapchain_create_info.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			swapchain_create_info.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount		= 0;
			swapchain_create_info.pQueueFamilyIndices		= nullptr;
			swapchain_create_info.preTransform				= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchain_create_info.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;		// Check this if rendering transparent windows
			swapchain_create_info.presentMode				= present_mode;
			swapchain_create_info.clipped					= VK_TRUE;
			swapchain_create_info.oldSwapchain				= old_vk_swapchain;

			result = vkCreateSwapchainKHR(
				vk_device,
				&swapchain_create_info,
				nullptr,
				&vk_swapchain
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot create Vulkan swapchain!" );
				return false;
			}
		}

		// Get swapchain images and create image views
		{
			swapchain_image_count = 0;
			result = vkGetSwapchainImagesKHR(
				vk_device,
				vk_swapchain,
				&swapchain_image_count,
				nullptr
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot query Vulkan swapchain images!" );
				return false;
			}
			assert( swapchain_image_count > 0 );
			vk_swapchain_images.resize( swapchain_image_count );
			result = vkGetSwapchainImagesKHR(
				vk_device,
				vk_swapchain,
				&swapchain_image_count,
				vk_swapchain_images.data()
			);
			if( result != VK_SUCCESS ) {
				instance.Report( result, "Internal error: Cannot query Vulkan swapchain images!" );
				return false;
			}

			// Destroy old swapchain image views if they exist
			for( auto v : vk_swapchain_image_views ) {
				vkDestroyImageView(
					vk_device,
					v,
					nullptr
				);
			}

			vk_swapchain_image_views.resize( swapchain_image_count );
			for( size_t i = 0; i < swapchain_image_count; ++i ) {

				VkImageViewCreateInfo image_view_create_info {};
				image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_create_info.pNext				= nullptr;
				image_view_create_info.flags				= 0;
				image_view_create_info.image				= vk_swapchain_images[ i ];
				image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
				image_view_create_info.format				= surface_format.format;
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

				result = vkCreateImageView(
					vk_device,
					&image_view_create_info,
					nullptr,
					&vk_swapchain_image_views[ i ]
				);
				if( result != VK_SUCCESS ) {
					instance.Report( result, "Internal error: Cannot create Vulkan swapchain image views!" );
					return false;
				}
			}
		}
	}

	// Destroy old swapchain if it exists
	{
		vkDestroySwapchainKHR(
			vk_device,
			old_vk_swapchain,
			nullptr
		);
	}

	should_reconstruct		= false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::ReCreateScreenshotResources()
{
	while( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE ) {
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
	}

	auto & memory_pool	= device_memory_pool;

	memory_pool.FreeCompleteResource( screenshot_buffer );
	memory_pool.FreeCompleteResource( screenshot_image );

	VkImageCreateInfo image_create_info {};
	image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext						= nullptr;
	image_create_info.flags						= 0;
	image_create_info.imageType					= VK_IMAGE_TYPE_2D;
	image_create_info.format					= VK_FORMAT_R8G8B8A8_UNORM;
	image_create_info.extent					= { extent.width, extent.height, 1 };
	image_create_info.mipLevels					= 1;
	image_create_info.arrayLayers				= 1;
	image_create_info.samples					= VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage						= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.queueFamilyIndexCount		= 0;
	image_create_info.pQueueFamilyIndices		= nullptr;
	image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
	screenshot_image = device_memory_pool.CreateCompleteImageResource(
		&image_create_info,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	if( screenshot_image != VK_SUCCESS ) {
		instance.Report( screenshot_image.result, "Internal error: Cannot create internal screenshot image, screenshots disabled!" );
		screenshot_state	= vk2d::vk2d_internal::WindowImpl::ScreenshotState::IDLE_ERROR;
		return false;
	}

	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext					= nullptr;
	buffer_create_info.flags					= 0;
	buffer_create_info.size						= VkDeviceSize( extent.width ) * VkDeviceSize( extent.height ) * 4;
	buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount	= 0;
	buffer_create_info.pQueueFamilyIndices		= nullptr;
	screenshot_buffer = device_memory_pool.CreateCompleteBufferResource(
		&buffer_create_info,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);
	if( screenshot_buffer != VK_SUCCESS ) {
		instance.Report( screenshot_buffer.result, "Internal error: Cannot create internal screenshot buffer, screenshots disabled!" );
		screenshot_state	= vk2d::vk2d_internal::WindowImpl::ScreenshotState::IDLE_ERROR;
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateFramebuffers()
{
	vk_framebuffers.resize( swapchain_image_count );

	bool use_multisampling		= samples != Multisamples::SAMPLE_COUNT_1;
	if( use_multisampling ) {
		multisample_render_targets.resize( swapchain_image_count );
	}

	for( uint32_t i = 0; i < swapchain_image_count; ++i ) {
		if( use_multisampling ) {
			VkImageCreateInfo						image_create_info {};
			image_create_info.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_create_info.pNext					= nullptr;
			image_create_info.flags					= 0;
			image_create_info.imageType				= VK_IMAGE_TYPE_2D;
			image_create_info.format				= surface_format.format;
			image_create_info.extent				= { extent.width, extent.height, 1 };
			image_create_info.mipLevels				= 1;
			image_create_info.arrayLayers			= 1;
			image_create_info.samples				= VkSampleCountFlagBits( samples );
			image_create_info.tiling				= VK_IMAGE_TILING_OPTIMAL;
			image_create_info.usage					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			image_create_info.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			image_create_info.queueFamilyIndexCount	= 0;
			image_create_info.pQueueFamilyIndices	= nullptr;
			image_create_info.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;

			VkImageViewCreateInfo					image_view_create_info {};
			image_view_create_info.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.pNext			= nullptr;
			image_view_create_info.flags			= 0;
			image_view_create_info.image			= VK_NULL_HANDLE;
			image_view_create_info.viewType			= VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format			= surface_format.format;
			image_view_create_info.components		= {
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

			multisample_render_targets[ i ] = device_memory_pool.CreateCompleteImageResource(
				&image_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&image_view_create_info
			);
			if( multisample_render_targets[ i ] != VK_SUCCESS ) {
				instance.Report( multisample_render_targets[ i ].result, "Internal error: Cannot create multisample render targets!" );
				return false;
			}
		}

		std::vector<VkImageView>	attachments;
		if( use_multisampling ) {
			attachments.push_back( multisample_render_targets[ i ].view );
		}
		attachments.push_back( vk_swapchain_image_views[ i ] );

		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext				= nullptr;
		framebuffer_create_info.flags				= 0;
		framebuffer_create_info.renderPass			= vk_render_pass;
		framebuffer_create_info.attachmentCount		= uint32_t( attachments.size() );
		framebuffer_create_info.pAttachments		= attachments.data();
		framebuffer_create_info.width				= extent.width;
		framebuffer_create_info.height				= extent.height;
		framebuffer_create_info.layers				= 1;

		auto result = vkCreateFramebuffer(
			vk_device,
			&framebuffer_create_info,
			nullptr,
			&vk_framebuffers[ i ]
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create Vulkan framebuffers!" );
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateWindowSynchronizationPrimitives()
{
	auto result = VK_SUCCESS;

	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;

	result = vkCreateFence(
		vk_device,
		&fence_create_info,
		nullptr,
		&vk_acquire_image_fence
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create image aquisition fence!" );
		return false;
	}

	VkSemaphoreCreateInfo mesh_transfer_semaphore_create_info {};
	mesh_transfer_semaphore_create_info.sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	mesh_transfer_semaphore_create_info.pNext	= nullptr;
	mesh_transfer_semaphore_create_info.flags	= 0;

	result = vkCreateSemaphore(
		vk_device,
		&mesh_transfer_semaphore_create_info,
		nullptr,
		&vk_transfer_semaphore
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create mesh transfer semaphore!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateFrameSynchronizationPrimitives()
{
	vk_submit_to_present_semaphores.resize( swapchain_image_count );

	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext		= nullptr;
	semaphore_create_info.flags		= 0;

	for( auto & s : vk_submit_to_present_semaphores ) {
		auto result = vkCreateSemaphore(
			vk_device,
			&semaphore_create_info,
			nullptr,
			&s
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create frame synchronization semaphores!" );
			return false;
		}
	}

	vk_gpu_to_cpu_frame_fences.resize( swapchain_image_count );

	VkFenceCreateInfo			fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;

	for( auto & f : vk_gpu_to_cpu_frame_fences ) {
		auto result =vkCreateFence(
			vk_device,
			&fence_create_info,
			nullptr,
			&f
		);
		if( result != VK_SUCCESS ) {
			instance.Report( result, "Internal error: Cannot create frame synchronization fences!" );
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CreateWindowFrameDataBuffer()
{
	// Create staging and device buffers
	{
		VkBufferCreateInfo staging_buffer_create_info {};
		staging_buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_buffer_create_info.pNext					= nullptr;
		staging_buffer_create_info.flags					= 0;
		staging_buffer_create_info.size						= sizeof( FrameData );
		staging_buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		staging_buffer_create_info.queueFamilyIndexCount	= 0;
		staging_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_staging_buffer = device_memory_pool.CreateCompleteBufferResource(
			&staging_buffer_create_info,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		if( frame_data_staging_buffer != VK_SUCCESS ) {
			instance.Report( frame_data_staging_buffer.result, "Internal error. Cannot create staging buffer for FrameData!" );
			return false;
		}

		VkBufferCreateInfo device_buffer_create_info {};
		device_buffer_create_info.sType						= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		device_buffer_create_info.pNext						= nullptr;
		device_buffer_create_info.flags						= 0;
		device_buffer_create_info.size						= sizeof( FrameData );
		device_buffer_create_info.usage						= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		device_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		device_buffer_create_info.queueFamilyIndexCount		= 0;
		device_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_device_buffer = device_memory_pool.CreateCompleteBufferResource(
			&device_buffer_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if( frame_data_device_buffer != VK_SUCCESS ) {
			instance.Report( frame_data_device_buffer.result, "Internal error. Cannot create device local buffer for FrameData!" );
			return false;
		}
	}

	// Create descriptor set
	{
		frame_data_descriptor_set	= instance.AllocateDescriptorSet(
			instance.GetGraphicsUniformBufferDescriptorSetLayout()
		);
		if( frame_data_descriptor_set != VK_SUCCESS ) {
			instance.Report( frame_data_descriptor_set.result, "Internal error: Cannot allocate descriptor set for FrameData device buffer!" );
			return false;
		}
		VkDescriptorBufferInfo descriptor_write_buffer_info {};
		descriptor_write_buffer_info.buffer	= frame_data_device_buffer.buffer;
		descriptor_write_buffer_info.offset	= 0;
		descriptor_write_buffer_info.range	= sizeof( FrameData );
		VkWriteDescriptorSet descriptor_write {};
		descriptor_write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.pNext				= nullptr;
		descriptor_write.dstSet				= frame_data_descriptor_set.descriptorSet;
		descriptor_write.dstBinding			= 0;
		descriptor_write.dstArrayElement	= 0;
		descriptor_write.descriptorCount	= 1;
		descriptor_write.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.pImageInfo			= nullptr;
		descriptor_write.pBufferInfo		= &descriptor_write_buffer_info;
		descriptor_write.pTexelBufferView	= nullptr;
		vkUpdateDescriptorSets(
			vk_device,
			1, &descriptor_write,
			0, nullptr
		);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CommitRenderTargetTextureRender(
	RenderTargetTextureRenderCollector		&	collector
)
{
	for( auto & d : render_target_texture_dependencies[ next_image ] ) {
		if( !d.render_target->CommitRenderTargetTextureRender( d, collector ) ) {
			return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::ConfirmRenderTargetTextureRenderSubmission()
{
	for( auto & d : render_target_texture_dependencies[ next_image ] ) {
		d.render_target->ConfirmRenderTargetTextureRenderSubmission( d );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::ConfirmRenderTargetTextureRenderFinished(
	uint32_t	for_frame_image_index
)
{
	for( auto & d : render_target_texture_dependencies[ for_frame_image_index ] ) {
		d.render_target->ConfirmRenderTargetTextureRenderFinished( d );
	}
	render_target_texture_dependencies[ for_frame_image_index ].clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::AbortRenderTargetTextureRender()
{
	for( auto & d : render_target_texture_dependencies[ next_image ] ) {
		d.render_target->AbortRenderTargetTextureRender( d );
	}
	render_target_texture_dependencies[ next_image ].clear();
}
//
//void vk2d::vk2d_internal::WindowImpl::ClearRenderTargetTextureDepencies()
//{
//	for( auto & d : render_target_texture_dependencies ) {
//		d.render_target->ClearRenderTargetTextureDepencies( d );
//	}
//	render_target_texture_dependencies.clear();
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CheckAndAddRenderTargetTextureDependency(
	Texture		*	texture
)
{
	auto render_target = dynamic_cast<RenderTargetTextureImpl*>( texture->texture_impl );
	if( render_target ) {
		if( std::none_of(
			render_target_texture_dependencies[ next_image ].begin(),
			render_target_texture_dependencies[ next_image ].end(),
			[render_target]( RenderTargetTextureDependencyInfo & rt )
			{
				if( render_target == rt.render_target ) return true;
				return false;
			} ) ) {
			render_target_texture_dependencies[ next_image ].push_back( render_target->GetDependencyInfo() );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::HandleScreenshotEvent()
{
	assert( screenshot_state == vk2d::vk2d_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT );

	if( event_handler ) {
		if( !screenshot_save_path.empty() ) {
			event_handler->EventScreenshot(
				my_interface,
				screenshot_save_path,
				{},
				!screenshot_event_error,
				screenshot_event_message
			);
		} else {
			event_handler->EventScreenshot(
				my_interface,
				{},
				screenshot_save_data,
				!screenshot_event_error,
				screenshot_event_message
			);
		}
	}

	screenshot_save_path			= "";
	screenshot_event_error			= false;
	screenshot_event_message		= "";
	screenshot_state				= vk2d::vk2d_internal::WindowImpl::ScreenshotState::IDLE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CmdBindGraphicsPipelineIfDifferent(
	VkCommandBuffer											command_buffer,
	const GraphicsPipelineSettings		&	pipeline_settings
)
{
	if( previous_pipeline_settings != pipeline_settings ) {
		auto pipeline = instance.GetGraphicsPipeline( pipeline_settings );

		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline
		);
		previous_pipeline_settings	= pipeline_settings;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CmdBindSamplerIfDifferent(
	VkCommandBuffer			command_buffer,
	Sampler		*	sampler
)
{
	assert( sampler );

	// if sampler or texture changed since previous call, bind a different descriptor set.
	if( sampler != previous_sampler ) {
		auto & set = sampler_descriptor_sets[ sampler ];

		// If this descriptor set doesn't exist yet for this
		// sampler texture combo, create one and update it.
		if( set.descriptor_set.descriptorSet == VK_NULL_HANDLE ) {
			set.descriptor_set = instance.AllocateDescriptorSet(
				instance.GetGraphicsSamplerDescriptorSetLayout()
			);

			VkDescriptorImageInfo image_info {};
			image_info.sampler						= sampler->impl->GetVulkanSampler();
			image_info.imageView					= VK_NULL_HANDLE;
			image_info.imageLayout					= VK_IMAGE_LAYOUT_UNDEFINED;

			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer						= sampler->impl->GetVulkanBufferForSamplerData();
			buffer_info.offset						= 0;
			buffer_info.range						= sizeof( SamplerImpl::BufferData );

			std::array<VkWriteDescriptorSet, 2> descriptor_write {};
			descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[ 0 ].pNext				= nullptr;
			descriptor_write[ 0 ].dstSet			= set.descriptor_set.descriptorSet;
			descriptor_write[ 0 ].dstBinding		= 0;
			descriptor_write[ 0 ].dstArrayElement	= 0;
			descriptor_write[ 0 ].descriptorCount	= 1;
			descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_SAMPLER;
			descriptor_write[ 0 ].pImageInfo		= &image_info;
			descriptor_write[ 0 ].pBufferInfo		= nullptr;
			descriptor_write[ 0 ].pTexelBufferView	= nullptr;

			descriptor_write[ 1 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[ 1 ].pNext				= nullptr;
			descriptor_write[ 1 ].dstSet			= set.descriptor_set.descriptorSet;
			descriptor_write[ 1 ].dstBinding		= 1;
			descriptor_write[ 1 ].dstArrayElement	= 0;
			descriptor_write[ 1 ].descriptorCount	= 1;
			descriptor_write[ 1 ].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_write[ 1 ].pImageInfo		= nullptr;
			descriptor_write[ 1 ].pBufferInfo		= &buffer_info;
			descriptor_write[ 1 ].pTexelBufferView	= nullptr;

			vkUpdateDescriptorSets(
				instance.GetVulkanDevice(),
				uint32_t( descriptor_write.size() ), descriptor_write.data(),
				0, nullptr
			);
		}
		set.previous_access_time = std::chrono::steady_clock::now();

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			instance.GetGraphicsPrimaryRenderPipelineLayout(),
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_SAMPLER_AND_SAMPLER_DATA,
			1, &set.descriptor_set.descriptorSet,
			0, nullptr
		);

		previous_sampler		= sampler;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CmdBindTextureIfDifferent(
	VkCommandBuffer			command_buffer,
	Texture		*	texture
)
{
	assert( texture );

	// if sampler or texture changed since previous call, bind a different descriptor set.
	if( texture != previous_texture ) {
		auto & set = texture_descriptor_sets[ texture ];

		// If this descriptor set doesn't exist yet for this
		// sampler texture combo, create one and update it.
		if( set.descriptor_set.descriptorSet == VK_NULL_HANDLE ) {
			set.descriptor_set = instance.AllocateDescriptorSet(
				instance.GetGraphicsTextureDescriptorSetLayout()
			);

			VkDescriptorImageInfo image_info {};
			image_info.sampler						= VK_NULL_HANDLE;
			image_info.imageView					= texture->texture_impl->GetVulkanImageView();
			image_info.imageLayout					= texture->texture_impl->GetVulkanImageLayout();

			std::array<VkWriteDescriptorSet, 1> descriptor_write {};
			descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[ 0 ].pNext				= nullptr;
			descriptor_write[ 0 ].dstSet			= set.descriptor_set.descriptorSet;
			descriptor_write[ 0 ].dstBinding		= 0;
			descriptor_write[ 0 ].dstArrayElement	= 0;
			descriptor_write[ 0 ].descriptorCount	= 1;
			descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptor_write[ 0 ].pImageInfo		= &image_info;
			descriptor_write[ 0 ].pBufferInfo		= nullptr;
			descriptor_write[ 0 ].pTexelBufferView	= nullptr;

			vkUpdateDescriptorSets(
				instance.GetVulkanDevice(),
				uint32_t( descriptor_write.size() ), descriptor_write.data(),
				0, nullptr
			);
		}
		set.previous_access_time = std::chrono::steady_clock::now();

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			instance.GetGraphicsPrimaryRenderPipelineLayout(),
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE,
			1, &set.descriptor_set.descriptorSet,
			0, nullptr
		);

		previous_texture		= texture;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::WindowImpl::CmdSetLineWidthIfDifferent(
	VkCommandBuffer						command_buffer,
	float								line_width
)
{
	if( previous_line_width != line_width ) {

		vkCmdSetLineWidth(
			command_buffer,
			line_width
		);

		previous_line_width	= line_width;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::WindowImpl::CmdUpdateFrameData(
	VkCommandBuffer			command_buffer
)
{
	// Window coordinate system scaling
	WindowCoordinateScaling window_coordinate_scaling {};

	switch( coordinate_space ) {
		case RenderCoordinateSpace::TEXEL_SPACE:
			window_coordinate_scaling.multiplier	= { 1.0f / ( extent.width / 2.0f ), 1.0f / ( extent.height / 2.0f ) };
			window_coordinate_scaling.offset		= { -1.0f, -1.0f };
			break;
		case RenderCoordinateSpace::TEXEL_SPACE_CENTERED:
			window_coordinate_scaling.multiplier	= { 1.0f / ( extent.width / 2.0f ), 1.0f / ( extent.height / 2.0f ) };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
			break;
		case RenderCoordinateSpace::NORMALIZED_SPACE:
		{
			float contained_minimum_dimension		= float( std::min( extent.width, extent.height ) );
			window_coordinate_scaling.multiplier	= { contained_minimum_dimension / ( extent.width / 2.0f ), contained_minimum_dimension / ( extent.height / 2.0f ) };
			window_coordinate_scaling.offset		= { -1.0f, -1.0f };
		}
		break;
		case RenderCoordinateSpace::NORMALIZED_SPACE_CENTERED:
		{
			float contained_minimum_dimension		= float( std::min( extent.width, extent.height ) );
			window_coordinate_scaling.multiplier	= { contained_minimum_dimension / extent.width, contained_minimum_dimension / extent.height };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
		}
		break;
		case RenderCoordinateSpace::LINEAR_SPACE:
			window_coordinate_scaling.multiplier	= { 1.0f, 1.0f };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
			break;
		default:
			window_coordinate_scaling.multiplier	= { 1.0f, 1.0f };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
			break;
	}

	// Copy data to staging buffer.
	{
		auto frame_data = frame_data_staging_buffer.memory.Map<FrameData>();
		if( !frame_data ) {
			instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot map FrameData staging buffer memory!" );
			return false;
		}
		frame_data->coordinate_scaling		= window_coordinate_scaling;
		frame_data_staging_buffer.memory.Unmap();
	}
	// Record transfer commands from staging buffer to device local buffer.
	{
		VkBufferCopy copy_region {};
		copy_region.srcOffset	= 0;
		copy_region.dstOffset	= 0;
		copy_region.size		= sizeof( FrameData );
		vkCmdCopyBuffer(
			command_buffer,
			frame_data_staging_buffer.buffer,
			frame_data_device_buffer.buffer,
			1, &copy_region
		);
	}

	return true;
}
