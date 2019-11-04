
#include "../Header/SourceCommon.h"

#include "../Header/WindowImpl.h"

#include "../Header/RendererImpl.h"

#include <memory>
#include <algorithm>
#include <set>



namespace vk2d {



constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



namespace _internal {






WindowImpl::WindowImpl(
	RendererImpl					*	renderer,
	const WindowCreateInfo			&	window_create_info
)
{
	create_info_copy				= window_create_info;
	renderer_parent					= renderer;
	report_function					= renderer_parent->GetReportFunction();

	instance						= renderer_parent->GetVulkanInstance();
	physical_device					= renderer_parent->GetVulkanPhysicalDevice();
	device							= renderer_parent->GetVulkanDevice();
	primary_render_queue			= renderer_parent->GetPrimaryRenderQueue();

	if( !CreateGLFWWindow() ) return;
	if( !CreateSurface() ) return;
	if( !CreateRenderPass() ) return;
	if( !CreateGraphicsPipelines() ) return;
	if( !CreateSwapchain() ) return;
	if( !CreateFramebuffers() ) return;
	if( !CreateCommandPool() ) return;
	if( !AllocateCommandBuffers() ) return;
	if( !CreateWindowSynchronizationPrimitives() ) return;
	if( !CreateFrameSynchronizationPrimitives() ) return;

	mesh_buffer		= std::make_unique<MeshBuffer>(
		device,
		renderer->GetPhysicalDeviceProperties().limits,
		this,
		renderer->GetDeviceMemoryPool() );

	if( !mesh_buffer ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create mesh buffers!" );
		}
		return;
	}

	is_good		= true;
}









WindowImpl::~WindowImpl()
{
	vkDeviceWaitIdle( device );

	mesh_buffer		= nullptr;

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

	for( auto p : pipelines ) {
		vkDestroyPipeline(
			device,
			p,
			nullptr
		);
	}

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









bool									AquireImage(
	WindowImpl						*	data,
	VkPhysicalDevice					physical_device,
	VkDevice							device,
	ResolvedQueue					&	primary_render_queue,
	uint32_t							nested_counter				= 0 )
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
			if( !data->RecreateResourcesAfterResizing() ) {
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

bool WindowImpl::BeginRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != _internal::NextRenderCallFunction::BEGIN ) {
		if( report_function ) {
			report_function( ReportSeverity::WARNING, "BeginRender() Called twice in a row!" );
		}
		return false;
	} else {
		next_render_call_function = _internal::NextRenderCallFunction::END;
	}

	// TODO: check if we need to recreate the swapchain
	if( recreate_swapchain ) {
		if( !RecreateResourcesAfterResizing() ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot recreate swapchain!" );
			}
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot aquire next swapchain image!" );
			}
			return false;
		}
	}

	// If next image index happens to same as the previous, presentation has propably already succeeded but
	// since we're using the image index as an index to our command buffers and framebuffers we'll have to
	// make sure that we don't start overwriting a command buffer until it's execution has completely
	// finished, so we'll have to synchronize the frame early in here.
	if( next_image == previous_image ) {
		if( !SynchronizeFrame() ) return false;
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot begin recording command buffer!" );
			}
			return false;
		}

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

			vkCmdBeginRenderPass(
				command_buffer,
				&render_pass_begin_info,
				VK_SUBPASS_CONTENTS_INLINE
			);
		}
	}

	return true;
}









