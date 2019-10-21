
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/Window.h"
#include "../Header/WindowImpl.h"
#include "../Header/RendererImpl.h"

#include "../../Include/VK2D/Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <array>



namespace vk2d {







VK2D_API					Window::Window(
	Renderer			*	renderer_parent,
	WindowCreateInfo	&	window_create_info )
{
	data	= std::make_unique<vk2d::_internal::WindowDataImpl>();
	if( !data ) {
		if( renderer_parent->data->create_info_copy.report_function ) {
			renderer_parent->data->create_info_copy.report_function( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return;
	}


	data->create_info_copy		= window_create_info;
	data->renderer				= renderer_parent;
	data->report_function		= renderer_parent->data->report_function;

	auto device					= data->renderer->data->device;

	if( !CreateGLFWWindow( data.get() ) ) return;

	if( !CreateSurface(
		data.get(),
		data->renderer->data->instance,
		data->renderer->data->physical_device,
		data->renderer->data->primary_render_queue
	) ) return;

	if( !CreateRenderPass( data.get(), device ) ) return;

	if( !CreateGraphicsPipelines(
		data.get(),
		device,
		data->renderer->data->pipeline_cache,
		data->renderer->data->pipeline_layout,
		data->renderer->data->vertex_shader_module,
		data->renderer->data->fragment_shader_module
	) ) return;

	if( !CreateCommandPool( data.get(), device, data->renderer->data->primary_render_queue ) ) return;
	if( !AllocateCommandBuffers( data.get(), device ) ) return;

	if( !CreateSwapchain(
		data.get(),
		data->renderer->data->physical_device,
		device,
		data->renderer->data->primary_render_queue
	) ) return;

	if( !CreateFramebuffers( data.get(), device ) ) return;
	if( !CreateWindowSynchronizationPrimitives( data.get(), device ) ) return;
	if( !CreateFrameSynchronizationPrimitives( data.get(), device ) ) return;

	// (Re-)Create frame vertex and index lists
	data->frame_vertices.resize( data->swapchain_image_count );
	data->frame_indices.resize( data->swapchain_image_count );

	is_good							= true;
}







VK2D_API Window::~Window()
{
	auto device = data->renderer->data->device;

	vkDeviceWaitIdle( device );

	if( data ) {

		for( auto f : data->gpu_to_cpu_frame_fences ) {
			vkDestroyFence(
				device,
				f,
				nullptr
			);
		}

		for( auto s : data->submit_to_present_semaphores ) {
			vkDestroySemaphore(
				device,
				s,
				nullptr
			);
		}

		vkDestroyFence(
			device,
			data->aquire_image_fence,
			nullptr
		);

		for( auto f : data->framebuffers ) {
			vkDestroyFramebuffer(
				device,
				f,
				nullptr
			);
		}

		for( auto v : data->swapchain_image_views ) {
			vkDestroyImageView(
				device,
				v,
				nullptr
			);
		}

		vkDestroySwapchainKHR(
			device,
			data->swapchain,
			nullptr
		);

		vkDestroyCommandPool(
			device,
			data->command_pool,
			nullptr
		);

		for( auto p : data->pipelines ) {
			vkDestroyPipeline(
				device,
				p,
				nullptr
			);
		}

		vkDestroyRenderPass(
			device,
			data->render_pass,
			nullptr
		);

		vkDestroySurfaceKHR(
			data->renderer->data->instance,
			data->surface,
			nullptr
		);

		glfwDestroyWindow( data->glfw_window );
	}
}







bool									AquireImage(
	_internal::WindowDataImpl		*	data,
	VkPhysicalDevice					physical_device,
	VkDevice							device,
	ResolvedQueue					&	primary_render_queue,
	uint32_t							nested_counter				= 0)
{
	auto result = vkAcquireNextImageKHR(
		device,
		data->swapchain,
		UINT64_MAX,
		VK_NULL_HANDLE,
		data->aquire_image_fence,
		&data->next_image
	);
	if( result != VK_SUCCESS ) {
		if( result == VK_SUBOPTIMAL_KHR ) {
			// Image aquired but is not optimal, continue but recreate swapchain next time we begin the render again.
			data->recreate_swapchain		= true;
		} else if( result == VK_ERROR_OUT_OF_DATE_KHR ) {
			// Image was not aquired so we cannot present anything until we recreate the swapchain.
			if( nested_counter ) {
				// Breaking out of nested call here, we tried aquiring an image twice before
				// now and it didn't work so we can assume it will not work and we can give up here.
				if( data->report_function ) {
					data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot aquire a swapchain image after retry!" );
				}
				return false;
			}
			// Cannot continue before we recreate the swapchain
			if( !RecreateResourcesAfterResizing(
				data,
				physical_device,
				device,
				primary_render_queue
			) ) {
				if( data->report_function ) {
					data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot recreate swapchain!" );
				}
				return false;
			}
			// After recreating the swapchain and resources, try to aquire
			// next image again, if that fails we should stop trying.
			if( !AquireImage(
				data,
				physical_device,
				device,
				primary_render_queue,
				++nested_counter
			) ) {

			}
		} else {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot aquire next image!" );
			}
			return false;
		}
	}
	vkWaitForFences(
		device,
		1, &data->aquire_image_fence,
		VK_TRUE,
		UINT64_MAX
	);
	vkResetFences(
		device,
		1, &data->aquire_image_fence
	);

	return true;
}

VK2D_API bool VK2D_APIENTRY Window::BeginRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( data->next_render_call_function != _internal::NextRenderCallFunction::BEGIN ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::WARNING, "BeginRender() Called twice in a row!" );
		}
		return false;
	} else {
		data->next_render_call_function = _internal::NextRenderCallFunction::END;
	}

	// TODO: check if we need to recreate the swapchain
	if( data->recreate_swapchain ) {
		if( !CreateSwapchain(
			data.get(),
			data->renderer->data->physical_device,
			data->renderer->data->device,
			data->renderer->data->primary_render_queue
		) ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot recreate swapchain!" );
			}
			return false;
		}
	}

	// Make sure we can write to the next command buffer by
	// aquiring a new image from the presentation engine,
	// this will tell which command buffer is ready to be reused
	{
		if( !AquireImage(
			data.get(),
			data->renderer->data->physical_device,
			data->renderer->data->device,
			data->renderer->data->primary_render_queue
		) ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot aquire next swapchain image!" );
			}
			return false;
		}
	}

	// If next image index happens to same as the previous, presentation has propably already succeeded but
	// since we're using the image index as an index to our command buffers and framebuffers we'll have to
	// make sure that we don't start overwriting a command buffer until it's execution has completely
	// finished, so we'll have to synchronize the frame early in here.
	if( data->next_image == data->previous_image ) {
		if( !SynchronizeFrame( data.get(), data->renderer->data->device ) ) return false;
	}

	// Begin command buffer
	{
		VkCommandBuffer		command_buffer			= data->command_buffers[ data->next_image ];

		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext				= nullptr;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo	= nullptr;

		if( vkBeginCommandBuffer(
			command_buffer,
			&command_buffer_begin_info
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot begin recording command buffer!" );
			}
			return false;
		}

		// Set viewport, scissor and initial line width
		{
			VkViewport viewport {};
			viewport.x			= 0;
			viewport.y			= 0;
			viewport.width		= float( data->extent.width );
			viewport.height		= float( data->extent.height );
			viewport.minDepth	= 0.0f;
			viewport.maxDepth	= 1.0f;
			vkCmdSetViewport(
				command_buffer,
				0, 1, &viewport
			);

			VkRect2D scissor {
				{ 0, 0 },
				data->extent.width
			};
			vkCmdSetScissor(
				command_buffer,
				0, 1, &scissor
			);

			vkCmdSetLineWidth(
				command_buffer,
				1.0f
			);
		}

		// Begin render pass
		{
			VkClearValue	clear_value {};
			clear_value.color.float32[ 0 ]		= 25.7f;
			clear_value.color.float32[ 1 ]		= 0.0f;
			clear_value.color.float32[ 2 ]		= 0.0f;
			clear_value.color.float32[ 3 ]		= 0.0f;

			VkRenderPassBeginInfo render_pass_begin_info {};
			render_pass_begin_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext			= nullptr;
			render_pass_begin_info.renderPass		= data->render_pass;
			render_pass_begin_info.framebuffer		= data->framebuffers[ data->next_image ];
			render_pass_begin_info.renderArea		= { { 0, 0 }, data->extent };
			render_pass_begin_info.clearValueCount	= 1;
			render_pass_begin_info.pClearValues		= &clear_value;

			vkCmdBeginRenderPass(
				command_buffer,
				&render_pass_begin_info,
				VK_SUBPASS_CONTENTS_INLINE
			);
		}
	}

	return true;
}







