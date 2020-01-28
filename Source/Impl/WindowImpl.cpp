
#include "../Header/Core/SourceCommon.h"

#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Impl/RendererImpl.h"
#include "../../Include/Interface/TextureResource.h"
#include "../Header/Impl/TextureResourceImpl.h"
#include "../../Include/Interface/Mesh.h"
#include "../Header/Impl/SamplerImpl.h"

#include <memory>
#include <algorithm>
#include <set>
#include <sstream>
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>



namespace vk2d {

namespace _internal {



// Function declarations
void glfwJoystickEventCallback( int joystick, int event );
void glfwWindowPosCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwWindowSizeCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwWindowCloseCallback( GLFWwindow * glfwWindow );
void glfwWindowRefreshCallback( GLFWwindow * glfwWindow );
void glfwWindowFocusCallback( GLFWwindow * glfwWindow, int focus );
void glfwWindowIconifyCallback( GLFWwindow * glfwWindow, int iconify );
void glfwFramebufferSizeCallback( GLFWwindow * glfwWindow, int x, int y );
void glfwMouseButtonCallback( GLFWwindow * glfwWindow, int button, int action, int mods );
void glfwCursorPosCallback( GLFWwindow * glfwWindow, double x, double y );
void glfwCursorEnterCallback( GLFWwindow * glfwWindow, int enter );
void glfwScrollCallback( GLFWwindow * glfwWindow, double x, double y );
void glfwKeyCallback( GLFWwindow * glfwWindow, int key, int scancode, int action, int mods );
void glfwCharModsCallback( GLFWwindow * glfwWindow, unsigned int codepoint, int mods );
void glfwFileDropCallback( GLFWwindow * glfwWindow, int fileCount, const char ** filePaths );



} // _internal

} // vk2d



///////////////////////////////////////////////////////////
// Internal callback functions
///////////////////////////////////////////////////////////

void vk2d::_internal::glfwWindowPosCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	impl->position = { int32_t( x ), int32_t( y ) };
	if( impl->event_handler ) {
		impl->event_handler->EventWindowPosition( impl->window_parent, { int32_t( x ), int32_t( y ) }  );
	}
}

void vk2d::_internal::glfwWindowSizeCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->extent					= { uint32_t( x ), uint32_t( y ) };
	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowSize( impl->window_parent, { uint32_t( x ), uint32_t( y ) } );
	}
}

void vk2d::_internal::glfwWindowCloseCallback(
	GLFWwindow		*	glfwWindow
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->should_close			= true;
//	glfwHideWindow( impl->glfw_window );
	if( impl->event_handler ) {
		impl->event_handler->EventWindowClose( impl->window_parent );
	}
}

void vk2d::_internal::glfwWindowRefreshCallback(
	GLFWwindow		*	glfwWindow
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowRefresh( impl->window_parent );
	}
}

void vk2d::_internal::glfwWindowFocusCallback(
	GLFWwindow		*	glfwWindow,
	int					focus
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	if( impl->event_handler ) {
		impl->event_handler->EventWindowFocus( impl->window_parent, bool( focus ) );
	}
}

void vk2d::_internal::glfwWindowIconifyCallback(
	GLFWwindow		*	glfwWindow,
	int					iconify
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	if( iconify ) {
		impl->is_minimized			= true;
	} else {
		impl->is_minimized			= false;
		impl->should_reconstruct	= true;
	}
	if( impl->event_handler ) {
		impl->event_handler->EventWindowIconify( impl->window_parent, bool( iconify ) );
	}
}

void vk2d::_internal::glfwFramebufferSizeCallback(
	GLFWwindow		*	glfwWindow,
	int					x,
	int					y
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );

	impl->extent					= { uint32_t( x ), uint32_t( y ) };
	impl->should_reconstruct		= true;
	if( impl->event_handler ) {
		impl->event_handler->EventWindowSize( impl->window_parent, { uint32_t( x ), uint32_t( y ) } );
	}
}



void vk2d::_internal::glfwMouseButtonCallback(
	GLFWwindow		*	glfwWindow,
	int					button,
	int					action,
	int					mods
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventMouseButton( impl->window_parent, vk2d::MouseButton( button ), vk2d::ButtonAction( action ), vk2d::ModifierKeyFlags( mods ) );
	}
}

void vk2d::_internal::glfwCursorPosCallback(
	GLFWwindow		*	glfwWindow,
	double				x,
	double				y
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCursorPosition( impl->window_parent, { x, y } );
	}
}

void vk2d::_internal::glfwCursorEnterCallback(
	GLFWwindow		*	glfwWindow,
	int					enter
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCursorEnter( impl->window_parent, bool( enter ) );
	}
}

void vk2d::_internal::glfwScrollCallback(
	GLFWwindow		*	glfwWindow,
	double				x,
	double				y
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventScroll( impl->window_parent, { x, y } );
	}
}

void vk2d::_internal::glfwKeyCallback(
	GLFWwindow		*	glfwWindow,
	int					key,
	int					scancode,
	int					action,
	int					mods
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventKeyboard( impl->window_parent, vk2d::KeyboardButton( key ), scancode, vk2d::ButtonAction( action ), vk2d::ModifierKeyFlags( mods ) );
	}
}

void vk2d::_internal::glfwCharModsCallback(
	GLFWwindow		*	glfwWindow,
	unsigned int		codepoint,
	int					mods
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		impl->event_handler->EventCharacter( impl->window_parent, codepoint, vk2d::ModifierKeyFlags( mods ) );
	}
}

void vk2d::_internal::glfwFileDropCallback(
	GLFWwindow		*	glfwWindow,
	int					fileCount,
	const char		**	filePaths
)
{
	auto impl = reinterpret_cast<vk2d::_internal::WindowImpl*>( glfwGetWindowUserPointer( glfwWindow ) );
	if( impl->event_handler ) {
		std::vector<std::filesystem::path> files( fileCount );
		for( int i = 0; i < fileCount; ++i ) {
			files[ i ]		= filePaths[ i ];
		}
		impl->event_handler->EventFileDrop( impl->window_parent, files );
	}
}







vk2d::_internal::WindowImpl::WindowImpl(
	vk2d::Window					*	window,
	vk2d::_internal::RendererImpl	*	renderer,
	const vk2d::WindowCreateInfo	&	window_create_info
)
{
	create_info_copy				= window_create_info;
	window_parent					= window;
	renderer_parent					= renderer;
	report_function					= renderer_parent->GetReportFunction();
	window_title					= create_info_copy.title;
	event_handler					= create_info_copy.event_handler;

	{
		samples							= create_info_copy.samples;
		vk2d::Multisamples max_samples	= renderer_parent->GetMaximumSupportedMultisampling();
		if( uint32_t( samples ) > uint32_t( max_samples ) ) {
			std::stringstream ss;
			ss << "Window parameter: 'vk2d::WindowCreateInfo::samples' was larger than the system supports.\n"
				<< "Maximum supported amount for this system is: '" << uint32_t( max_samples ) << "'.";
			renderer_parent->Report(
				vk2d::ReportSeverity::WARNING,
				ss.str()
			);
			samples						= max_samples;
		}
		vk2d::Multisamples supported_samples	= vk2d::Multisamples( renderer_parent->GetPhysicalDeviceProperties().limits.framebufferColorSampleCounts );
		if( !( uint32_t( samples ) & uint32_t( supported_samples ) ) ) {
			samples								= vk2d::Multisamples::SAMPLE_COUNT_1;
		}
	}

	instance						= renderer_parent->GetVulkanInstance();
	physical_device					= renderer_parent->GetVulkanPhysicalDevice();
	device							= renderer_parent->GetVulkanDevice();
	primary_render_queue			= renderer_parent->GetPrimaryRenderQueue();

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

	mesh_buffer		= std::make_unique<vk2d::_internal::MeshBuffer>(
		renderer_parent,
		device,
		renderer->GetPhysicalDeviceProperties().limits,
		this,
		renderer->GetDeviceMemoryPool() );

	if( !mesh_buffer ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create MeshBuffer object!" );
		return;
	}

	{
		int32_t x = 0, y = 0;
		glfwGetWindowPos( glfw_window, &x, &y );
		position	= { x, y };

		glfwGetWindowSize( glfw_window, &x, &y );
		extent		= { uint32_t( x ), uint32_t( y ) };
	}
	{
		glfwSetWindowUserPointer( glfw_window, this );

		glfwSetWindowPosCallback( glfw_window, vk2d::_internal::glfwWindowPosCallback );
		glfwSetWindowSizeCallback( glfw_window, vk2d::_internal::glfwWindowSizeCallback );
		glfwSetWindowCloseCallback( glfw_window, vk2d::_internal::glfwWindowCloseCallback );
		glfwSetWindowRefreshCallback( glfw_window, vk2d::_internal::glfwWindowRefreshCallback );
		glfwSetWindowFocusCallback( glfw_window, vk2d::_internal::glfwWindowFocusCallback );
		glfwSetWindowIconifyCallback( glfw_window, vk2d::_internal::glfwWindowIconifyCallback );
		glfwSetFramebufferSizeCallback( glfw_window, vk2d::_internal::glfwFramebufferSizeCallback );

		glfwSetMouseButtonCallback( glfw_window, vk2d::_internal::glfwMouseButtonCallback );
		glfwSetCursorPosCallback( glfw_window, vk2d::_internal::glfwCursorPosCallback );
		glfwSetCursorEnterCallback( glfw_window, vk2d::_internal::glfwCursorEnterCallback );
		glfwSetScrollCallback( glfw_window, vk2d::_internal::glfwScrollCallback );
		glfwSetKeyCallback( glfw_window, vk2d::_internal::glfwKeyCallback );
		glfwSetCharModsCallback( glfw_window, vk2d::_internal::glfwCharModsCallback );
		glfwSetDropCallback( glfw_window, vk2d::_internal::glfwFileDropCallback );
	}

	is_good		= true;
}