bool WindowImpl::EndRender()
{
	// Calls to BeginRender() and EndRender() should alternate, check it's our turn
	if( next_render_call_function != _internal::NextRenderCallFunction::END ) {
		if( report_function ) {
			report_function( ReportSeverity::WARNING, "EndRender() Called twice in a row!" );
		}
		return false;
	} else {
		next_render_call_function = _internal::NextRenderCallFunction::BEGIN;
	}

	VkCommandBuffer		render_command_buffer	= render_command_buffers[ next_image ];

	// End render pass
	{
		vkCmdEndRenderPass( render_command_buffer );
	}

	// End command buffer
	if( vkEndCommandBuffer( render_command_buffer ) != VK_SUCCESS ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot compile command buffer at EndRender()!" );
		}
		return false;
	}

	// Synchronize the previous frame here, this waits for the previous
	// frame to finish fully rendering before continuing execution.
	if( !SynchronizeFrame() ) return false;

	// Record command buffer to upload mesh data to GPU
	{
		// Begin command buffer
		{
			VkCommandBufferBeginInfo transfer_command_buffer_begin_info {};
			transfer_command_buffer_begin_info.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			transfer_command_buffer_begin_info.pNext			= nullptr;
			transfer_command_buffer_begin_info.flags			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			transfer_command_buffer_begin_info.pInheritanceInfo	= nullptr;

			// SynchronizeFrame() above will guarantee that the transfer command buffer is not
			// still being processed so we can begin re-recording the transfer command buffer here.
			if( vkBeginCommandBuffer(
				mesh_transfer_command_buffer,
				&transfer_command_buffer_begin_info
			) != VK_SUCCESS ) {
				if( report_function ) {
					report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot begin mesh GPU transfer command buffer!" );
				}
				return false;
			}
		}

		// Record commands
		{
			if( !mesh_buffer->CmdUploadMeshDataToGPU(
				mesh_transfer_command_buffer
			) ) {
				if( report_function ) {
					report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot record commands to transfer mesh to GPU!" );
				}
				return false;
			}
		}

		// End command buffer
		{
			if( vkEndCommandBuffer(
				mesh_transfer_command_buffer
			) != VK_SUCCESS ) {
				if( report_function ) {
					report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot compile commands to transfer mesh to GPU!" );
				}
				return false;
			}
		}
	}

	// Submit swapchain image
	{
		std::array<VkSubmitInfo, 2> submit_infos {};

		submit_infos[ 0 ].sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_infos[ 0 ].pNext						= nullptr;
		submit_infos[ 0 ].waitSemaphoreCount		= 0;
		submit_infos[ 0 ].pWaitSemaphores			= nullptr;
		submit_infos[ 0 ].pWaitDstStageMask			= nullptr;
		submit_infos[ 0 ].commandBufferCount		= 1;
		submit_infos[ 0 ].pCommandBuffers			= &mesh_transfer_command_buffer;
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

		if( vkQueueSubmit(
			primary_render_queue.queue,
			uint32_t( submit_infos.size() ), submit_infos.data() ,
			gpu_to_cpu_frame_fences[ next_image ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot submit command buffer to queue!" );
			}
			return false;
		}
	}

	vkDeviceWaitIdle( device );

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
		auto result = vkQueuePresentKHR(
			primary_render_queue.queue,
			&present_info
		);
		if( result != VK_SUCCESS || present_result != VK_SUCCESS ) {
			if( result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_ERROR_OUT_OF_DATE_KHR ||
				result == VK_SUBOPTIMAL_KHR || present_result == VK_SUBOPTIMAL_KHR ) {
				recreate_swapchain	= true;
			} else {
				if( report_function ) {
					report_function( ReportSeverity::WARNING, "Cannot present image!" );
				}
				return false;
			}
		}
	}

	vkDeviceWaitIdle( device );

	previous_image						= next_image;
	previous_frame_need_synchronization	= true;
	previous_pipeline_type				= _internal::PipelineType::NONE;

	glfwPollEvents();

	return true;
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
void WindowImpl::Draw_TriangleList(
	bool								filled,
	std::vector<Vertex>				&	vertices,
	std::vector<VertexIndex_3>		&	indices )
{
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( indices.size() * 3 );
	std::vector<uint32_t> raw_indices;
	raw_indices.reserve( index_count );
	for( size_t i = 0; i < indices.size(); ++i ) {
		raw_indices.push_back( indices[ i ].indices[ 0 ] );
		raw_indices.push_back( indices[ i ].indices[ 1 ] );
		raw_indices.push_back( indices[ i ].indices[ 2 ] );
	}

	if( filled ) {
		CmdBindPipeline(
			command_buffer,
			_internal::PipelineType::FILLED_POLYGON_LIST
		);
	} else {
		CmdBindPipeline(
			command_buffer,
			_internal::PipelineType::WIREFRAME_POLYGON_LIST
		);
	}

	auto result = mesh_buffer->CmdPushMesh(
		command_buffer,
		vertices,
		raw_indices );

	vkCmdDrawIndexed(
		command_buffer,
		index_count,
		1,
		result.offsets.first_index,
		int32_t( result.offsets.vertex_offset ),
		0
	);
}