VK2D_API bool VK2D_APIENTRY Window::EndRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( data->next_render_call_function != _internal::NextRenderCallFunction::END ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::WARNING, "EndRender() Called twice in a row!" );
		}
		return false;
	} else {
		data->next_render_call_function = _internal::NextRenderCallFunction::BEGIN;
	}

	VkCommandBuffer		command_buffer	= data->command_buffers[ data->next_image ];

	// End render pass
	{
		vkCmdEndRenderPass( command_buffer );
	}

	// End command buffer
	if( vkEndCommandBuffer( command_buffer ) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot compile command buffer at EndRender()!" );
		}
		return false;
	}

	// Submit swapchain image
	{
		VkSubmitInfo submit_info {};
		submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext					= nullptr;
		submit_info.waitSemaphoreCount		= 0;
		submit_info.pWaitSemaphores			= nullptr;
		submit_info.pWaitDstStageMask		= nullptr;
		submit_info.commandBufferCount		= 1;
		submit_info.pCommandBuffers			= &command_buffer;
		submit_info.signalSemaphoreCount	= 1;
		submit_info.pSignalSemaphores		= &data->submit_to_present_semaphores[ data->next_image ];

		if( vkQueueSubmit(
			data->renderer->data->primary_render_queue.queue,
			1, &submit_info,
			data->gpu_to_cpu_frame_fences[ data->next_image ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot submit command buffer to queue!" );
			}
			return false;
		}
	}

	// Present swapchain image
	{
		VkResult present_result				= VK_SUCCESS;
		VkPresentInfoKHR present_info {};
		present_info.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext					= nullptr;
		present_info.waitSemaphoreCount		= 1;
		present_info.pWaitSemaphores		= &data->submit_to_present_semaphores[ data->next_image ];
		present_info.swapchainCount			= 1;
		present_info.pSwapchains			= &data->swapchain;
		present_info.pImageIndices			= &data->next_image;
		present_info.pResults				= &present_result;
		auto result = vkQueuePresentKHR(
			data->renderer->data->primary_render_queue.queue,
			&present_info
		);
		if( result != VK_SUCCESS || present_result != VK_SUCCESS ) {
			if( result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_ERROR_OUT_OF_DATE_KHR ||
				result == VK_SUBOPTIMAL_KHR || present_result == VK_SUBOPTIMAL_KHR ) {
				data->recreate_swapchain	= true;
			} else {
				if( data->report_function ) {
					data->report_function( ReportSeverity::WARNING, "Cannot present image!" );
				}
				return false;
			}
		}
	}

	// Synchronize the previous frame here, this waits for the previous
	// frame to finish fully rendering before continuing execution.
	if( !SynchronizeFrame( data.get(), data->renderer->data->device ) ) return false;

	data->previous_image						= data->next_image;
	data->previous_frame_need_synchronization	= true;

	glfwPollEvents();

	return true;
}