vk2d::_internal::WindowImpl::~WindowImpl()
{
	vkDeviceWaitIdle( device );

	while( screenshot_being_saved ) {
		std::this_thread::sleep_for( std::chrono::microseconds( 500 ) );
	};
	if( screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT ) {
		HandleScreenshotEvent();
	}

	renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( screenshot_image );
	renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( screenshot_buffer );

	mesh_buffer		= nullptr;

	renderer_parent->GetDescriptorPool()->FreeDescriptorSet( frame_data_descriptor_set );
	renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( frame_data_device_buffer );
	renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( frame_data_staging_buffer );

	for( auto f : gpu_to_cpu_frame_fences ) {
		vkDestroyFence(
			device,
			f,
			nullptr
		);
	}

	for( auto s : submit_to_present_semaphores ) {
		vkDestroySemaphore(
			device,
			s,
			nullptr
		);
	}

	vkDestroySemaphore(
		device,
		mesh_transfer_semaphore,
		nullptr
	);

	vkDestroyFence(
		device,
		aquire_image_fence,
		nullptr
	);

	vkDestroyCommandPool(
		device,
		command_pool,
		nullptr
	);

	for( auto f : framebuffers ) {
		vkDestroyFramebuffer(
			device,
			f,
			nullptr
		);
	}

	for( auto m : multisample_render_targets ) {
		renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( m );
	}

	for( auto v : swapchain_image_views ) {
		vkDestroyImageView(
			device,
			v,
			nullptr
		);
	}

	vkDestroySwapchainKHR(
		device,
		swapchain,
		nullptr
	);

	vkDestroyRenderPass(
		device,
		render_pass,
		nullptr
	);

	vkDestroySurfaceKHR(
		instance,
		surface,
		nullptr
	);

	glfwDestroyWindow( glfw_window );
}









void vk2d::_internal::WindowImpl::CloseWindow()
{
	should_close = true;
}

bool vk2d::_internal::WindowImpl::ShouldClose()
{
	return should_close;
}



bool vk2d::_internal::AquireImage(
	vk2d::_internal::WindowImpl		*	impl,
	VkPhysicalDevice					physical_device,
	VkDevice							device,
	vk2d::_internal::ResolvedQueue	&	primary_render_queue,
	uint32_t							nested_counter				= 0 )
{
	auto result = vkAcquireNextImageKHR(
		device,
		impl->swapchain,
		UINT64_MAX,
		VK_NULL_HANDLE,
		impl->aquire_image_fence,
		&impl->next_image
	);
	if( result != VK_SUCCESS ) {
		if( result == VK_SUBOPTIMAL_KHR ) {
			// Image aquired but is not optimal, continue but recreate swapchain next time we begin the render again.
			impl->renderer_parent->Report( vk2d::ReportSeverity::INFO, "Aquired suboptimal image, continue but recreate swapchain next frame." );
			impl->should_reconstruct		= true;
		} else if( result == VK_ERROR_OUT_OF_DATE_KHR ) {
			// Image was not aquired so we cannot present anything until we recreate the swapchain.
			impl->renderer_parent->Report( vk2d::ReportSeverity::INFO, "Could not aquire image, out of date swapchain, recreate swapchain now." );
			if( nested_counter ) {
				// Breaking out of nested call here, we tried aquiring an image twice before
				// now and it didn't work so we can assume it will not work and we can give up here.
				impl->renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Could not aquire image after retry, out of date swapchain, Cannot recreate swapchain!" );
				return false;
			}
			// Cannot continue before we recreate the swapchain
			if( !impl->RecreateWindowSizeDependantResources() ) {
				impl->renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot recreate window resources after resizing window!" );
				return false;
			}
			// After recreating the swapchain and resources, try to aquire
			// next image again, if that fails we should stop trying.
			if( !AquireImage(
				impl,
				physical_device,
				device,
				primary_render_queue,
				++nested_counter
			) ) {

			}
		} else {
			impl->renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot aquire next swapchain image!" );

			return false;
		}
	}
	vkWaitForFences(
		device,
		1, &impl->aquire_image_fence,
		VK_TRUE,
		UINT64_MAX
	);
	vkResetFences(
		device,
		1, &impl->aquire_image_fence
	);

	return true;
}