void WindowImpl::Draw_LineList(
	std::vector<Vertex>				&	vertices,
	std::vector<VertexIndex_2>		&	indices )
{
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( indices.size() * 2 );
	std::vector<uint32_t> raw_indices;
	raw_indices.reserve( index_count );
	for( size_t i = 0; i < indices.size(); ++i ) {
		raw_indices.push_back( indices[ i ].indices[ 0 ] );
		raw_indices.push_back( indices[ i ].indices[ 1 ] );
	}

	CmdBindPipeline(
		command_buffer,
		_internal::PipelineType::LINE_LIST
	);

	auto result = mesh_buffer->CmdPushMesh(
		command_buffer,
		vertices,
		raw_indices );

	vkCmdDrawIndexed(
		command_buffer,
		index_count,
		1,
		result.offsets.first_index,
		int32_t( result.offsets.vertex_offset ),
		0
	);
}

void WindowImpl::Draw_PointList(
	std::vector<Vertex>				&	vertices
)
{
	auto command_buffer					= render_command_buffers[ next_image ];

	auto vertex_count	= uint32_t( vertices.size() );

	CmdBindPipeline(
		command_buffer,
		_internal::PipelineType::POINT_LIST
	);

	auto result = mesh_buffer->CmdPushMesh(
		command_buffer,
		vertices,
		{} );

	vkCmdDraw(
		command_buffer,
		vertex_count,
		1,
		result.offsets.vertex_offset,
		0
	);
}

void WindowImpl::Draw_Line(
	Coords								point_1,
	Coords								point_2,
	Color								color )
{
	std::vector<Vertex>				vertices( 2 );
	std::vector<VertexIndex_2>		indices( 1 );

	vertices[ 0 ].vertex_coords		= point_1;
	vertices[ 0 ].uv_coords			= {};
	vertices[ 0 ].color				= color;

	vertices[ 1 ].vertex_coords		= point_2;
	vertices[ 1 ].uv_coords			= {};
	vertices[ 1 ].color				= color;

	indices[ 0 ].indices			= { 0, 1 };

	Draw_LineList(
		vertices,
		indices
	);
}