VK2D_API void VK2D_APIENTRY Window::Draw_TriangleList(
	bool								filled,
	std::vector<Vertex>				&	vertices,
	std::vector<VertexIndex_3>		&	indices )
{
	// TODO: we need a mesh buffer that is able to keep track of changing vertices
	// and indices as well as record command to copy data from host to device.
	todo here;

	auto command_buffer			= data->command_buffers[ data->next_image ];
	auto & frame_mesh_buffer	= data->frame_mesh_buffer[ data->next_image ];

	if( filled ) {
		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			data->pipelines[ uint32_t( _internal::PipelineType::FILLED_POLYGON_LIST ) ]
		);
	}

	vkCmdDrawIndexed(
		data->command_buffers[ data->next_image ],
		uint32_t( indices.size() ),
		1,
		uint32_t( indices.size() + frame_indices.size() ),
		frame_vertices.size(),
		0
	);

	frame_vertices.insert( frame_vertices.end(), vertices.begin(), vertices.end() );

	frame_indices.reserve( frame_vertices.size() + indices.size() * 3 );
	for( auto & i : indices ) {
		frame_indices.push_back( i.indices[ 0 ] );
		frame_indices.push_back( i.indices[ 1 ] );
		frame_indices.push_back( i.indices[ 2 ] );
	}
}









bool RecreateResourcesAfterResizing(
	_internal::WindowDataImpl		*	data,
	VkPhysicalDevice					physical_device,
	VkDevice							device,
	ResolvedQueue						primary_render_queue )
{
	if( !CreateSwapchain(
		data,
		physical_device,
		device,
		primary_render_queue,
		data->swapchain
	) ) return false;

	// Check if any other resources need to be reallocated

	// Reallocate framebuffers
	if( data->framebuffers.size() != data->swapchain_image_count ) {
		if( data->framebuffers.size() ) {
			for( auto fb : data->framebuffers ) {
				vkDestroyFramebuffer(
					device,
					fb,
					nullptr
				);
			}
		}
		if( !CreateFramebuffers( data, device ) ) return false;
	}

	// Reallocate command buffers
	if( data->command_buffers.size() != data->swapchain_image_count ) {
		if( data->command_buffers.size() ) {
			vkFreeCommandBuffers(
				device,
				data->command_pool,
				uint32_t( data->command_buffers.size() ),
				data->command_buffers.data()
			);
		}
		if( !AllocateCommandBuffers( data, device ) ) return false;
	}

	if( data->submit_to_present_semaphores.size()	!= data->swapchain_image_count ||
		data->gpu_to_cpu_frame_fences.size()		!= data->swapchain_image_count ) {

		// Recreate synchronization semaphores
		if( data->submit_to_present_semaphores.size() ) {
			for( auto s : data->submit_to_present_semaphores ) {
				vkDestroySemaphore(
					device,
					s,
					nullptr
				);
			}
		}

		// Recreate synchronization fences
		if( data->gpu_to_cpu_frame_fences.size() ) {
			vkResetFences( device, uint32_t( data->gpu_to_cpu_frame_fences.size() ), data->gpu_to_cpu_frame_fences.data() );
			for( auto s : data->gpu_to_cpu_frame_fences ) {
				vkDestroyFence(
					device,
					s,
					nullptr
				);
			}
		}

		if( !CreateFrameSynchronizationPrimitives( data, device ) ) return false;
	}

	// (Re-)Create frame vertex and index lists
	data->frame_vertices.resize( data->swapchain_image_count );
	data->frame_indices.resize( data->swapchain_image_count );

	data->recreate_swapchain		= false;

	return true;
}









bool									CreateGLFWWindow(
	_internal::WindowDataImpl		*	data
)
{

	glfwWindowHint( GLFW_RESIZABLE, data->create_info_copy.resizeable );
	glfwWindowHint( GLFW_VISIBLE, data->create_info_copy.visible );
	glfwWindowHint( GLFW_DECORATED, data->create_info_copy.decorated );
	glfwWindowHint( GLFW_FOCUSED, data->create_info_copy.focused );
	glfwWindowHint( GLFW_AUTO_ICONIFY, GLFW_FALSE );
	glfwWindowHint( GLFW_MAXIMIZED, data->create_info_copy.maximized );
	glfwWindowHint( GLFW_CENTER_CURSOR, GLFW_TRUE );
	glfwWindowHint( GLFW_TRANSPARENT_FRAMEBUFFER, data->create_info_copy.transparent_framebuffer );
	glfwWindowHint( GLFW_FOCUS_ON_SHOW, GLFW_TRUE );
	glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );

	std::vector<GLFWmonitor*> monitors;
	{
		int monitor_count = 0;
		auto monitors_ptr = glfwGetMonitors( &monitor_count );
		for( int i = 0; i < monitor_count; ++i ) {
			monitors.push_back( monitors_ptr[ i ] );
		}
	}

	GLFWmonitor * fullscreen_monitor = nullptr;
	if( data->create_info_copy.fullscreen_monitor > 0 &&
		data->create_info_copy.fullscreen_monitor <= uint32_t( monitors.size() ) ) {
		fullscreen_monitor		= monitors[ data->create_info_copy.fullscreen_monitor ];
	}

	data->glfw_window = glfwCreateWindow(
		int( data->create_info_copy.width ),
		int( data->create_info_copy.height ),
		data->create_info_copy.title.c_str(),
		fullscreen_monitor,
		nullptr );
	if( !data->glfw_window ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create window!" );
		}
		return false;
	}

	return true;
}