bool vk2d::_internal::WindowImpl::BeginRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != vk2d::_internal::NextRenderCallFunction::BEGIN ) {
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "'Window::BeginRender()' Called twice in a row!" );
		return false;
	} else {
		next_render_call_function = vk2d::_internal::NextRenderCallFunction::END;
	}

	if( should_reconstruct ) {
		if( !RecreateWindowSizeDependantResources() ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot recreate window resources after resizing window!" );
			return false;
		}
	}

	// Make sure we can write to the next command buffer by
	// aquiring a new image from the presentation engine,
	// this will tell which command buffer is ready to be reused
	{
		if( !AquireImage(
			this,
			physical_device,
			device,
			primary_render_queue
		) ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot aquire next swapchain image!" );
			return false;
		}
	}

	// If next image index happens to same as the previous, presentation has propably already succeeded but
	// since we're using the image index as an index to our command buffers and framebuffers we'll have to
	// make sure that we don't start overwriting a command buffer until it's execution has completely
	// finished, so we'll have to synchronize the frame early in here.
	if( next_image == previous_image ) {
		if( !SynchronizeFrame() ) {
			renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot synchronize frame, cannot output to window!" );
			return false;
		}
	}

	// Begin command buffer
	{
		VkCommandBuffer		command_buffer			= render_command_buffers[ next_image ];

		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext				= nullptr;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo	= nullptr;

		if( vkBeginCommandBuffer(
			command_buffer,
			&command_buffer_begin_info
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record primary render command buffer!" );
			return false;
		}
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"WindowImpl",
			vk2d::_internal::CommandBufferCheckpointType::BEGIN_COMMAND_BUFFER
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
				renderer_parent->GetPipelineLayout(),
				DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA,
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
			render_pass_begin_info.renderPass		= render_pass;
			render_pass_begin_info.framebuffer		= framebuffers[ next_image ];
			render_pass_begin_info.renderArea		= { { 0, 0 }, extent };
			render_pass_begin_info.clearValueCount	= 1;
			render_pass_begin_info.pClearValues		= &clear_value;

			vk2d::_internal::CmdInsertCommandBufferCheckpoint(
				command_buffer,
				"WindowImpl",
				vk2d::_internal::CommandBufferCheckpointType::BEGIN_RENDER_PASS
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









bool vk2d::_internal::WindowImpl::EndRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != vk2d::_internal::NextRenderCallFunction::END ) {
		renderer_parent->Report( vk2d::ReportSeverity::WARNING, "'Window::EndRender()' Called twice in a row!" );
		return false;
	} else {
		next_render_call_function = vk2d::_internal::NextRenderCallFunction::BEGIN;
	}

	VkCommandBuffer		render_command_buffer	= render_command_buffers[ next_image ];

	// End render pass
	{
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			render_command_buffer,
			"WindowImpl",
			vk2d::_internal::CommandBufferCheckpointType::END_RENDER_PASS
		);
		vkCmdEndRenderPass( render_command_buffer );
	}

	// Save screenshot if it was requested
	{
		if( screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::REQUESTED ) {

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
				imageMemoryBarriers[ 1 ].image					= swapchain_images[ next_image ];
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
					swapchain_images[ next_image ],
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
				imageMemoryBarriers[ 1 ].image					= swapchain_images[ next_image ];
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
				screenshot_state			= vk2d::_internal::WindowImpl::ScreenshotState::WAITING_RENDER;
				screenshot_swapchain_id		= next_image;
			}
		}
	}
	// End command buffer
	vk2d::_internal::CmdInsertCommandBufferCheckpoint(
		render_command_buffer,
		"WindowImpl",
		vk2d::_internal::CommandBufferCheckpointType::END_COMMAND_BUFFER
	);
	if( vkEndCommandBuffer( render_command_buffer ) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot compile primary render command buffer!" );
		return false;
	}

	// Synchronize the previous frame here, this waits for the previous
	// frame to finish fully rendering before continuing execution.
	if( !SynchronizeFrame() ) {
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot synchronize frame, cannot output to window!" );
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

			if( vkBeginCommandBuffer(
				complementary_transfer_command_buffer,
				&transfer_command_buffer_begin_info
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record mesh to GPU transfer command buffer!" );
				return false;
			}
		}

		// Record commands to upload frame data to gpu
		{
			if( !CmdUpdateFrameData(
				complementary_transfer_command_buffer
			) ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record commands to transfer WindowFrameData to GPU!" );
				return false;
			}
		}

		// Record commands to upload mesh data to gpu
		{
			if( !mesh_buffer->CmdUploadMeshDataToGPU(
				complementary_transfer_command_buffer
			) ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot record commands to transfer mesh data to GPU!" );
				return false;
			}
		}

		// End command buffer
		{
			if( vkEndCommandBuffer(
				complementary_transfer_command_buffer
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot compile mesh to GPU transfer command buffer!" );
				return false;
			}
		}
	}

	// Submit swapchain image
	{
		std::vector<VkSubmitInfo> submit_infos( 2 );

		submit_infos[ 0 ].sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_infos[ 0 ].pNext						= nullptr;
		submit_infos[ 0 ].waitSemaphoreCount		= 0;
		submit_infos[ 0 ].pWaitSemaphores			= nullptr;
		submit_infos[ 0 ].pWaitDstStageMask			= nullptr;
		submit_infos[ 0 ].commandBufferCount		= 1;
		submit_infos[ 0 ].pCommandBuffers			= &complementary_transfer_command_buffer;
		submit_infos[ 0 ].signalSemaphoreCount		= 1;
		submit_infos[ 0 ].pSignalSemaphores			= &mesh_transfer_semaphore;

		VkPipelineStageFlags wait_dst_stage_mask	= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		submit_infos[ 1 ].sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_infos[ 1 ].pNext						= nullptr;
		submit_infos[ 1 ].waitSemaphoreCount		= 1;
		submit_infos[ 1 ].pWaitSemaphores			= &mesh_transfer_semaphore;
		submit_infos[ 1 ].pWaitDstStageMask			= &wait_dst_stage_mask;
		submit_infos[ 1 ].commandBufferCount		= 1;
		submit_infos[ 1 ].pCommandBuffers			= &render_command_buffer;
		submit_infos[ 1 ].signalSemaphoreCount		= 1;
		submit_infos[ 1 ].pSignalSemaphores			= &submit_to_present_semaphores[ next_image ];

		auto result = primary_render_queue.Submit(
			submit_infos,
			gpu_to_cpu_frame_fences[ next_image ]
		);
		if( result != VK_SUCCESS ) {
			renderer_parent->Report(
				vk2d::ReportSeverity::CRITICAL_ERROR,
				"Internal error: Cannot submit frame end command buffers! " +
				vk2d::_internal::VkResultToString( result )
			);
			return false;
		}
	}

	// DEBUGGING ONLY:
//	{
//		auto result = vkDeviceWaitIdle( device );
//		if( result != VK_SUCCESS ) {
//			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, vk2d::_internal::VkResultToString( result ) );
//			return false;
//		}
//	}

	// Present swapchain image
	{
		VkResult present_result				= VK_SUCCESS;
		VkPresentInfoKHR present_info {};
		present_info.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext					= nullptr;
		present_info.waitSemaphoreCount		= 1;
		present_info.pWaitSemaphores		= &submit_to_present_semaphores[ next_image ];
		present_info.swapchainCount			= 1;
		present_info.pSwapchains			= &swapchain;
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
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot present render results to window!" );
				return false;
			}
		}
	}

	previous_image						= next_image;
	previous_frame_need_synchronization	= true;
	previous_pipeline_settings			= {};
	previous_sampler					= nullptr;
	previous_texture					= nullptr;

	glfwPollEvents();

	return true;
}

void vk2d::_internal::WindowImpl::UpdateEvents()
{
	glfwPollEvents();
}

void vk2d::_internal::WindowImpl::TakeScreenshot(
	const std::filesystem::path		&	save_path,
	bool								include_alpha
)
{
	if( screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::IDLE ) {
		screenshot_path		= save_path;
		screenshot_state	= vk2d::_internal::WindowImpl::ScreenshotState::REQUESTED;
		screenshot_alpha	= include_alpha;
	}
}

void vk2d::_internal::WindowImpl::Focus()
{
	glfwFocusWindow( glfw_window );
}

void vk2d::_internal::WindowImpl::SetOpacity( float opacity )
{
	glfwSetWindowOpacity( glfw_window, opacity );
}

float vk2d::_internal::WindowImpl::GetOpacity()
{
	return glfwGetWindowOpacity( glfw_window );
}

void vk2d::_internal::WindowImpl::Hide( bool hidden )
{
	if( hidden ) {
		glfwHideWindow( glfw_window );
	} else {
		glfwShowWindow( glfw_window );
	}
}

bool vk2d::_internal::WindowImpl::IsHidden()
{
	return !glfwGetWindowAttrib( glfw_window, GLFW_VISIBLE );
}

void vk2d::_internal::WindowImpl::DisableEvents(
	bool		disable_events
)
{
	if( disable_events ) {
		event_handler	= nullptr;
	} else {
		event_handler	= create_info_copy.event_handler;
	}
}

bool vk2d::_internal::WindowImpl::AreEventsDisabled()
{
	return !!event_handler;
}

void vk2d::_internal::WindowImpl::SetFullscreen(
	vk2d::Monitor		*	monitor,
	uint32_t				frequency )
{
	if( monitor ) {
		glfwSetWindowMonitor( glfw_window, monitor->impl->monitor, 0, 0, extent.width, extent.height, frequency );
		if( !glfwGetWindowMonitor( glfw_window ) ) {
			glfwSetWindowMonitor( glfw_window, nullptr, position.x, position.y, extent.width, extent.height, INT32_MAX );
		}
	} else {
		glfwSetWindowMonitor( glfw_window, nullptr, position.x, position.y, extent.width, extent.height, INT32_MAX );
	}
}

bool vk2d::_internal::WindowImpl::IsFullscreen()
{
	return !!glfwGetWindowMonitor( glfw_window );
}

vk2d::Vector2d vk2d::_internal::WindowImpl::GetCursorPosition()
{
	double x = 0, y = 0;
	glfwGetCursorPos( glfw_window, &x, &y );
	return { x, y };
}

void vk2d::_internal::WindowImpl::SetCursorPosition(
	vk2d::Vector2d		new_position
)
{
	glfwSetCursorPos( glfw_window, new_position.x, new_position.y );
}

void vk2d::_internal::WindowImpl::SetCursor( vk2d::Cursor * cursor )
{
	if( cursor ) {
		glfwSetCursor( glfw_window, cursor->impl->cursor );
	} else {
		glfwSetCursor( glfw_window, nullptr );
	}
}

std::string vk2d::_internal::WindowImpl::GetClipboardString()
{
	return glfwGetClipboardString( glfw_window );
}

void vk2d::_internal::WindowImpl::SetClipboardString( const std::string & str )
{
	glfwSetClipboardString( glfw_window, str.c_str() );
}

void vk2d::_internal::WindowImpl::SetTitle(
	const std::string		&	title
)
{
	window_title	= title;
	glfwSetWindowTitle( glfw_window, window_title.c_str() );
}

std::string vk2d::_internal::WindowImpl::GetTitle()
{
	return window_title;
}