void WindowImpl::Draw_Box(
	bool								filled,
	Coords								top_left,
	Coords								bottom_right,
	Color								color )
{
	std::vector<Vertex>				vertices( 4 );

	// 0. Top left
	vertices[ 0 ].vertex_coords		= { top_left.x,			top_left.y };
	vertices[ 0 ].uv_coords			= vertices[ 0 ].vertex_coords;
	vertices[ 0 ].color				= color;

	// 1. Top right
	vertices[ 1 ].vertex_coords		= { bottom_right.x,		top_left.y };
	vertices[ 1 ].uv_coords			= vertices[ 1 ].vertex_coords;
	vertices[ 1 ].color				= color;

	// 2. Bottom left
	vertices[ 2 ].vertex_coords		= { top_left.x,			bottom_right.y };
	vertices[ 2 ].uv_coords			= vertices[ 2 ].vertex_coords;
	vertices[ 2 ].color				= color;

	// 3. Bottom right
	vertices[ 3 ].vertex_coords		= { bottom_right.x,		bottom_right.y };
	vertices[ 3 ].uv_coords			= vertices[ 3 ].vertex_coords;
	vertices[ 3 ].color				= color;

	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3>	indices( 2 );
		indices[ 0 ].indices		= { 0, 2, 1 };
		indices[ 1 ].indices		= { 1, 2, 3 };
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2>	indices( 4 );
		indices[ 0 ].indices		= { 0, 2 };
		indices[ 1 ].indices		= { 2, 3 };
		indices[ 2 ].indices		= { 3, 1 };
		indices[ 3 ].indices		= { 1, 0 };
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_Circle(
	bool								filled,
	Coords								top_left,
	Coords								bottom_right,
	float								edge_count,
	Color								color
)
{
	if( edge_count < 3.0f ) edge_count = 3.0f;

	Coords center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float rotation_step			= 0.0f;
	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( edge_count );
	if( edge_count - uint32_t( edge_count ) ) {
		++edge_count_integer;
	}

	std::vector<Vertex> vertices( edge_count_integer );
	for( uint32_t i = 0; i < edge_count_integer; ++i ) {
		vertices[ i ].vertex_coords		= {
			std::cos( rotation_step ) * center_to_edge_x + center_point.x,
			std::sin( rotation_step ) * center_to_edge_y + center_point.y
		};
		vertices[ i ].uv_coords			= {
			std::cos( rotation_step ),
			std::sin( rotation_step )
		};
		vertices[ i ].color				= color;
		rotation_step					+= rotation_step_size;
	}

	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3> indices( size_t( edge_count_integer ) + 1 );
		{
			uint32_t i = 0;
			for( i = 2; i < edge_count_integer; ++i ) {
				assert( i < edge_count_integer );
				indices[ i ] = { 0, i - 1, i };
			}
		}
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2> indices( size_t( edge_count_integer ) + 1 );
		{
			uint32_t i = 0;
			for( i = 1; i < edge_count_integer; ++i ) {
				assert( i < edge_count_integer );
				indices[ i ] = { i - 1, i };
			}
			indices[ edge_count_integer ] = { edge_count_integer - 1, 0 };
		}
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_Pie(
	bool								filled,
	Coords								top_left,
	Coords								bottom_right,
	float								begin_angle_radians,
	float								coverage,
	float								edge_count,
	Color								color
)
{
	if( edge_count < 3.0f )			edge_count		= 3.0f;
	if( coverage > 1.0f )			coverage		= 1.0f;
	if( coverage <= 0.0f )			return;			// Nothing to draw

	Coords center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float rotation_step			= 0.0f;
	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( edge_count );
	if( edge_count - uint32_t( edge_count ) ) {
		++edge_count_integer;
	}

	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

	std::vector<Vertex> vertices; vertices.reserve( size_t( edge_count_integer ) + 2 );
	vertices.push_back( {
		center_point,
		{ 0.5f, 0.5f },
		color }
	);
	while( true ) {
		if( rotation_step >= area_angle ) break;
		vertices.push_back( { {
				std::cos( rotation_step + begin_angle_radians ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step + begin_angle_radians ) * center_to_edge_y + center_point.y
			}, {
				std::cos( rotation_step ),
				std::sin( rotation_step )
			},
			color }
		);
		rotation_step					+= rotation_step_size;
	}
	vertices.push_back( { {
			std::cos( end_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( end_angle_radians ) * center_to_edge_y + center_point.y
		}, {
			std::cos( end_angle_radians ),
			std::sin( end_angle_radians )
		},
		color }
	);


	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3> indices( vertices.size() + 1 );
		{
			uint32_t i = 0;
			for( i = 2; i < indices.size() - 1; ++i ) {
				//assert( i < edge_count_integer );
				indices[ i ] = { 0, i - 1, i };
			}
		}
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2> indices; indices.reserve( vertices.size() + 2 );
		{
			if( coverage < 1.0f )		indices.push_back( { 0, 1 } );
			uint32_t i = 0;
			for( i = 2; i < uint32_t( vertices.size() ); ++i ) {
				indices.push_back( { i - 1, i } );
			}
			if( coverage < 1.0f )		indices.push_back( { uint32_t( vertices.size() - 1 ), 0 } );
		}
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_PieBox(
	bool								filled,
	Coords								top_left,
	Coords								bottom_right,
	float								begin_angle_radians,
	float								coverage,
	Color								color
)
{
	if( coverage >= 1.0f ) {
		Draw_Box( filled, top_left, bottom_right, color );
		return;		// Done
	}
	if( coverage <= 0.0f ) {
		return;		// Nothing to draw
	}

	Coords center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float width					= std::abs( bottom_right.x - top_left.x );
	float height				= std::abs( bottom_right.y - top_left.y );

	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

	enum AllPointsIndex : uint32_t {
		BOX_CORNER_0,
		BOX_CORNER_1,
		BOX_CORNER_2,
		BOX_CORNER_3,
		PIE_BEGIN,
		PIE_END,
	};
	std::array<Coords, 6>	unordered_outer_points { {
		{ top_left.x,		top_left.y		},
		{ bottom_right.x,	top_left.y		},
		{ bottom_right.x,	bottom_right.y	},
		{ top_left.x,		bottom_right.y	}
	} };

	// Get pie begin and end coordinates
	{
		// Specialized version of Ray to AABB intersecion test that
		// always intersects and returns the exit coordinates only.
		auto RayExitIntersection	=[](
			Coords box_coords_1,
			Coords box_coords_2,
			Coords box_center,		// ray origin, and box center are both at the same coordinates
			Coords ray_end
			) -> Coords
		{
			Coords		ray_begin			= box_center;
			float		clipped_exit		= 1.0;

			// Clipping on x and y axis
			float dim_low_x		= ( box_coords_1.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_low_y		= ( box_coords_1.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );
			float dim_high_x	= ( box_coords_2.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_high_y	= ( box_coords_2.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );

			dim_high_x			= std::max( dim_low_x, dim_high_x );
			dim_high_y			= std::max( dim_low_y, dim_high_y );
			clipped_exit		= std::min( dim_high_y, dim_high_x );

			Coords collider_localized				= ray_end - ray_begin;
			Coords localized_intersection_point		= collider_localized * clipped_exit;
			Coords globalized_intersection_point	= localized_intersection_point + box_center;
			return globalized_intersection_point;
		};

		float ray_lenght		= center_to_edge_x + center_to_edge_y;
		Coords ray_angle_1_end	= { std::cos( begin_angle_radians ) * ray_lenght, std::sin( begin_angle_radians ) * ray_lenght };
		Coords ray_angle_2_end	= { std::cos( end_angle_radians ) * ray_lenght, std::sin( end_angle_radians ) * ray_lenght };
		ray_angle_1_end			+= center_point;
		ray_angle_2_end			+= center_point;

		unordered_outer_points[ PIE_BEGIN ]	= RayExitIntersection(
			top_left,
			bottom_right,
			center_point,
			ray_angle_1_end
		);
		unordered_outer_points[ PIE_END ]		= RayExitIntersection(
			top_left,
			bottom_right,
			center_point,
			ray_angle_2_end
		);
	}

	auto IsOnSameXAxis	= [](
		Coords			point_1,
		Coords			point_2
		) -> bool
	{
		if( point_1.y < point_2.y + 0.0001f &&
			point_1.y > point_2.y - 0.0001f ) {
			return true;
		}
		return false;
	};

	auto IsOnSameYAxis	= [](
		Coords			point_1,
		Coords			point_2
		) -> bool
	{
		if( point_1.x < point_2.x + 0.0001f &&
			point_1.x > point_2.x - 0.0001f ) {
			return true;
		}
		return false;
	};

	// Generate an ordered point list
	std::vector<Coords> outer_point_list;
	{
		// We'll linearize / unwrap the box so that a single number represents x/y coordinates
		//
		// 0---1
		// |   |	->	0---1---2---3---0
		// 3---2

		struct LinearPoint {
			uint32_t	original_linear_point_index;
			float		linear_coords;
			Coords		actual_coords;
		};
		float distance_counter = 0.0f;
		std::array<LinearPoint, 6>	linear_points { {
			{ BOX_CORNER_0, 0.0f,							unordered_outer_points[ BOX_CORNER_0 ] },
			{ BOX_CORNER_1, distance_counter += width,		unordered_outer_points[ BOX_CORNER_1 ] },
			{ BOX_CORNER_2, distance_counter += height,		unordered_outer_points[ BOX_CORNER_2 ] },
			{ BOX_CORNER_3, distance_counter += width,		unordered_outer_points[ BOX_CORNER_3 ] }
		} };

		auto ResolveLinearPoint	= [
			width,
			height,
			IsOnSameXAxis,
			IsOnSameYAxis,
			&unordered_outer_points,
			&linear_points
		](
			uint32_t		index
			)
		{
			// Find which side of the box the point is located at
			Coords actual_coords	= unordered_outer_points[ index ];

			if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
				// Top
				float linear		= ( actual_coords.x - linear_points[ 0 ].actual_coords.x ) + linear_points[ 0 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 1 ] ) ) {
				// Right
				float linear		= ( actual_coords.y - linear_points[ 1 ].actual_coords.y ) + linear_points[ 1 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 3 ] ) ) {
				// Bottom
				float linear		= ( linear_points[ 2 ].actual_coords.x - actual_coords.x ) + linear_points[ 2 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
				// Left
				float linear		= ( linear_points[ 3 ].actual_coords.y - actual_coords.y ) + linear_points[ 3 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else {
				assert( 0 );
			}
		};

		ResolveLinearPoint( PIE_BEGIN );
		ResolveLinearPoint( PIE_END );

		std::sort(
			linear_points.begin(),
			linear_points.end(),
			[]( LinearPoint & p1, LinearPoint & p2 )
			{
				return p1.linear_coords < p2.linear_coords;
			} );

		// We have a list of points in a winding order.
		// Now we need to figure out which points are
		// within and collect a list from those.

		// Find current index for pie begin and end in linear_points array
		uint32_t	linear_resolved_pie_begin_point		= UINT32_MAX;
		uint32_t	linear_resolved_pie_end_point		= UINT32_MAX;
		for( size_t i = 0; i < linear_points.size(); ++i ) {
			if( linear_points[ i ].original_linear_point_index == PIE_BEGIN )	linear_resolved_pie_begin_point	= uint32_t( i );
			if( linear_points[ i ].original_linear_point_index == PIE_END )		linear_resolved_pie_end_point	= uint32_t( i );
		}
		assert( linear_resolved_pie_begin_point != UINT32_MAX );
		assert( linear_resolved_pie_end_point != UINT32_MAX );

		// Sort() organized everything for us, now we just need to
		// collect all in-between coordinates from the indices
		outer_point_list.reserve( 6 );
		uint32_t current_linear_point_index		= linear_resolved_pie_begin_point;
		while( current_linear_point_index != linear_resolved_pie_end_point ) {
			outer_point_list.push_back( linear_points[ current_linear_point_index ].actual_coords );
			if( ( ++current_linear_point_index ) == 6 ) current_linear_point_index = 0;
		}
		outer_point_list.push_back( linear_points[ linear_resolved_pie_end_point ].actual_coords );
	}

	// Create vertices
	std::vector<Vertex> vertices;
	{
		vertices.reserve( 7 );

		vertices.push_back( {
			center_point,
			{ 0.5f, 0.5f },
			color
			} );
		for( auto opl : outer_point_list ) {
			vertices.push_back( {
				opl,
				( opl - unordered_outer_points[ 0 ] ) / Coords( width, height ),
				color
				} );
		}
	}

	if( filled ) {
		std::vector<VertexIndex_3> indices;		indices.reserve( vertices.size() );
		for( uint32_t i = 2; i < uint32_t( vertices.size() ); ++i ) {
			indices.push_back( { 0, i - 1, i } );
		}

		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		std::vector<VertexIndex_2> indices;		indices.reserve( vertices.size() );
		for( uint32_t i = 1; i < uint32_t( vertices.size() ); ++i ) {
			indices.push_back( { i - 1, i } );
		}
		indices.push_back( { uint32_t( vertices.size() - 1 ), 0 } );
		Draw_LineList(
			vertices,
			indices
		);
	}
}









bool WindowImpl::SynchronizeFrame()
{
	if( previous_frame_need_synchronization ) {
		if( vkWaitForFences(
			device,
			1, &gpu_to_cpu_frame_fences[ previous_image ],
			VK_TRUE,
			UINT64_MAX
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Error synchronizing frame!" );
			}
			return false;
		}
		if( vkResetFences(
			device,
			1, &gpu_to_cpu_frame_fences[ previous_image ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Error synchronizing frame!" );
			}
			return false;
		}
		// And we also don't need to synchronize later.
		previous_frame_need_synchronization	= false;
	}

	return true;
}









bool WindowImpl::RecreateResourcesAfterResizing( )
{
	if( !CreateSwapchain() ) return false;

	// Check if any other resources need to be reallocated

	// Reallocate framebuffers
	{
		if( framebuffers.size() ) {
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

	recreate_swapchain		= false;

	return true;
}









bool WindowImpl::CreateGLFWWindow()
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

	std::vector<GLFWmonitor*> monitors;
	{
		int monitor_count = 0;
		auto monitors_ptr = glfwGetMonitors( &monitor_count );
		for( int i = 0; i < monitor_count; ++i ) {
			monitors.push_back( monitors_ptr[ i ] );
		}
	}

	GLFWmonitor * fullscreen_monitor = nullptr;
	if( create_info_copy.fullscreen_monitor > 0 &&
		create_info_copy.fullscreen_monitor <= uint32_t( monitors.size() ) ) {
		fullscreen_monitor		= monitors[ create_info_copy.fullscreen_monitor ];
	}

	glfw_window = glfwCreateWindow(
		int( create_info_copy.width ),
		int( create_info_copy.height ),
		create_info_copy.title.c_str(),
		fullscreen_monitor,
		nullptr );
	if( !glfw_window ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create window!" );
		}
		return false;
	}

	return true;
}









bool WindowImpl::CreateSurface()
{
	{
		auto result = glfwCreateWindowSurface(
			instance,
			glfw_window,
			nullptr,
			&surface
		);
		if( result != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create window surface!" );
			}
			return false;
		}

		VkBool32 surface_supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physical_device,
			primary_render_queue.queueFamilyIndex,
			surface,
			&surface_supported
		);
		if( !surface_supported ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Vulkan surface does not support presentation using primary queue!" );
			}
			return false;
		}
	}

	if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
		surface,
		&surface_capabilities
	) != VK_SUCCESS ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot get physical device surface capabilities!" );
		}
		return false;
	}

	// Check if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is supported
	{
		VkImageUsageFlags required_image_support = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if( !( ( surface_capabilities.supportedUsageFlags & required_image_support ) == required_image_support ) ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Window surface does not support color attachments!" );
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
				surface,
				&surface_format_count,
				nullptr
			);
			surface_formats.resize( surface_format_count );
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device,
				surface,
				&surface_format_count,
				surface_formats.data()
			);
		}
		surface_format = surface_formats[ 0 ];
		if( surface_format.format == VK_FORMAT_UNDEFINED ) {
			surface_format.format			= VK_FORMAT_R8G8B8A8_UNORM;
			surface_format.colorSpace		= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
	}

	return true;
}