bool									CreateSurface(
	_internal::WindowDataImpl		*	data,
	VkInstance							instance,
	VkPhysicalDevice					physical_device,
	ResolvedQueue					&	primary_render_queue
)
{
	{
		auto result = glfwCreateWindowSurface(
			instance,
			data->glfw_window,
			nullptr,
			&data->surface
		);
		if( result != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create window surface!" );
			}
			return false;
		}

		VkBool32 surface_supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physical_device,
			primary_render_queue.queueFamilyIndex,
			data->surface,
			&surface_supported
		);
		if( !surface_supported ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Vulkan surface does not support presentation using primary queue!" );
			}
			return false;
		}
	}

	if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
		data->surface,
		&data->surface_capabilities
	) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot get physical device surface capabilities!" );
		}
		return false;
	}

	// Check if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is supported
	{
		VkImageUsageFlags required_image_support = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if( !( ( data->surface_capabilities.supportedUsageFlags & required_image_support ) == required_image_support ) ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Window surface does not support color attachments!" );
			}
			return false;
		}
	}

	// Figure out surface format
	{
		std::vector<VkSurfaceFormatKHR> surface_formats;
		{
			uint32_t surface_format_count = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device,
				data->surface,
				&surface_format_count,
				nullptr
			);
			surface_formats.resize( surface_format_count );
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device,
				data->surface,
				&surface_format_count,
				surface_formats.data()
			);
		}
		data->surface_format = surface_formats[ 0 ];
		if( data->surface_format.format == VK_FORMAT_UNDEFINED ) {
			data->surface_format.format			= VK_FORMAT_R8G8B8A8_UNORM;
			data->surface_format.colorSpace		= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
	}

	return true;
}









bool									CreateRenderPass(
	_internal::WindowDataImpl		*	data,
	VkDevice							device
)
{

	std::array<VkAttachmentDescription, 1>		color_attachment_descriptions {};
	color_attachment_descriptions[ 0 ].flags			= 0;
	color_attachment_descriptions[ 0 ].format			= data->surface_format.format;
	color_attachment_descriptions[ 0 ].samples			= VkSampleCountFlagBits( data->create_info_copy.samples );
	color_attachment_descriptions[ 0 ].loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_descriptions[ 0 ].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_descriptions[ 0 ].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::array<VkAttachmentReference, 0>		input_attachment_references {};

	std::array<VkAttachmentReference, 1>		color_attachment_references {};
	color_attachment_references[ 0 ].attachment		= 0;	// points to color_attachment_descriptions[ 0 ]
	color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<uint32_t, 0>						preserve_attachments {};

	std::array<VkSubpassDescription, 1>			subpasses {};
	subpasses[ 0 ].flags						= 0;
	subpasses[ 0 ].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[ 0 ].inputAttachmentCount			= uint32_t( input_attachment_references.size() );
	subpasses[ 0 ].pInputAttachments			= input_attachment_references.data();
	subpasses[ 0 ].colorAttachmentCount			= uint32_t( color_attachment_references.size() );
	subpasses[ 0 ].pColorAttachments			= color_attachment_references.data();
	subpasses[ 0 ].pResolveAttachments			= data->create_info_copy.samples == Multisamples::SAMPLE_COUNT_1 ? nullptr : color_attachment_references.data();
	subpasses[ 0 ].pDepthStencilAttachment		= nullptr;
	subpasses[ 0 ].preserveAttachmentCount		= uint32_t( preserve_attachments.size() );
	subpasses[ 0 ].pPreserveAttachments			= preserve_attachments.data();

	// OPTIMIZATION: Look here to see if we need to narrow the scope of synchronization to possibly gain performance
	std::array<VkSubpassDependency, 2>			subpass_dependencies {};
	subpass_dependencies[ 0 ].srcSubpass		= VK_SUBPASS_EXTERNAL;
	subpass_dependencies[ 0 ].dstSubpass		= 0;
	subpass_dependencies[ 0 ].srcStageMask		= VK_PIPELINE_STAGE_HOST_BIT;
	subpass_dependencies[ 0 ].dstStageMask		= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	subpass_dependencies[ 0 ].srcAccessMask		= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[ 0 ].dstAccessMask		= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[ 0 ].dependencyFlags	= 0;

	subpass_dependencies[ 1 ].srcSubpass		= 0;
	subpass_dependencies[ 1 ].dstSubpass		= VK_SUBPASS_EXTERNAL;
	subpass_dependencies[ 1 ].srcStageMask		= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	subpass_dependencies[ 1 ].dstStageMask		= VK_PIPELINE_STAGE_HOST_BIT;
	subpass_dependencies[ 1 ].srcAccessMask		= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[ 1 ].dstAccessMask		= VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
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
		&data->render_pass
	) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create render pass!" );
		}
		return false;
	}

	return true;
}