void vk2d::_internal::WindowImpl::SetIcon(
	const std::vector<std::filesystem::path>	&	image_paths
)
{
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

void vk2d::_internal::WindowImpl::SetPosition(
	vk2d::Vector2i		new_position
)
{
	glfwSetWindowPos( glfw_window, new_position.x, new_position.y );
}

vk2d::Vector2i vk2d::_internal::WindowImpl::GetPosition()
{
	int x = 0, y = 0;
	glfwGetWindowPos( glfw_window, &x, &y );
	return { int32_t( x ), int32_t( y ) };
}

void vk2d::_internal::WindowImpl::SetSize(
	vk2d::Vector2u			new_size
)
{
	extent					= { uint32_t( new_size.x ), uint32_t( new_size.y ) };
	should_reconstruct		= true;
}

vk2d::Vector2u vk2d::_internal::WindowImpl::GetSize()
{
	return { extent.width, extent.height };
}

void vk2d::_internal::WindowImpl::Iconify(
	bool			iconified
)
{
	if( iconified ) {
		glfwIconifyWindow( glfw_window );
	} else {
		glfwRestoreWindow( glfw_window );
	}
}

bool vk2d::_internal::WindowImpl::IsIconified()
{
	return glfwGetWindowAttrib( glfw_window, GLFW_ICONIFIED );
}

void vk2d::_internal::WindowImpl::SetMaximized(
	bool			maximized
)
{
	if( maximized ) {
		glfwMaximizeWindow( glfw_window );
	} else {
		glfwRestoreWindow( glfw_window );
	}
}

bool vk2d::_internal::WindowImpl::GetMaximized()
{
	return glfwGetWindowAttrib( glfw_window, GLFW_MAXIMIZED );
}

void vk2d::_internal::WindowImpl::SetCursorState(
	vk2d::CursorState		new_state
)
{
	switch( new_state ) {
	case vk2d::CursorState::NORMAL:
		glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case vk2d::CursorState::HIDDEN:
		glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
		break;
	case vk2d::CursorState::LOCKED:
		glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		break;
	default:
		renderer_parent->Report(
			vk2d::ReportSeverity::WARNING,
			"Parameter error: 'Window::SetCursorState()' -> parameter: 'new_state' was not any value in\n"
			"'vk2d::CursorState', defaulting to 'vk2d::CursorState::NORMAL'"
		);
		glfwSetInputMode( glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	}
}

vk2d::CursorState vk2d::_internal::WindowImpl::GetCursorState()
{
	auto state = glfwGetInputMode( glfw_window, GLFW_CURSOR );
	switch( state ) {
	case GLFW_CURSOR_NORMAL:
		return vk2d::CursorState::NORMAL;
	case GLFW_CURSOR_HIDDEN:
		return vk2d::CursorState::HIDDEN;
	case GLFW_CURSOR_DISABLED:
		return vk2d::CursorState::LOCKED;
	default:
		assert( 0 && "Should not happen" );
		return vk2d::CursorState::NORMAL;
	}
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// WindowImpl::Draw_TriangleList()
// 
// - Records commands to bind a pipeline, draw triangle list to the swapchain image
// - Pushes vertices and indices to mesh buffer that will get uploaded to the GPU prior
//   to submitting the command buffer that these drawing commands were recorded to.
// 
// - Returns:		void
// - Parameters:
//   filled			=: true -> polygons will be filled, false -> polygons will be rendered as wireframe
//   vertices		=: A vector of <Vertex> defining the end points of the polygons
//   indices		=: A vector of <VertexIndex_3> defining the surface of the polygon, each VertexIndex_3 defines a single polygon,
//						VertexIndex_3 contains an array of 3 uint32_t, these correspond to the index number in the vertices vector,
//						of between which the polygon will be drawn.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::_internal::WindowImpl::DrawTriangleList(
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	bool										filled,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	auto index_count	= uint32_t( indices.size() * 3 );
	std::vector<uint32_t> raw_indices;
	raw_indices.resize( index_count );
	for( size_t i = 0, a = 0; i < indices.size(); ++i, a += 3 ) {
		raw_indices[ a + 0 ] = indices[ i ].indices[ 0 ];
		raw_indices[ a + 1 ] = indices[ i ].indices[ 1 ];
		raw_indices[ a + 2 ] = indices[ i ].indices[ 2 ];
	}

	DrawTriangleList(
		raw_indices,
		vertices,
		texture_channels,
		filled,
		texture,
		sampler
	);
}

void vk2d::_internal::WindowImpl::DrawTriangleList(
	const std::vector<uint32_t>				&	raw_indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	bool										filled,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( raw_indices.size() );

	if( !texture ) {
		texture = renderer_parent->GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = renderer_parent->GetDefaultSampler();
	}

	{
		bool multitextured = texture->GetLayerCount() > 1 &&
			texture_channel_weights.size() >= texture->GetLayerCount() * vertices.size();

		auto shader_programs = renderer_parent->GetCompatibleShaderModules(
			multitextured,
			sampler->impl->IsAnyBorderColorEnabled(),
			3
		);

		vk2d::_internal::PipelineSettings pipeline_settings {};
		pipeline_settings.render_pass			= render_pass;
		pipeline_settings.primitive_topology	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline_settings.polygon_mode			= filled ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
		pipeline_settings.shader_programs		= shader_programs;
		pipeline_settings.samples				= VkSampleCountFlags( create_info_copy.samples );

		CmdBindPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdBindTextureSamplerIfDifferent(
		command_buffer,
		sampler,
		texture
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		vertices,
		texture_channel_weights );

	{
		PushConstants pc {};
		pc.index_offset				= push_result.location_info.index_offset;
		pc.index_count				= 3;
		pc.vertex_offset			= push_result.location_info.vertex_offset;
		pc.texture_channel_offset	= push_result.location_info.texture_channel_offset;
		pc.texture_channel_count	= texture->impl->GetLayerCount();

		vkCmdPushConstants(
			command_buffer,
			renderer_parent->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof( pc ),
			&pc
		);
	}

	if( push_result.success ) {
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::BEGIN_DRAW
		);
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			1,
			push_result.location_info.index_offset,
			int32_t( push_result.location_info.vertex_offset ),
			0
		);
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::END_DRAW
		);
	} else {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
}

void vk2d::_internal::WindowImpl::DrawLineList(
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler,
	float										line_width
)
{
	auto index_count	= uint32_t( indices.size() * 2 );
	std::vector<uint32_t> raw_indices;
	raw_indices.resize( index_count );
	for( size_t i = 0, a = 0; i < indices.size(); ++i, a += 2 ) {
		raw_indices[ a + 0 ] = indices[ i ].indices[ 0 ];
		raw_indices[ a + 1 ] = indices[ i ].indices[ 1 ];
	}

	DrawLineList(
		raw_indices,
		vertices,
		texture_channels,
		texture,
		sampler,
		line_width
	);
}

void vk2d::_internal::WindowImpl::DrawLineList(
	const std::vector<uint32_t>				&	raw_indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler,
	float										line_width
)
{
	/*
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( raw_indices.size() );

	CmdBindPipelineIfDifferent(
		command_buffer,
		_internal::PipelineType::LINE_LIST
	);

	CmdBindTextureIfDifferent(
		command_buffer,
		texture
	);

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler
	);

	auto result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		vertices,
		{} );

	CmdSetLineWidthIfDifferent(
		command_buffer,
		line_width
	);

	if( result.success ) {
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			1,
			result.offsets.first_index,
			int32_t( result.offsets.vertex_offset ),
			0
		);
	} else {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
	*/
}

void vk2d::_internal::WindowImpl::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	/*
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );

	CmdBindPipelineIfDifferent(
		command_buffer,
		_internal::PipelineType::POINT_LIST
	);

	CmdBindTextureIfDifferent(
		command_buffer,
		texture
	);

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler
	);

	auto result = mesh_buffer->CmdPushMesh(
		command_buffer,
		vertices,
		{} );

	if( result.success ) {
		vkCmdDraw(
			command_buffer,
			vertex_count,
			1,
			result.offsets.vertex_offset,
			0
		);
	} else {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
	*/
}

void vk2d::_internal::WindowImpl::DrawLine(
	vk2d::Vector2f					point_1,
	vk2d::Vector2f					point_2,
	vk2d::Colorf					color )
{
	/*
	std::vector<vk2d::Vertex>		vertices( 2 );
	std::vector<VertexIndex_2>		indices( 1 );

	vertices[ 0 ].vertex_coords		= point_1;
	vertices[ 0 ].uv_coords			= {};
	vertices[ 0 ].color				= color;

	vertices[ 1 ].vertex_coords		= point_2;
	vertices[ 1 ].uv_coords			= {};
	vertices[ 1 ].color				= color;

	indices[ 0 ].indices			= { 0, 1 };

	DrawLineList(
		vertices,
		indices
	);
	*/
}

void vk2d::_internal::WindowImpl::DrawBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	vk2d::Colorf					color )
{
	auto mesh = vk2d::GenerateBoxMesh(
		top_left,
		bottom_right,
		filled );
	mesh.SetVertexColor( color );
	DrawMesh( mesh );
}