bool WindowImpl::CreateRenderPass()
{

	std::array<VkAttachmentDescription, 1>		color_attachment_descriptions {};
	color_attachment_descriptions[ 0 ].flags			= 0;
	color_attachment_descriptions[ 0 ].format			= surface_format.format;
	color_attachment_descriptions[ 0 ].samples			= VkSampleCountFlagBits( create_info_copy.samples );
	color_attachment_descriptions[ 0 ].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_descriptions[ 0 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_descriptions[ 0 ].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_descriptions[ 0 ].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_descriptions[ 0 ].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::array<VkAttachmentReference, 0>		input_attachment_references {};

	std::array<VkAttachmentReference, 1>		color_attachment_references {};
	color_attachment_references[ 0 ].attachment		= 0;	// points to color_attachment_descriptions[ 0 ]
	color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
	subpasses[ 0 ].pResolveAttachments			= create_info_copy.samples == Multisamples::SAMPLE_COUNT_1 ? nullptr : color_attachment_references.data();
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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create render pass!" );
		}
		return false;
	}

	return true;
}









bool WindowImpl::CreateGraphicsPipelines()
{

	pipelines.resize( size_t( _internal::PipelineType::PIPELINE_TYPE_COUNT ) );

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
	shader_stages[ 0 ].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[ 0 ].pNext					= nullptr;
	shader_stages[ 0 ].flags					= 0;
	shader_stages[ 0 ].stage					= VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[ 0 ].module					= renderer_parent->GetVertexShaderModule();
	shader_stages[ 0 ].pName					= "main";
	shader_stages[ 0 ].pSpecializationInfo		= nullptr;

	shader_stages[ 1 ].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[ 1 ].pNext					= nullptr;
	shader_stages[ 1 ].flags					= 0;
	shader_stages[ 1 ].stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[ 1 ].module					= renderer_parent->GetFragmentShaderModule();
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
	viewport.width		= 800;
	viewport.height		= 600;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor {
		{ 0, 0 },
		{ 800, 600 }
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
	multisample_state_create_info.rasterizationSamples		= VkSampleCountFlagBits( create_info_copy.samples );
	multisample_state_create_info.sampleShadingEnable		= VK_FALSE;
	multisample_state_create_info.minSampleShading			= 1.0f;
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
	pipeline_create_info.layout					= renderer_parent->GetPipelineLayout();
	pipeline_create_info.renderPass				= render_pass;
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	// Create filled polygon pipeline
	{
		if( vkCreateGraphicsPipelines(
			device,
			renderer_parent->GetPipelineCache(),
			1,
			&pipeline_create_info,
			nullptr,
			&pipelines[ size_t( _internal::PipelineType::FILLED_POLYGON_LIST ) ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
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
			renderer_parent->GetPipelineCache(),
			1,
			&pipeline_create_info,
			nullptr,
			&pipelines[ size_t( _internal::PipelineType::WIREFRAME_POLYGON_LIST ) ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
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
			renderer_parent->GetPipelineCache(),
			1,
			&pipeline_create_info,
			nullptr,
			&pipelines[ size_t( _internal::PipelineType::LINE_LIST ) ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
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
			renderer_parent->GetPipelineCache(),
			1,
			&pipeline_create_info,
			nullptr,
			&pipelines[ size_t( _internal::PipelineType::POINT_LIST ) ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create graphics pipeline!" );
			}
			return false;
		}
	}

	return true;
}









bool WindowImpl::CreateCommandPool()
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
		&command_pool
	) != VK_SUCCESS ) {
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan command pool!" );
		}
		return false;
	}

	return true;
}









bool WindowImpl::AllocateCommandBuffers()
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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot allocate command buffers!" );
		}
		return false;
	}

	for( size_t i = 0; i < swapchain_image_count; ++i ) {
		render_command_buffers[ i ]		= temp[ i ];
	}
	mesh_transfer_command_buffer		= temp[ swapchain_image_count ];

	return true;
}