bool									CreateGraphicsPipelines(
	_internal::WindowDataImpl		*	data,
	VkDevice							device,
	VkPipelineCache						pipeline_cache,
	VkPipelineLayout					pipeline_layout,
	VkShaderModule						vertex_shader_module,
	VkShaderModule						fragment_shader_module
)
{

	data->pipelines.resize( size_t( _internal::PipelineType::PIPELINE_TYPE_COUNT ) );

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
	shader_stages[ 0 ].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[ 0 ].pNext					= nullptr;
	shader_stages[ 0 ].flags					= 0;
	shader_stages[ 0 ].stage					= VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[ 0 ].module					= vertex_shader_module;
	shader_stages[ 0 ].pName					= "main";
	shader_stages[ 0 ].pSpecializationInfo	= nullptr;

	shader_stages[ 1 ].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[ 1 ].pNext					= nullptr;
	shader_stages[ 1 ].flags					= 0;
	shader_stages[ 1 ].stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[ 1 ].module					= fragment_shader_module;
	shader_stages[ 1 ].pName					= "main";
	shader_stages[ 1 ].pSpecializationInfo		= nullptr;

	// Make sure this matches Vertex in RenderPrimitives.h
	std::array<VkVertexInputBindingDescription, 1> vertex_input_binding_descriptions {};
	vertex_input_binding_descriptions[ 0 ].binding		= 0;
	vertex_input_binding_descriptions[ 0 ].stride		= sizeof( Vertex );
	vertex_input_binding_descriptions[ 0 ].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 3> vertex_input_attribute_descriptions {};
	vertex_input_attribute_descriptions[ 0 ].location	= 0;
	vertex_input_attribute_descriptions[ 0 ].binding	= 0;
	vertex_input_attribute_descriptions[ 0 ].format		= VK_FORMAT_R32G32_SFLOAT;
	vertex_input_attribute_descriptions[ 0 ].offset		= offsetof( Vertex, vertex_coords );

	vertex_input_attribute_descriptions[ 1 ].location	= 1;
	vertex_input_attribute_descriptions[ 1 ].binding	= 0;
	vertex_input_attribute_descriptions[ 1 ].format		= VK_FORMAT_R32G32_SFLOAT;
	vertex_input_attribute_descriptions[ 1 ].offset		= offsetof( Vertex, uv_coords );

	vertex_input_attribute_descriptions[ 2 ].location	= 2;
	vertex_input_attribute_descriptions[ 2 ].binding	= 0;
	vertex_input_attribute_descriptions[ 2 ].format		= VK_FORMAT_R32G32B32A32_SFLOAT;
	vertex_input_attribute_descriptions[ 2 ].offset		= offsetof( Vertex, color );

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
	vertex_input_state_create_info.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.pNext							= nullptr;
	vertex_input_state_create_info.flags							= 0;
	vertex_input_state_create_info.vertexBindingDescriptionCount	= uint32_t( vertex_input_binding_descriptions.size() );
	vertex_input_state_create_info.pVertexBindingDescriptions		= vertex_input_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount	= uint32_t( vertex_input_attribute_descriptions.size() );
	vertex_input_state_create_info.pVertexAttributeDescriptions		= vertex_input_attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {};
	input_assembly_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.pNext						= nullptr;
	input_assembly_state_create_info.flags						= 0;
	input_assembly_state_create_info.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_state_create_info.primitiveRestartEnable		= VK_FALSE;

	VkViewport viewport {};
	viewport.x			= 0;
	viewport.y			= 0;
	viewport.width		= 512;
	viewport.height		= 512;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor {
		{ 0, 0 },
		{ 512, 512 }
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info {};
	viewport_state_create_info.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.pNext			= nullptr;
	viewport_state_create_info.flags			= 0;
	viewport_state_create_info.viewportCount	= 1;
	viewport_state_create_info.pViewports		= &viewport;
	viewport_state_create_info.scissorCount		= 1;
	viewport_state_create_info.pScissors		= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
	rasterization_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.pNext						= nullptr;
	rasterization_state_create_info.flags						= 0;
	rasterization_state_create_info.depthClampEnable			= VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable		= VK_FALSE;
	rasterization_state_create_info.polygonMode					= VK_POLYGON_MODE_FILL;
	rasterization_state_create_info.cullMode					= VK_CULL_MODE_NONE;
	rasterization_state_create_info.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable				= VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor		= 0.0f;
	rasterization_state_create_info.depthBiasClamp				= 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor		= 0.0f;
	rasterization_state_create_info.lineWidth					= 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
	multisample_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.pNext						= nullptr;
	multisample_state_create_info.flags						= 0;
	multisample_state_create_info.rasterizationSamples		= VkSampleCountFlagBits( data->create_info_copy.samples );
	multisample_state_create_info.sampleShadingEnable		= VK_FALSE;
	multisample_state_create_info.minSampleShading			= 0.0f;
	multisample_state_create_info.pSampleMask				= nullptr;
	multisample_state_create_info.alphaToCoverageEnable		= VK_FALSE;
	multisample_state_create_info.alphaToOneEnable			= VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info {};
	depth_stencil_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.pNext					= nullptr;
	depth_stencil_state_create_info.flags					= 0;
	depth_stencil_state_create_info.depthTestEnable			= VK_FALSE;
	depth_stencil_state_create_info.depthWriteEnable		= VK_FALSE;
	depth_stencil_state_create_info.depthCompareOp			= VK_COMPARE_OP_NEVER;
	depth_stencil_state_create_info.depthBoundsTestEnable	= VK_FALSE;
	depth_stencil_state_create_info.stencilTestEnable		= VK_FALSE;
	depth_stencil_state_create_info.front.failOp			= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.passOp			= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.depthFailOp		= VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.compareOp			= VK_COMPARE_OP_NEVER;
	depth_stencil_state_create_info.front.compareMask		= 0;
	depth_stencil_state_create_info.front.writeMask			= 0;
	depth_stencil_state_create_info.front.reference			= 0;
	depth_stencil_state_create_info.back					= depth_stencil_state_create_info.front;
	depth_stencil_state_create_info.minDepthBounds			= 0.0f;
	depth_stencil_state_create_info.maxDepthBounds			= 1.0f;

	std::array<VkPipelineColorBlendAttachmentState, 1> color_blend_attachment_states {};
	color_blend_attachment_states[ 0 ].blendEnable			= VK_TRUE;
	color_blend_attachment_states[ 0 ].srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].colorBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].srcAlphaBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].alphaBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].colorWriteMask		=
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
	color_blend_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.pNext					= nullptr;
	color_blend_state_create_info.flags					= 0;
	color_blend_state_create_info.logicOpEnable			= VK_FALSE;
	color_blend_state_create_info.logicOp				= VK_LOGIC_OP_CLEAR;
	color_blend_state_create_info.attachmentCount		= uint32_t( color_blend_attachment_states.size() );
	color_blend_state_create_info.pAttachments			= color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[ 0 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 1 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 2 ]	= 0.0f;
	color_blend_state_create_info.blendConstants[ 3 ]	= 0.0f;

	std::vector<VkDynamicState> dynamic_states {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
	dynamic_state_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pNext				= nullptr;
	dynamic_state_create_info.flags				= 0;
	dynamic_state_create_info.dynamicStateCount	= uint32_t( dynamic_states.size() );
	dynamic_state_create_info.pDynamicStates	= dynamic_states.data();

	VkGraphicsPipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.pNext					= nullptr;
	pipeline_create_info.flags					= 0;
	pipeline_create_info.stageCount				= uint32_t( shader_stages.size() );
	pipeline_create_info.pStages				= shader_stages.data();
	pipeline_create_info.pVertexInputState		= &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState	= &input_assembly_state_create_info;
	pipeline_create_info.pTessellationState		= nullptr;
	pipeline_create_info.pViewportState			= &viewport_state_create_info;
	pipeline_create_info.pRasterizationState	= &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState		= &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState		= &depth_stencil_state_create_info;
	pipeline_create_info.pColorBlendState		= &color_blend_state_create_info;
	pipeline_create_info.pDynamicState			= &dynamic_state_create_info;
	pipeline_create_info.layout					= pipeline_layout;
	pipeline_create_info.renderPass				= data->render_pass;
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	// Create filled polygon pipeline
	{
		if( vkCreateGraphicsPipelines(
			device,
			pipeline_cache,
			1,
			&pipeline_create_info,
			nullptr,
			&data->pipelines[ size_t( _internal::PipelineType::FILLED_POLYGON_LIST ) ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
			}
			return false;
		}
	}

	// Create wireframe polygon pipeline
	{
		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info_wireframe {};
		rasterization_state_create_info_wireframe.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info_wireframe.pNext						= nullptr;
		rasterization_state_create_info_wireframe.flags						= 0;
		rasterization_state_create_info_wireframe.depthClampEnable			= VK_FALSE;
		rasterization_state_create_info_wireframe.rasterizerDiscardEnable	= VK_FALSE;
		rasterization_state_create_info_wireframe.polygonMode				= VK_POLYGON_MODE_LINE;
		rasterization_state_create_info_wireframe.cullMode					= VK_CULL_MODE_NONE;
		rasterization_state_create_info_wireframe.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_create_info_wireframe.depthBiasEnable			= VK_FALSE;
		rasterization_state_create_info_wireframe.depthBiasConstantFactor	= 0.0f;
		rasterization_state_create_info_wireframe.depthBiasClamp			= 0.0f;
		rasterization_state_create_info_wireframe.depthBiasSlopeFactor		= 0.0f;
		rasterization_state_create_info_wireframe.lineWidth					= 1.0f;

		pipeline_create_info.pRasterizationState		= &rasterization_state_create_info_wireframe;

		if( vkCreateGraphicsPipelines(
			device,
			pipeline_cache,
			1,
			&pipeline_create_info,
			nullptr,
			&data->pipelines[ size_t( _internal::PipelineType::WIREFRAME_POLYGON_LIST ) ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
			}
			return false;
		}
	}

	// Create line pipeline
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info_line {};
		input_assembly_state_create_info_line.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_create_info_line.pNext						= nullptr;
		input_assembly_state_create_info_line.flags						= 0;
		input_assembly_state_create_info_line.topology					= VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		input_assembly_state_create_info_line.primitiveRestartEnable	= VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info_line {};
		rasterization_state_create_info_line.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info_line.pNext						= nullptr;
		rasterization_state_create_info_line.flags						= 0;
		rasterization_state_create_info_line.depthClampEnable			= VK_FALSE;
		rasterization_state_create_info_line.rasterizerDiscardEnable	= VK_FALSE;
		rasterization_state_create_info_line.polygonMode				= VK_POLYGON_MODE_LINE;
		rasterization_state_create_info_line.cullMode					= VK_CULL_MODE_NONE;
		rasterization_state_create_info_line.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_create_info_line.depthBiasEnable			= VK_FALSE;
		rasterization_state_create_info_line.depthBiasConstantFactor	= 0.0f;
		rasterization_state_create_info_line.depthBiasClamp				= 0.0f;
		rasterization_state_create_info_line.depthBiasSlopeFactor		= 0.0f;
		rasterization_state_create_info_line.lineWidth					= 1.0f;

		pipeline_create_info.pInputAssemblyState		= &input_assembly_state_create_info_line;
		pipeline_create_info.pRasterizationState		= &rasterization_state_create_info_line;

		if( vkCreateGraphicsPipelines(
			device,
			pipeline_cache,
			1,
			&pipeline_create_info,
			nullptr,
			&data->pipelines[ size_t( _internal::PipelineType::LINE_LIST ) ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
			}
			return false;
		}
	}

	// Create point pipeline
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info_point {};
		input_assembly_state_create_info_point.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_create_info_point.pNext					= nullptr;
		input_assembly_state_create_info_point.flags					= 0;
		input_assembly_state_create_info_point.topology					= VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		input_assembly_state_create_info_point.primitiveRestartEnable	= VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterization_state_create_info_point {};
		rasterization_state_create_info_point.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_create_info_point.pNext						= nullptr;
		rasterization_state_create_info_point.flags						= 0;
		rasterization_state_create_info_point.depthClampEnable			= VK_FALSE;
		rasterization_state_create_info_point.rasterizerDiscardEnable	= VK_FALSE;
		rasterization_state_create_info_point.polygonMode				= VK_POLYGON_MODE_POINT;
		rasterization_state_create_info_point.cullMode					= VK_CULL_MODE_NONE;
		rasterization_state_create_info_point.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_create_info_point.depthBiasEnable			= VK_FALSE;
		rasterization_state_create_info_point.depthBiasConstantFactor	= 0.0f;
		rasterization_state_create_info_point.depthBiasClamp			= 0.0f;
		rasterization_state_create_info_point.depthBiasSlopeFactor		= 0.0f;
		rasterization_state_create_info_point.lineWidth					= 1.0f;

		pipeline_create_info.pInputAssemblyState		= &input_assembly_state_create_info_point;
		pipeline_create_info.pRasterizationState		= &rasterization_state_create_info_point;

		if( vkCreateGraphicsPipelines(
			device,
			pipeline_cache,
			1,
			&pipeline_create_info,
			nullptr,
			&data->pipelines[ size_t( _internal::PipelineType::POINT_LIST ) ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
			}
			return false;
		}
	}

	return true;
}









bool CreateCommandPool(
	_internal::WindowDataImpl		*	data,
	VkDevice							device,
	ResolvedQueue					&	primary_render_queue )
{

	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext				= nullptr;
	command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex	= primary_render_queue.queueFamilyIndex;

	if( vkCreateCommandPool(
		device,
		&command_pool_create_info,
		nullptr,
		&data->command_pool
	) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan command pool!" );
		}
		return false;
	}

	return true;
}









bool									AllocateCommandBuffers(
	_internal::WindowDataImpl		*	data,
	VkDevice							device )
{
	data->command_buffers.resize( data->swapchain_image_count );

	VkCommandBufferAllocateInfo command_buffer_allocate_info {};
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext				= nullptr;
	command_buffer_allocate_info.commandPool		= data->command_pool;
	command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount	= data->swapchain_image_count;

	if( vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		data->command_buffers.data()
	) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot allocate command buffers!" );
		}
		return false;
	}

	return true;
}