void vk2d::_internal::WindowImpl::DrawCircle(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateCircleMesh(
		top_left,
		bottom_right,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	DrawMesh( mesh );
}

void vk2d::_internal::WindowImpl::DrawPie(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GeneratePieMesh(
		top_left,
		bottom_right,
		begin_angle_radians,
		coverage,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	DrawMesh( mesh );
}

void vk2d::_internal::WindowImpl::DrawPieBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GeneratePieBoxMesh(
		top_left,
		bottom_right,
		begin_angle_radians,
		coverage,
		filled
	);
	mesh.SetVertexColor( color );
	DrawMesh( mesh );
}

void vk2d::_internal::WindowImpl::DrawTexture(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	vk2d::TextureResource		*	texture,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateBoxMesh(
		top_left,
		bottom_right,
		true
	);
	mesh.SetTexture( texture );
	mesh.SetVertexColor( color );
	DrawMesh( mesh );
}

void vk2d::_internal::WindowImpl::DrawMesh(
	const vk2d::Mesh		&	mesh )
{
	if( mesh.vertices.size() == 0 ) return;

	switch( mesh.mesh_type ) {
	case vk2d::MeshType::TRIANGLE_FILLED:
		DrawTriangleList(
			mesh.indices,
			mesh.vertices,
			mesh.texture_channel_weights,
			true,
			mesh.texture,
			mesh.sampler
		);
		break;
	case vk2d::MeshType::TRIANGLE_WIREFRAME:
		DrawTriangleList(
			mesh.indices,
			mesh.vertices,
			mesh.texture_channel_weights,
			false,
			mesh.texture,
			mesh.sampler
		);
		break;
	case vk2d::MeshType::LINE:
		DrawLineList(
			mesh.indices,
			mesh.vertices,
			mesh.texture_channel_weights,
			mesh.texture,
			mesh.sampler,
			mesh.line_width
		);
		break;
	case vk2d::MeshType::POINT:
		DrawPointList(
			mesh.vertices,
			mesh.texture_channel_weights,
			mesh.texture,
			mesh.sampler
		);
		break;
	default:
		break;
	}
}






namespace vk2d {
namespace _internal {

class ScreenshotSaverTask : public vk2d::_internal::Task {
public:
	ScreenshotSaverTask(
		vk2d::_internal::WindowImpl		*	window
	) :
		window( window )
	{}

	void											operator()(
		vk2d::_internal::ThreadPrivateResource	*	thread_resource )
	{
		assert( window->screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE );

		auto path				= window->screenshot_path;
		auto pixel_count		= VkDeviceSize( window->extent.width ) * VkDeviceSize( window->extent.height );

		// Try to map the memory for screenshot buffer data.
		auto pixel_rgba_data = window->screenshot_buffer.memory.Map<uint8_t>();
		if( !pixel_rgba_data ) {
			window->renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot save screenshot, cannot map screenshot buffer memory!" );
			window->screenshot_event_error			= true;
			window->screenshot_event_message	= "Internal error: Cannot map buffer data.";
			window->screenshot_state				= vk2d::_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT;
			window->screenshot_being_saved			= false;
			return;
		}
		auto screenshot_data	= pixel_rgba_data;
		int pixel_channels		= 4;

		// Prepare for if we don't want to save the alpha channel.
		std::vector<uint8_t> pixel_rgb_data;
		if( !window->screenshot_alpha ) {
			pixel_rgb_data.resize( pixel_count * 3 );
			for( VkDeviceSize i = 0; i < pixel_count; ++i ) {
				auto pixel_rgba_offset		= 4 * i;
				auto pixel_rgb_offset		= 3 * i;
				pixel_rgb_data[ pixel_rgb_offset + 0 ]	= pixel_rgba_data[ pixel_rgba_offset + 0 ];
				pixel_rgb_data[ pixel_rgb_offset + 1 ]	= pixel_rgba_data[ pixel_rgba_offset + 1 ];
				pixel_rgb_data[ pixel_rgb_offset + 2 ]	= pixel_rgba_data[ pixel_rgba_offset + 2 ];
			}
			screenshot_data		= pixel_rgb_data.data();
			pixel_channels		= 3;
		}

		int stbi_write_success	= 0;
		auto extension			= path.extension();
		if( extension == ".png" ) {
			stbi_write_success = stbi_write_png(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data,
				0
			);
		} else if( extension == ".bmp" ) {
			stbi_write_success = stbi_write_bmp(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data
			);
		} else if( extension == ".tga" ) {
			stbi_write_success = stbi_write_tga(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data
			);
		} else if( extension == ".jpg" ) {
			stbi_write_success = stbi_write_jpg(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data,
				90
			);
		} else if( extension == ".jpeg" ) {
			stbi_write_success = stbi_write_jpg(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data,
				90
			);
		} else {
			window->renderer_parent->Report( vk2d::ReportSeverity::INFO, "Screenshot extension was not known, saving screenshot as .png" );
			path += ".png";
			stbi_write_success = stbi_write_png(
				path.string().c_str(),
				int( window->extent.width ),
				int( window->extent.height ),
				pixel_channels,
				screenshot_data,
				0
			);
		}
		window->screenshot_buffer.memory.Unmap();

		if( stbi_write_success ) {
			window->screenshot_event_error			= false;
			window->screenshot_event_message		= std::string( "Screenshot successfully saved at: " ) + path.string();
		} else {
			window->renderer_parent->Report( vk2d::ReportSeverity::WARNING, std::string( "Cannot save screenshot: '" ) + path.string() + "'" );
			window->screenshot_event_error			= true;
			window->screenshot_event_message		= std::string( "Cannot save screenshot '" ) + path.string() + "'";
		}

		window->screenshot_state					= vk2d::_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT;
		window->screenshot_being_saved				= false;
	}

	vk2d::_internal::WindowImpl			*	window			= {};
	std::filesystem::path					path			= {};
};

} // _internal
} // vk2d



bool vk2d::_internal::WindowImpl::SynchronizeFrame()
{
	if( previous_frame_need_synchronization ) {
		if( vkWaitForFences(
			device,
			1, &gpu_to_cpu_frame_fences[ previous_image ],
			VK_TRUE,
			UINT64_MAX
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot properly synchronize frame." );
			return false;
		}
		if( vkResetFences(
			device,
			1, &gpu_to_cpu_frame_fences[ previous_image ]
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot properly synchronize frame." );
			return false;
		}

		{
			if( screenshot_state			== vk2d::_internal::WindowImpl::ScreenshotState::WAITING_RENDER &&
				screenshot_swapchain_id		== previous_image ) {

				screenshot_state			= vk2d::_internal::WindowImpl::ScreenshotState::WAITING_FILE_WRITE;
				screenshot_being_saved		= true;

				renderer_parent->GetThreadPool()->ScheduleTask(
					std::make_unique<vk2d::_internal::ScreenshotSaverTask>( this ),
					renderer_parent->GetGeneralThreads() );
			}

			if( screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT ) {
				HandleScreenshotEvent();
			}
		}

		// And we also don't need to synchronize later.
		previous_frame_need_synchronization	= false;
	}

	return true;
}

bool vk2d::_internal::WindowImpl::IsGood()
{
	return is_good;
}









bool vk2d::_internal::WindowImpl::RecreateWindowSizeDependantResources( )
{
	renderer_parent->Report( vk2d::ReportSeverity::INFO, "Begin recreating window resources." );

	if( !ReCreateSwapchain() ) return false;
	ReCreateScreenshotResources();

	// Reallocate framebuffers
	{
		if( framebuffers.size() ) {
			for( auto m : multisample_render_targets ) {
				renderer_parent->GetDeviceMemoryPool()->FreeCompleteResource( m );
			}
			for( auto fb : framebuffers ) {
				vkDestroyFramebuffer(
					device,
					fb,
					nullptr
				);
			}
		}
		if( !CreateFramebuffers() ) return false;
	}

	// Reallocate command buffers
	if( render_command_buffers.size() != swapchain_image_count ) {
		if( render_command_buffers.size() ) {
			vkFreeCommandBuffers(
				device,
				command_pool,
				uint32_t( render_command_buffers.size() ),
				render_command_buffers.data()
			);
		}
		if( !AllocateCommandBuffers() ) return false;
	}

	if( submit_to_present_semaphores.size() != swapchain_image_count ||
		gpu_to_cpu_frame_fences.size() != swapchain_image_count ) {

		// Recreate synchronization semaphores
		if( submit_to_present_semaphores.size() ) {
			for( auto s : submit_to_present_semaphores ) {
				vkDestroySemaphore(
					device,
					s,
					nullptr
				);
			}
		}

		// Recreate synchronization fences
		if( gpu_to_cpu_frame_fences.size() ) {
			vkResetFences( device, uint32_t( gpu_to_cpu_frame_fences.size() ), gpu_to_cpu_frame_fences.data() );
			for( auto s : gpu_to_cpu_frame_fences ) {
				vkDestroyFence(
					device,
					s,
					nullptr
				);
			}
		}

		if( !CreateFrameSynchronizationPrimitives() ) return false;
	}

	should_reconstruct		= false;

	renderer_parent->Report( vk2d::ReportSeverity::INFO, "Done recreating window resources." );

	return true;
}









bool vk2d::_internal::WindowImpl::CreateGLFWWindow()
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
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create glfw window!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::WindowImpl::CreateSurface()
{
	{
		auto result = glfwCreateWindowSurface(
			instance,
			glfw_window,
			nullptr,
			&surface
		);
		if( result != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan surface!" );
			return false;
		}

		VkBool32 surface_supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physical_device,
			primary_render_queue.GetQueueFamilyIndex(),
			surface,
			&surface_supported
		);
		if( !surface_supported ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Vulkan surface does not support presentation using primary render queue, cannot continue!" );
			return false;
		}
	}

	if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
		surface,
		&surface_capabilities
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot get physical device surface capabilities, cannot continue!" );
		return false;
	}

	// Check if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is supported
	{
		VkImageUsageFlags required_image_support = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if( !( ( surface_capabilities.supportedUsageFlags & required_image_support ) == required_image_support ) ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Window Vulkan surface does not support required image capabilities, cannot continue!" );
			return false;
		}
	}

	// Figure out surface format
	{
		std::vector<VkSurfaceFormatKHR> surface_formats;
		{
			uint32_t surface_format_count = 0;
			if( vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device,
				surface,
				&surface_format_count,
				nullptr
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query physical device surface formats, cannot continue!" );
				return false;
			}
			surface_formats.resize( surface_format_count );
			if( vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device,
				surface,
				&surface_format_count,
				surface_formats.data()
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query physical device surface formats, cannot continue!" );
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









bool vk2d::_internal::WindowImpl::CreateRenderPass()
{
	bool use_multisampling	= samples != vk2d::Multisamples::SAMPLE_COUNT_1;

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
	subpasses[ 0 ].pResolveAttachments			= samples == vk2d::Multisamples::SAMPLE_COUNT_1 ? nullptr : resolve_attachment_references.data();
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

	if( vkCreateRenderPass(
		device,
		&render_pass_create_info,
		nullptr,
		&render_pass
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan render pass!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::WindowImpl::CreateCommandPool()
{

	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext				= nullptr;
	command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex	= primary_render_queue.GetQueueFamilyIndex();

	if( vkCreateCommandPool(
		device,
		&command_pool_create_info,
		nullptr,
		&command_pool
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create window Vulkan command pool!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::WindowImpl::AllocateCommandBuffers()
{
	render_command_buffers.resize( swapchain_image_count );
	std::vector<VkCommandBuffer> temp( swapchain_image_count + 1 );

	VkCommandBufferAllocateInfo command_buffer_allocate_info {};
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext				= nullptr;
	command_buffer_allocate_info.commandPool		= command_pool;
	command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount	= uint32_t( temp.size() );

	if( vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		temp.data()
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot allocate window Vulkan command buffers!" );
		return false;
	}

	for( size_t i = 0; i < swapchain_image_count; ++i ) {
		render_command_buffers[ i ]		= temp[ i ];
	}
	complementary_transfer_command_buffer		= temp[ swapchain_image_count ];

	return true;
}









bool vk2d::_internal::WindowImpl::ReCreateSwapchain()
{
	if( !SynchronizeFrame() ) return false;

	auto old_swapchain		= swapchain;

	// Create swapchain
	{
		// Figure out image count
		{
			if( create_info_copy.vsync ) {
				swapchain_image_count = 2;	// Vsync enabled, we only need 2 swapchain images
			} else {
				swapchain_image_count = 3;	// Vsync disabled, we should use at least 3 images
			}
			if( surface_capabilities.maxImageCount != 0 ) {
				if( swapchain_image_count > surface_capabilities.maxImageCount ) {
					swapchain_image_count = surface_capabilities.maxImageCount;
				}
			}
			if( swapchain_image_count < surface_capabilities.minImageCount ) {
				swapchain_image_count = surface_capabilities.minImageCount;
			}
		}

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
			if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physical_device,
				surface,
				&surface_capabilities
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query physical device surface capabilities, cannot continue!" );
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
					if( vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						surface,
						&present_mode_count,
						nullptr
					) != VK_SUCCESS ) {
						renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query physical device surface present modes!" );
						return false;
					}
					surface_present_modes.resize( present_mode_count );
					if( vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						surface,
						&present_mode_count,
						surface_present_modes.data()
					) != VK_SUCCESS ) {
						renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query physical device surface present modes!" );
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
			swapchain_create_info.surface					= surface;
			swapchain_create_info.minImageCount				= swapchain_image_count;
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
			swapchain_create_info.oldSwapchain				= old_swapchain;

			auto result = vkCreateSwapchainKHR(
				device,
				&swapchain_create_info,
				nullptr,
				&swapchain
			);
			if( result != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan swapchain!" );
				return false;
			}
		}

		// Get swapchain images and create image views
		{
			uint32_t swapchain_image_count = 0;
			if( vkGetSwapchainImagesKHR(
				device,
				swapchain,
				&swapchain_image_count,
				nullptr
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query Vulkan swapchain images!" );
				return false;
			}
			swapchain_images.resize( swapchain_image_count );
			if( vkGetSwapchainImagesKHR(
				device,
				swapchain,
				&swapchain_image_count,
				swapchain_images.data()
			) != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot query Vulkan swapchain images!" );
				return false;
			}

			// Destroy old swapchain image views if they exist
			for( auto v : swapchain_image_views ) {
				vkDestroyImageView(
					device,
					v,
					nullptr
				);
			}

			swapchain_image_views.resize( swapchain_image_count );
			for( size_t i = 0; i < swapchain_image_count; ++i ) {

				VkImageViewCreateInfo image_view_create_info {};
				image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_create_info.pNext				= nullptr;
				image_view_create_info.flags				= 0;
				image_view_create_info.image				= swapchain_images[ i ];
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

				auto result = vkCreateImageView(
					device,
					&image_view_create_info,
					nullptr,
					&swapchain_image_views[ i ]
				);
				if( result != VK_SUCCESS ) {
					renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan swapchain image views!" );
					return false;
				}
			}
		}
	}

	// Destroy old swapchain if it exists
	{
		vkDestroySwapchainKHR(
			device,
			old_swapchain,
			nullptr
		);
	}

	should_reconstruct		= false;

	return true;
}

bool vk2d::_internal::WindowImpl::ReCreateScreenshotResources()
{
	while( screenshot_being_saved ) {
		std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
	}

	auto memory_pool	= renderer_parent->GetDeviceMemoryPool();
	assert( memory_pool );

	memory_pool->FreeCompleteResource( screenshot_buffer );
	memory_pool->FreeCompleteResource( screenshot_image );

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
	screenshot_image = renderer_parent->GetDeviceMemoryPool()->CreateCompleteImageResource(
		&image_create_info,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	if( screenshot_image != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create internal screenshot image, screenshots disabled!" );
		screenshot_state	= vk2d::_internal::WindowImpl::ScreenshotState::IDLE_ERROR;
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
	screenshot_buffer = renderer_parent->GetDeviceMemoryPool()->CreateCompleteBufferResource(
		&buffer_create_info,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);
	if( screenshot_buffer != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create internal screenshot buffer, screenshots disabled!" );
		screenshot_state	= vk2d::_internal::WindowImpl::ScreenshotState::IDLE_ERROR;
		return false;
	}

	return true;
}









bool vk2d::_internal::WindowImpl::CreateFramebuffers()
{
	framebuffers.resize( swapchain_image_count );

	bool use_multisampling		= samples != vk2d::Multisamples::SAMPLE_COUNT_1;
	if( use_multisampling ) {
		multisample_render_targets.resize( swapchain_image_count );
	}

	for( uint32_t i = 0; i < swapchain_image_count; ++i ) {
		if( use_multisampling ) {
			VkImageCreateInfo						image_create_info		{};
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

			VkImageViewCreateInfo					image_view_create_info	{};
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

			multisample_render_targets[ i ] = renderer_parent->GetDeviceMemoryPool()->CreateCompleteImageResource(
				&image_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&image_view_create_info
			);
			if( multisample_render_targets[ i ] != VK_SUCCESS ) {
				renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create multisample render targets!" );
				return false;
			}
		}

		std::vector<VkImageView>	attachments;
		if( use_multisampling ) {
			attachments.push_back( multisample_render_targets[ i ].view );
		}
		attachments.push_back( swapchain_image_views[ i ] );

		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext				= nullptr;
		framebuffer_create_info.flags				= 0;
		framebuffer_create_info.renderPass			= render_pass;
		framebuffer_create_info.attachmentCount		= uint32_t( attachments.size() );
		framebuffer_create_info.pAttachments		= attachments.data();
		framebuffer_create_info.width				= extent.width;
		framebuffer_create_info.height				= extent.height;
		framebuffer_create_info.layers				= 1;

		if( vkCreateFramebuffer(
			device,
			&framebuffer_create_info,
			nullptr,
			&framebuffers[ i ]
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create Vulkan framebuffers!" );
			return false;
		}
	}

	return true;
}









bool vk2d::_internal::WindowImpl::CreateWindowSynchronizationPrimitives()
{
	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;
	if( vkCreateFence(
		device,
		&fence_create_info,
		nullptr,
		&aquire_image_fence
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create image aquisition fence!" );
		return false;
	}

	VkSemaphoreCreateInfo mesh_transfer_semaphore_create_info {};
	mesh_transfer_semaphore_create_info.sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	mesh_transfer_semaphore_create_info.pNext	= nullptr;
	mesh_transfer_semaphore_create_info.flags	= 0;

	if( vkCreateSemaphore(
		device,
		&mesh_transfer_semaphore_create_info,
		nullptr,
		&mesh_transfer_semaphore
	) != VK_SUCCESS ) {
		renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create mesh transfer semaphore!" );
		return false;
	}

	return true;
}









bool vk2d::_internal::WindowImpl::CreateFrameSynchronizationPrimitives()
{
	submit_to_present_semaphores.resize( swapchain_image_count );

	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext		= nullptr;
	semaphore_create_info.flags		= 0;

	for( auto & s : submit_to_present_semaphores ) {
		if( vkCreateSemaphore(
			device,
			&semaphore_create_info,
			nullptr,
			&s
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create frame synchronization semaphores!" );
			return false;
		}
	}

	gpu_to_cpu_frame_fences.resize( swapchain_image_count );

	VkFenceCreateInfo			fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;

	for( auto & f : gpu_to_cpu_frame_fences ) {
		if( vkCreateFence(
			device,
			&fence_create_info,
			nullptr,
			&f
		) != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create frame synchronization fences!" );
			return false;
		}
	}

	return true;
}

bool vk2d::_internal::WindowImpl::CreateWindowFrameDataBuffer()
{
	// Create staging and device buffers
	{
		VkBufferCreateInfo staging_buffer_create_info {};
		staging_buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_buffer_create_info.pNext					= nullptr;
		staging_buffer_create_info.flags					= 0;
		staging_buffer_create_info.size						= sizeof( vk2d::_internal::WindowFrameData );
		staging_buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		staging_buffer_create_info.queueFamilyIndexCount	= 0;
		staging_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_staging_buffer = renderer_parent->GetDeviceMemoryPool()->CreateCompleteBufferResource(
			&staging_buffer_create_info,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		if( frame_data_staging_buffer != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error. Cannot create staging buffer for WindowFrameData!" );
			return false;
		}

		VkBufferCreateInfo device_buffer_create_info {};
		device_buffer_create_info.sType						= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		device_buffer_create_info.pNext						= nullptr;
		device_buffer_create_info.flags						= 0;
		device_buffer_create_info.size						= sizeof( vk2d::_internal::WindowFrameData );
		device_buffer_create_info.usage						= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		device_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		device_buffer_create_info.queueFamilyIndexCount		= 0;
		device_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_device_buffer = renderer_parent->GetDeviceMemoryPool()->CreateCompleteBufferResource(
			&device_buffer_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if( frame_data_device_buffer != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error. Cannot create device local buffer for WindowFrameData!" );
			return false;
		}
	}

	// Create descriptor set
	{
		frame_data_descriptor_set	= renderer_parent->GetDescriptorPool()->AllocateDescriptorSet(
			renderer_parent->GetUniformBufferDescriptorSetLayout()
		);
		if( frame_data_descriptor_set != VK_SUCCESS ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot allocate descriptor set for WindowFrameData device buffer!" );
			return false;
		}
		VkDescriptorBufferInfo descriptor_write_buffer_info {};
		descriptor_write_buffer_info.buffer	= frame_data_device_buffer.buffer;
		descriptor_write_buffer_info.offset	= 0;
		descriptor_write_buffer_info.range	= sizeof( vk2d::_internal::WindowFrameData );
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
			device,
			1, &descriptor_write,
			0, nullptr
		);
	}

	return true;
}

void vk2d::_internal::WindowImpl::HandleScreenshotEvent()
{
	assert( screenshot_state == vk2d::_internal::WindowImpl::ScreenshotState::WAITING_EVENT_REPORT );

	if( event_handler ) {
		event_handler->EventScreenshot(
			window_parent,
			screenshot_path,
			!screenshot_event_error,
			screenshot_event_message
		);
	}

	screenshot_path					= "";
	screenshot_event_error			= false;
	screenshot_event_message		= "";
	screenshot_state				= vk2d::_internal::WindowImpl::ScreenshotState::IDLE;
}

void vk2d::_internal::WindowImpl::CmdBindPipelineIfDifferent(
	VkCommandBuffer									command_buffer,
	const vk2d::_internal::PipelineSettings		&	pipeline_settings
)
{
	if( previous_pipeline_settings != pipeline_settings ) {
		auto pipeline = renderer_parent->GetPipeline( pipeline_settings );

		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline
		);
		previous_pipeline_settings	= pipeline_settings;
	}
}

void vk2d::_internal::WindowImpl::CmdBindTextureSamplerIfDifferent(
	VkCommandBuffer						command_buffer,
	vk2d::Sampler					*	sampler,
	vk2d::TextureResource			*	texture )
{
	// If sampler and texture did not change since last call, do nothing.
	if( !sampler ) {
		sampler		= renderer_parent->GetDefaultSampler();
	}
	if( !texture ) {
		texture		= renderer_parent->GetDefaultTexture();
	}

	// if sampler or texture changed since previous call, bind a different descriptor set.
	if( sampler != previous_sampler || texture != previous_texture ) {
		auto & set = sampler_texture_descriptor_sets[ sampler ][ texture ];

		// If this descriptor set doesn't exist yet for this
		// sampler texture combo, create one and update it.
		if( set.descriptor_set.descriptorSet == VK_NULL_HANDLE ) {
			set.descriptor_set = renderer_parent->GetDescriptorPool()->AllocateDescriptorSet(
				renderer_parent->GetSamplerTextureDescriptorSetLayout()
			);

			texture->WaitUntilLoaded();

			VkDescriptorImageInfo image_info {};
			image_info.sampler						= sampler->impl->GetVulkanSampler();
			image_info.imageView					= texture->impl->GetVulkanImageView();
			image_info.imageLayout					= texture->impl->GetVulkanImageLayout();

			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer						= sampler->impl->GetVulkanBufferForSamplerData();
			buffer_info.offset						= 0;
			buffer_info.range						= sizeof( vk2d::_internal::SamplerImpl::BufferData );

			std::array<VkWriteDescriptorSet, 2> descriptor_write {};
			descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[ 0 ].pNext				= nullptr;
			descriptor_write[ 0 ].dstSet			= set.descriptor_set.descriptorSet;
			descriptor_write[ 0 ].dstBinding		= 0;
			descriptor_write[ 0 ].dstArrayElement	= 0;
			descriptor_write[ 0 ].descriptorCount	= 1;
			descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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
				device,
				uint32_t( descriptor_write.size() ), descriptor_write.data(),
				0, nullptr );
		}
		set.previous_access_time = std::chrono::steady_clock::now();

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			renderer_parent->GetPipelineLayout(),
			DESCRIPTOR_SET_ALLOCATION_TEXTURE_AND_SAMPLER,
			1, &set.descriptor_set.descriptorSet,
			0, nullptr
		);

		previous_sampler		= sampler;
		previous_texture		= texture;
	}
}

void vk2d::_internal::WindowImpl::CmdSetLineWidthIfDifferent(
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

bool vk2d::_internal::WindowImpl::CmdUpdateFrameData(
	VkCommandBuffer			command_buffer
)
{
	// Window coordinate system scaling
	vk2d::_internal::WindowCoordinateScaling window_coordinate_scaling {};

	switch( create_info_copy.coordinate_space ) {
	case vk2d::WindowCoordinateSpace::TEXEL_SPACE:
		window_coordinate_scaling.multiplier	= { 1.0f / ( extent.width / 2.0f ), 1.0f / ( extent.height / 2.0f ) };
		window_coordinate_scaling.offset		= { -1.0f, -1.0f };
		break;
	case vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED:
		window_coordinate_scaling.multiplier	= { 1.0f / ( extent.width / 2.0f ), 1.0f / ( extent.height / 2.0f ) };
		window_coordinate_scaling.offset		= { 0.0f, 0.0f };
		break;
	case vk2d::WindowCoordinateSpace::NORMALIZED_SPACE:
	{
		float contained_minimum_dimension		= float( std::min( extent.width, extent.height ) );
		window_coordinate_scaling.multiplier	= { contained_minimum_dimension / ( extent.width / 2.0f ), contained_minimum_dimension / ( extent.height / 2.0f ) };
		window_coordinate_scaling.offset		= { -1.0f, -1.0f };
	}
	break;
	case vk2d::WindowCoordinateSpace::NORMALIZED_SPACE_CENTERED:
	{
		float contained_minimum_dimension		= float( std::min( extent.width, extent.height ) );
		window_coordinate_scaling.multiplier	= { contained_minimum_dimension / extent.width, contained_minimum_dimension / extent.height };
		window_coordinate_scaling.offset		= { 0.0f, 0.0f };
	}
	break;
	case vk2d::WindowCoordinateSpace::NORMALIZED_VULKAN:
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
		auto frame_data = frame_data_staging_buffer.memory.Map<vk2d::_internal::WindowFrameData>();
		if( !frame_data ) {
			renderer_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot map WindowFrameData staging buffer memory!" );
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
		copy_region.size		= sizeof( vk2d::_internal::WindowFrameData );
		vkCmdCopyBuffer(
			command_buffer,
			frame_data_staging_buffer.buffer,
			frame_data_device_buffer.buffer,
			1, &copy_region
		);
	}

	return true;
}







vk2d::_internal::CursorImpl::CursorImpl(
	vk2d::_internal::RendererImpl		*	renderer,
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	renderer_parent		= renderer;
	assert( renderer_parent );

	int x = 0, y = 0, channels = 0;
	auto stbiData = stbi_load( image_path.string().c_str(), &x, &y, &channels, 4 );
	if( stbiData ) {
		std::vector<vk2d::Color8> data( x * y );
		std::memcpy( data.data(), stbiData, data.size() * sizeof( vk2d::Color8 ) );
		free( stbiData );
		*this = vk2d::_internal::CursorImpl(
			renderer,
			{ uint32_t( x ), uint32_t( y ) },
			data,
			hot_spot
		);
	}
}

vk2d::_internal::CursorImpl::CursorImpl(
	vk2d::_internal::RendererImpl		*	renderer,
	vk2d::Vector2u							image_size,
	const std::vector<vk2d::Color8>		&	image_data,
	vk2d::Vector2i							hot_spot
)
{
	renderer_parent		= renderer;
	assert( renderer_parent );

	if( size_t( image_size.x ) * size_t( image_size.y ) != image_data.size() ) {
		std::stringstream ss;
		ss << "Cannot create cursor, image dimensions do not match required texel count.\n"
			<< "Cursor dimensions are: X=" << image_size.x << " Y=" << image_size.y << ".";
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, ss.str() );
		return;
	}

	pixel_data.resize( size_t( image_size.x ) * size_t( image_size.y ) * sizeof( vk2d::Color8 ) );
	std::memcpy( pixel_data.data(), image_data.data(), pixel_data.size() * sizeof( vk2d::Color8 ) );

	GLFWimage glfwImage {};
	glfwImage.width		= image_size.x;
	glfwImage.height	= image_size.y;
	glfwImage.pixels	= (uint8_t*)pixel_data.data();
	cursor				= glfwCreateCursor( &glfwImage, int( hot_spot.x ), int( hot_spot.y ) );
	if( cursor ) {
		hotSpot	= { hot_spot.x, hot_spot.y };
		extent	= { image_size.x, image_size.y };
	} else {
		renderer_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor!" );
		return;
	}

	is_good = true;
}

vk2d::_internal::CursorImpl::CursorImpl(
	const vk2d::_internal::CursorImpl	&	other )
{
	this->~CursorImpl();
	*this	= vk2d::_internal::CursorImpl(
		other.renderer_parent,
		{ other.extent.width, other.extent.height },
		other.pixel_data,
		{ other.hotSpot.x, other.hotSpot.y }
	);
}

vk2d::_internal::CursorImpl::~CursorImpl()
{
	glfwDestroyCursor( cursor );
	cursor			= nullptr;
	hotSpot			= {};
	is_good			= false;
}

vk2d::_internal::CursorImpl & vk2d::_internal::CursorImpl::operator=(
	vk2d::_internal::CursorImpl			&	other )
{
	this->~CursorImpl();
	*this	= vk2d::_internal::CursorImpl(
		other.renderer_parent,
		{ other.extent.width, other.extent.height },
		other.pixel_data,
		{ other.hotSpot.x, other.hotSpot.y }
	);

	return *this;
}

bool vk2d::_internal::CursorImpl::IsGood()
{
	return is_good;
}

vk2d::_internal::RendererImpl * vk2d::_internal::CursorImpl::GetRenderer()
{
	return renderer_parent;
}

const std::vector<vk2d::Color8> & vk2d::_internal::CursorImpl::GetPixelData()
{
	return pixel_data;
}

GLFWcursor * vk2d::_internal::CursorImpl::GetGLFWcursor()
{
	return cursor;
}

vk2d::Vector2u vk2d::_internal::CursorImpl::GetSize()
{
	return { extent.width, extent.height };
}

vk2d::Vector2i vk2d::_internal::CursorImpl::GetHotSpot()
{
	return { hotSpot.x, hotSpot.y };
}






vk2d::_internal::MonitorImpl::MonitorImpl(
	GLFWmonitor									*	monitor,
	VkOffset2D										position,
	VkExtent2D										physical_size,
	std::string										name,
	vk2d::MonitorVideoMode							current_video_mode,
	const std::vector<vk2d::MonitorVideoMode>	&	video_modes )
{
	this->monitor				= monitor;
	this->position				= position;
	this->physical_size			= physical_size;
	this->name					= name;
	this->current_video_mode	= current_video_mode;
	this->video_modes			= video_modes;

	assert( this->monitor );

	is_good						= true;
}

const vk2d::MonitorVideoMode & vk2d::_internal::MonitorImpl::GetCurrentVideoMode() const
{
	return current_video_mode;
}

const std::vector<vk2d::MonitorVideoMode> & vk2d::_internal::MonitorImpl::GetVideoModes() const
{
	return video_modes;
}

void vk2d::_internal::MonitorImpl::SetGamma(
	float		gamma
)
{
	glfwSetGamma(
		monitor,
		gamma
	);
}

vk2d::GammaRamp vk2d::_internal::MonitorImpl::GetGammaRamp()
{
	auto glfwRamp		= glfwGetGammaRamp( monitor );
	vk2d::GammaRamp		ret {};
	ret.count			= glfwRamp->size;
	ret.red.reserve( ret.count );
	ret.green.reserve( ret.count );
	ret.blue.reserve( ret.count );
	for( uint32_t i = 0; i < ret.count; ++i ) {
		ret.red.push_back( glfwRamp->red[ i ] );
		ret.green.push_back( glfwRamp->green[ i ] );
		ret.blue.push_back( glfwRamp->blue[ i ] );
	}
	return ret;
}

void vk2d::_internal::MonitorImpl::SetGammaRamp(
	const vk2d::GammaRamp		&	ramp
)
{
	auto modifiable_ramp = ramp;

	if( ( modifiable_ramp.count != uint32_t( modifiable_ramp.red.size() ) ) ||
		( modifiable_ramp.count != uint32_t( modifiable_ramp.green.size() ) ) ||
		( modifiable_ramp.count != uint32_t( modifiable_ramp.blue.size() ) ) ) {
		return;
	}

	GLFWgammaramp glfwRamp {};
	glfwRamp.size		= modifiable_ramp.count;
	glfwRamp.red		= modifiable_ramp.red.data();
	glfwRamp.green		= modifiable_ramp.green.data();
	glfwRamp.blue		= modifiable_ramp.blue.data();
	glfwSetGammaRamp( monitor, &glfwRamp );
}

bool vk2d::_internal::MonitorImpl::IsGood()
{
	return is_good;
}