bool WindowImpl::CreateSwapchain()
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
				create_info_copy.min_width,
				create_info_copy.min_height
			};
			max_extent		= {
				create_info_copy.max_width,
				create_info_copy.max_height
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
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physical_device,
				surface,
				&surface_capabilities
			);

			extent	= surface_capabilities.currentExtent;
		}

		// Figure out present mode
		{
			bool present_mode_found		= false;
			if( create_info_copy.vsync ) {
				// Using VSync we should use FIFO, this mode is required to be supported so we can rely on that and just use it without checking
				present_mode		= VK_PRESENT_MODE_FIFO_KHR;
				present_mode_found		= true;
			} else {
				// Not using VSync, we should try mailbox first and immediate second, fall back to FIFO if neither is supported
				std::vector<VkPresentModeKHR> surface_present_modes;
				{
					uint32_t present_mode_count = 0;
					vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						surface,
						&present_mode_count,
						nullptr
					);
					surface_present_modes.resize( present_mode_count );
					vkGetPhysicalDeviceSurfacePresentModesKHR(
						physical_device,
						surface,
						&present_mode_count,
						surface_present_modes.data()
					);
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
			swapchain_create_info.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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
				if( report_function ) {
					report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create vulkan swapchain!" );
				}
				return false;
			}
		}

		// Get swapchain images and create image views
		{
			uint32_t swapchain_image_count = 0;
			vkGetSwapchainImagesKHR(
				device,
				swapchain,
				&swapchain_image_count,
				nullptr
			);
			swapchain_images.resize( swapchain_image_count );
			vkGetSwapchainImagesKHR(
				device,
				swapchain,
				&swapchain_image_count,
				swapchain_images.data()
			);

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
					if( report_function ) {
						report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create image views for swapchain!" );
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

	recreate_swapchain		= false;

	return true;
}









bool WindowImpl::CreateFramebuffers()
{
	framebuffers.resize( swapchain_image_count );

	for( uint32_t i = 0; i < swapchain_image_count; ++i ) {

		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext				= nullptr;
		framebuffer_create_info.flags				= 0;
		framebuffer_create_info.renderPass			= render_pass;
		framebuffer_create_info.attachmentCount		= 1;
		framebuffer_create_info.pAttachments		= &swapchain_image_views[ i ];
		framebuffer_create_info.width				= extent.width;
		framebuffer_create_info.height				= extent.height;
		framebuffer_create_info.layers				= 1;

		if( vkCreateFramebuffer(
			device,
			&framebuffer_create_info,
			nullptr,
			&framebuffers[ i ]
		) != VK_SUCCESS ) {
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create framebuffers!" );
			}
			return false;
		}
	}

	return true;
}









bool WindowImpl::CreateWindowSynchronizationPrimitives()
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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create image aquisition fence!" );
		}
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
		if( report_function ) {
			report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create mesh transfer semaphore!" );
		}
		return false;
	}

	return true;
}









bool WindowImpl::CreateFrameSynchronizationPrimitives()
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create synchronization semaphores!" );
			}
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
			if( report_function ) {
				report_function( ReportSeverity::NON_CRITICAL_ERROR, "Cannot create synchronization fences!" );
			}
			return false;
		}
	}

	return true;
}

void WindowImpl::CmdBindPipeline(
	VkCommandBuffer						command_buffer,
	_internal::PipelineType				pipeline_type
)
{
	if( previous_pipeline_type != pipeline_type ) {
		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelines[ uint32_t( pipeline_type ) ]
		);
		previous_pipeline_type = pipeline_type;
	}
}









} // _internal

} // vk2d