bool									CreateSwapchain(
	_internal::WindowDataImpl		*	data,
	VkPhysicalDevice					physical_device,
	VkDevice							device,
	ResolvedQueue						primary_render_queue,
	VkSwapchainKHR						old_swapchain
)
{
	if( !SynchronizeFrame( data, device ) ) return false;

	// Create swapchain
	{
		// Figure out image count
		{
			if( data->create_info_copy.vsync ) {
				data->swapchain_image_count = 2;	// Vsync enabled, we only need 2 swapchain images
			} else {
				data->swapchain_image_count = 3;	// Vsync disabled, we should use at least 3 images
			}
			if( data->surface_capabilities.maxImageCount != 0 ) {
				if( data->swapchain_image_count > data->surface_capabilities.maxImageCount ) {
					data->swapchain_image_count = data->surface_capabilities.maxImageCount;
				}
			}
			if( data->swapchain_image_count < data->surface_capabilities.minImageCount ) {
				data->swapchain_image_count = data->surface_capabilities.minImageCount;
			}
		}

		// Figure out image dimensions and set window minimum and maximum sizes
		{
			data->min_extent		= {
				data->create_info_copy.min_width,
				data->create_info_copy.min_height
			};
			data->max_extent		= {
				data->create_info_copy.max_width,
				data->create_info_copy.max_height
			};

			// Set window size limits
			glfwSetWindowSizeLimits(
				data->glfw_window,
				int( data->min_extent.width ),
				int( data->min_extent.height ),
				int( data->max_extent.width ),
				int( data->max_extent.height )
			);

			// Get new surface capabilities as window extent might have changed
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physical_device,
				data->surface,
				&data->surface_capabilities
			);

			data->extent	= data->surface_capabilities.currentExtent;
		}

		// Figure out present mode
		{
			bool present_mode_found		= false;
			if( data->create_info_copy.vsync ) {
				// Using VSync we should use FIFO, this mode is required to be supported so we can rely on that and just use it without checking
				data->present_mode		= VK_PRESENT_MODE_FIFO_KHR;
				present_mode_found		= true;
			} else {
				// Not using VSync, we should try mailbox first and immediate second, fall back to FIFO if neither is supported
				std::vector<VkPresentModeKHR> surface_present_modes;
				{
					uint32_t present_mode_count = 0;
					vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						data->surface,
						&present_mode_count,
						nullptr
					);
					surface_present_modes.resize( present_mode_count );
					vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						data->surface,
						&present_mode_count,
						surface_present_modes.data()
					);
				}
				// Since there are only 2 things we're interested in finding we'll do a simple
				// check if we could find either, if we found the better one, break out so we
				// don't pick the worse option later.
				for( auto p : surface_present_modes ) {
					if( p == VK_PRESENT_MODE_MAILBOX_KHR ) {
						data->present_mode	= VK_PRESENT_MODE_MAILBOX_KHR;
						present_mode_found	= true;
						break;	// found the best, break out
					} else if( p == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
						data->present_mode	= VK_PRESENT_MODE_IMMEDIATE_KHR;
						present_mode_found	= true;
					}
				}
			}
			if( !present_mode_found ) {
				// Present mode not found, use FIFO as a fallback
				data->present_mode		= VK_PRESENT_MODE_FIFO_KHR;
			}

			VkSwapchainCreateInfoKHR swapchain_create_info {};
			swapchain_create_info.sType						= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchain_create_info.pNext						= nullptr;
			swapchain_create_info.flags						= 0;
			swapchain_create_info.surface					= data->surface;
			swapchain_create_info.minImageCount				= data->swapchain_image_count;
			swapchain_create_info.imageFormat				= data->surface_format.format;
			swapchain_create_info.imageColorSpace			= data->surface_format.colorSpace;
			swapchain_create_info.imageExtent				= data->extent;
			swapchain_create_info.imageArrayLayers			= 1;
			swapchain_create_info.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchain_create_info.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount		= 0;
			swapchain_create_info.pQueueFamilyIndices		= nullptr;
			swapchain_create_info.preTransform				= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchain_create_info.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;		// Check this if rendering transparent windows
			swapchain_create_info.presentMode				= data->present_mode;
			swapchain_create_info.clipped					= VK_FALSE;
			swapchain_create_info.oldSwapchain				= old_swapchain;

			auto result = vkCreateSwapchainKHR(
				device,
				&swapchain_create_info,
				nullptr,
				&data->swapchain
			);
			if( result != VK_SUCCESS ) {
				if( data->report_function ) {
					data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan swapchain!" );
				}
				return false;
			}
		}

		// Get swapchain images and create image views
		{
			uint32_t swapchain_image_count = 0;
			vkGetSwapchainImagesKHR(
				device,
				data->swapchain,
				&swapchain_image_count,
				nullptr
			);
			data->swapchain_images.resize( swapchain_image_count );
			vkGetSwapchainImagesKHR(
				device,
				data->swapchain,
				&swapchain_image_count,
				data->swapchain_images.data()
			);

			// Destroy old swapchain image views if they exist
			for( auto v : data->swapchain_image_views ) {
				vkDestroyImageView(
					device,
					v,
					nullptr
				);
			}

			data->swapchain_image_views.resize( swapchain_image_count );
			for( size_t i = 0; i < swapchain_image_count; ++i ) {

				VkImageViewCreateInfo image_view_create_info {};
				image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_create_info.pNext				= nullptr;
				image_view_create_info.flags				= 0;
				image_view_create_info.image				= data->swapchain_images[ i ];
				image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
				image_view_create_info.format				= data->surface_format.format;
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
					&data->swapchain_image_views[ i ]
				);
				if( result != VK_SUCCESS ) {
					if( data->report_function ) {
						data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create image views for swapchain!" );
					}
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

	data->recreate_swapchain		= false;

	return true;
}









bool CreateFramebuffers(
	_internal::WindowDataImpl		*	data,
	VkDevice							device )
{
	data->framebuffers.resize( data->swapchain_image_count );

	for( uint32_t i = 0; i < data->swapchain_image_count; ++i ) {

		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext				= nullptr;
		framebuffer_create_info.flags				= 0;
		framebuffer_create_info.renderPass			= data->render_pass;
		framebuffer_create_info.attachmentCount		= 1;
		framebuffer_create_info.pAttachments		= &data->swapchain_image_views[ i ];
		framebuffer_create_info.width				= data->extent.width;
		framebuffer_create_info.height				= data->extent.height;
		framebuffer_create_info.layers				= 1;

		if( vkCreateFramebuffer(
			device,
			&framebuffer_create_info,
			nullptr,
			&data->framebuffers[ i ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create framebuffers!" );
			}
			return false;
		}
	}

	return true;
}









bool CreateWindowSynchronizationPrimitives(
	_internal::WindowDataImpl		*	data,
	VkDevice							device )
{
	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;
	if( vkCreateFence(
		device,
		&fence_create_info,
		nullptr,
		&data->aquire_image_fence
	) != VK_SUCCESS ) {
		if( data->report_function ) {
			data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan fence!" );
		}
		return false;
	}

	return true;
}









bool									CreateFrameSynchronizationPrimitives(
	_internal::WindowDataImpl		*	data,
	VkDevice							device )
{
	data->submit_to_present_semaphores.resize( data->swapchain_image_count );

	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext		= nullptr;
	semaphore_create_info.flags		= 0;

	for( auto & s : data->submit_to_present_semaphores ) {
		if( vkCreateSemaphore(
			device,
			&semaphore_create_info,
			nullptr,
			&s
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create synchronization semaphores!" );
			}
			return false;
		}
	}

	data->gpu_to_cpu_frame_fences.resize( data->swapchain_image_count );

	VkFenceCreateInfo			fence_create_info {};
	fence_create_info.sType		= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext		= nullptr;
	fence_create_info.flags		= 0;

	for( auto & f : data->gpu_to_cpu_frame_fences ) {
		if( vkCreateFence(
			device,
			&fence_create_info,
			nullptr,
			&f
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create synchronization fences!" );
			}
			return false;
		}
	}

	return true;
}









bool SynchronizeFrame(
	_internal::WindowDataImpl	*	data,
	VkDevice						device
)
{
	if( data->previous_frame_need_synchronization ) {
		if( vkWaitForFences(
			device,
			1, &data->gpu_to_cpu_frame_fences[ data->previous_image ],
			VK_TRUE,
			UINT64_MAX
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Error synchronizing frame!" );
			}
			return false;
		}
		if( vkResetFences(
			device,
			1, &data->gpu_to_cpu_frame_fences[ data->previous_image ]
		) != VK_SUCCESS ) {
			if( data->report_function ) {
				data->report_function( ReportSeverity::NON_CRITICAL_ERROR, "Error synchronizing frame!" );
			}
			return false;
		}
		// And we also don't need to synchronize later.
		data->previous_frame_need_synchronization	= false;
	}

	return true;
}
