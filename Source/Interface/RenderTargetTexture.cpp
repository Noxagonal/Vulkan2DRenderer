
#include "../Core/SourceCommon.h"

#include "../../Include/Types/Vector2.h"
#include "../../Include/Types/Rect2.h"
#include "../../Include/Types/Color.h"
#include "../../Include/Types/Mesh.h"

#include "../System/RenderTargetTextureDependecyGraphInfo.hpp"
#include "../System/ShaderInterface.h"
#include "../System/MeshBuffer.h"

#include "../../Include/Interface/Instance.h"
#include "InstanceImpl.h"

#include "../../Include/Interface/Texture.h"
#include "TextureImpl.h"

#include "../../Include/Interface/RenderTargetTexture.h"
#include "RenderTargetTextureImpl.h"

#include "../../Include/Interface/Sampler.h"
#include "SamplerImpl.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::RenderTargetTexture::RenderTargetTexture(
	vk2d::_internal::InstanceImpl						*	instance,
	const vk2d::RenderTargetTextureCreateInfo			&	create_info
)
{
	impl = std::make_unique<vk2d::_internal::RenderTargetTextureImpl>(
		instance,
		create_info
	);
	if( !impl || !impl->IsGood() ) {
		impl			= nullptr;
	}

	texture_impl	= impl.get();
}


VK2D_API vk2d::RenderTargetTexture::~RenderTargetTexture()
{}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::SetSize(
	vk2d::Vector2u		new_size
)
{
	impl->SetSize(
		new_size
	);
}

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::RenderTargetTexture::GetSize() const
{
	return impl->GetSize();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::RenderTargetTexture::GetLayerCount() const
{
	return impl->GetLayerCount();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::WaitUntilLoaded()
{
	return impl->WaitUntilLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::IsLoaded()
{
	return impl->IsLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::BeginRender()
{
	return impl->BeginRender();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::EndRender(
	vk2d::Vector2f						blur_amount,
	vk2d::RenderTargetTextureBlurType	blur_type
)
{
	return impl->EndRender( blur_type, blur_amount );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawTriangleList(
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	bool										filled,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawTriangleList(
		indices,
		vertices,
		texture_channels,
		filled,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawLineList(
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawLineList(
		indices,
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawPointList(
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPoint(
	vk2d::Vector2f			location,
	vk2d::Colorf			color,
	float					size
)
{
	auto mesh = vk2d::GeneratePointMeshFromList(
		{ location }
	);
	mesh.SetVertexColor( color );
	mesh.SetPointSize( size );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawLine(
	vk2d::Vector2f					point_1,
	vk2d::Vector2f					point_2,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateLineMeshFromList(
		{ point_1, point_2 },
		{ { 0, 1 } }
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawRectangle(
	vk2d::Rect2f					area,
	bool							filled,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateRectangleMesh(
		area,
		filled
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawEllipse(
	vk2d::Rect2f					area,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateEllipseMesh(
		area,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawEllipsePie(
	vk2d::Rect2f					area,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateEllipsePieMesh(
		area,
		begin_angle_radians,
		coverage,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawRectanglePie(
	vk2d::Rect2f					area,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateRectanglePieMesh(
		area,
		begin_angle_radians,
		coverage,
		filled
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawTexture(
	vk2d::Vector2f				top_left,
	vk2d::Texture			*	texture,
	vk2d::Colorf				color
)
{
	if( texture ) {
		auto texture_size = texture->GetSize();
		auto bottom_right = top_left + vk2d::Vector2f( float( texture_size.x ), float( texture_size.y ) );
		auto mesh = vk2d::GenerateRectangleMesh(
			{ top_left, bottom_right }
		);
		mesh.SetTexture( texture );
		mesh.SetVertexColor( color );
		impl->DrawMesh( mesh );
	}
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawMesh(
	const vk2d::Mesh	&	mesh
)
{
	impl->DrawMesh(
		mesh
	);
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::IsGood() const
{
	return !!impl;
}







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







vk2d::_internal::RenderTargetTextureImpl::RenderTargetTextureImpl(
	vk2d::_internal::InstanceImpl				*	instance,
	const vk2d::RenderTargetTextureCreateInfo	&	create_info
)
{
	assert( instance );
	if( !instance->IsThisThreadCreatorThread() ) {
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Cannot create render target texture, this needs to be created from the main thread." );
		return;
	}

	this->instance			= instance;
	this->create_info_copy	= create_info;
	this->surface_format	= VK_FORMAT_R8G8B8A8_UNORM;

	this->samples			= CheckSupportedMultisampleCount( instance, create_info_copy.samples );


	if( !DetermineType() ) return;
	if( !CreateCommandBuffers() ) return;
	if( !CreateFrameDataBuffers() ) return;
	if( !CreateRenderPasses() ) return;
	if( !CreateImages( create_info.size ) ) return;
	if( !CreateFramebuffers() ) return;
	if( !CreateSynchronizationPrimitives() ) return;

	mesh_buffer		= std::make_unique<vk2d::_internal::MeshBuffer>(
		instance,
		instance->GetVulkanDevice(),
		instance->GetVulkanPhysicalDeviceProperties().limits,
		instance->GetDeviceMemoryPool()
		);

		// Initial final image layouts, change later if implementing mipmapless render target texture.
	vk_attachment_image_final_layout	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	vk_sampled_image_final_layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vk_sampled_image_final_access_mask	= VK_ACCESS_SHADER_READ_BIT;

	is_good					= true;
}


vk2d::_internal::RenderTargetTextureImpl::~RenderTargetTextureImpl()
{
	WaitIdle();

	DestroySynchronizationPrimitives();
	DestroyFramebuffers();
	DestroyImages();
	DestroyRenderPasses();
	DestroyFrameDataBuffers();
	DestroyCommandBuffers();
}

void vk2d::_internal::RenderTargetTextureImpl::SetSize(
	vk2d::Vector2u			new_size
)
{
	if( size != new_size ) {
		// TODO: Consider a better synchronization for RenderTargetTextureImpl::SetSize().
		// Consider making surfaces their own unit and when changing size, next render would
		// immediately use the new size but the old surfaces would be kept around until all
		// rendering operations have finished on them.
		{
			std::array<VkSemaphore, std::tuple_size_v<decltype( swap_buffers )>> semaphores;
			std::array<uint64_t, std::tuple_size_v<decltype( swap_buffers )>> semaphore_values;
			for( size_t i = 0; i < std::size( swap_buffers ); ++i ) {
				//semaphores[ i ]			= create_info_copy.enable_blur ? swap_buffers[ i ].vk_mipmap_complete_semaphore : swap_buffers[ i ].vk_render_complete_semaphore;
				semaphores[ i ]			= swap_buffers[ i ].vk_render_complete_semaphore;
				semaphore_values[ i ]	= 1;
			}
			VkSemaphoreWaitInfo wait_info {};
			wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
			wait_info.pNext				= nullptr;
			wait_info.flags				= 0;
			wait_info.semaphoreCount	= uint32_t( std::size( semaphores ) );
			wait_info.pSemaphores		= semaphores.data();
			wait_info.pValues			= semaphore_values.data();

			using namespace std::chrono;
			using namespace std::chrono_literals;

			auto result = vkWaitSemaphores(
				instance->GetVulkanDevice(),
				&wait_info,
				duration_cast<nanoseconds>( 5s ).count()
			);
			if( result != VK_SUCCESS ) {
				instance->Report( result, "Internal error: Error destroying RenderTargetTexture, timeout while waiting for render to finish!" );
			}
		}

		DestroyImages();
		DestroyFramebuffers();

		CreateImages(
			new_size
		);
		CreateFramebuffers();
	}
}

vk2d::Vector2u vk2d::_internal::RenderTargetTextureImpl::GetSize() const
{
	return size;
}

uint32_t vk2d::_internal::RenderTargetTextureImpl::GetLayerCount() const
{
	return 1;
}

uint32_t vk2d::_internal::RenderTargetTextureImpl::GetCurrentSwapBuffer() const
{
	return current_swap_buffer;
}

VkImage vk2d::_internal::RenderTargetTextureImpl::GetVulkanImage() const
{
	return swap_buffers[ current_swap_buffer ].sampled_image.image;
}

VkImageView vk2d::_internal::RenderTargetTextureImpl::GetVulkanImageView() const
{
	return swap_buffers[ current_swap_buffer ].sampled_image.view;
}

VkImageLayout vk2d::_internal::RenderTargetTextureImpl::GetVulkanImageLayout() const
{
	return vk_sampled_image_final_layout;
}

VkFramebuffer vk2d::_internal::RenderTargetTextureImpl::GetFramebuffer(
	vk2d::_internal::RenderTargetTextureDependencyInfo			&	dependency_info
) const
{
	assert( dependency_info.render_target == this );
	return swap_buffers[ dependency_info.swap_buffer_index ].vk_render_framebuffer;
}

VkSemaphore vk2d::_internal::RenderTargetTextureImpl::GetAllCompleteSemaphore(
	vk2d::_internal::RenderTargetTextureDependencyInfo		&	dependency_info
) const
{
	assert( dependency_info.render_target == this );
	return swap_buffers[ dependency_info.swap_buffer_index ].vk_render_complete_semaphore;
}

uint64_t vk2d::_internal::RenderTargetTextureImpl::GetRenderCounter(
	vk2d::_internal::RenderTargetTextureDependencyInfo			&	dependency_info
) const
{
	assert( dependency_info.render_target == this );
	return swap_buffers[ dependency_info.swap_buffer_index ].render_counter;
}

bool vk2d::_internal::RenderTargetTextureImpl::WaitUntilLoaded()
{
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::IsLoaded()
{
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::BeginRender()
{
	auto result = VK_SUCCESS;

	++current_swap_buffer;
	if( current_swap_buffer >= uint32_t( std::size( swap_buffers ) ) ) current_swap_buffer = 0;

	auto & swap = swap_buffers[ current_swap_buffer ];

	{
		// Wait here for the semaphore to signal, this takes care of synchronization
		// just in case buffer we're trying to modify hasn't finished rendering.
		if( !SynchronizeFrame() ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, synchronization error!" );
			return false;
		}

		// We no longer contain sampled image that's ready to be used without synchronization.
		swap.contains_non_pending_sampled_image = false;

		ResetRenderTargetTextureRenderDependencies( current_swap_buffer );
	}

	// Begin command buffer
	{
		VkCommandBuffer	command_buffer	= swap.vk_render_command_buffer;

		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext				= nullptr;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo	= nullptr;

		result = vkBeginCommandBuffer(
			command_buffer,
			&command_buffer_begin_info
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot render to RenderTargetTexture, Cannot record primary render command buffer!" );
			return false;
		}
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"RenderTargetTextureImpl",
			vk2d::_internal::CommandBufferCheckpointType::BEGIN_COMMAND_BUFFER
		);

		// Set viewport, scissor and initial line width
		{
			VkViewport viewport {};
			viewport.x			= 0;
			viewport.y			= 0;
			viewport.width		= float( size.x );
			viewport.height		= float( size.y );
			viewport.minDepth	= 0.0f;
			viewport.maxDepth	= 1.0f;
			vkCmdSetViewport(
				command_buffer,
				0, 1, &viewport
			);

			VkRect2D scissor {
				{ 0, 0 },
				{ size.x, size.y }
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
				instance->GetGraphicsPrimaryRenderPipelineLayout(),
				GRAPHICS_DESCRIPTOR_SET_ALLOCATION_WINDOW_FRAME_DATA,
				1, &frame_data_descriptor_set.descriptorSet,
				0, nullptr
			);
		}

		// Begin render pass
		{
			std::array<VkClearValue, 2> clear_values;
			clear_values[ 0 ].color.float32[ 0 ]		= 0.0f;
			clear_values[ 0 ].color.float32[ 1 ]		= 0.0f;
			clear_values[ 0 ].color.float32[ 2 ]		= 0.0f;
			clear_values[ 0 ].color.float32[ 3 ]		= 0.0f;

			clear_values[ 1 ].color.float32[ 0 ]		= 0.0f;
			clear_values[ 1 ].color.float32[ 1 ]		= 0.0f;
			clear_values[ 1 ].color.float32[ 2 ]		= 0.0f;
			clear_values[ 1 ].color.float32[ 3 ]		= 0.0f;

			VkRenderPassBeginInfo render_pass_begin_info {};
			render_pass_begin_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext			= nullptr;
			render_pass_begin_info.renderPass		= vk_attachment_render_pass;
			render_pass_begin_info.framebuffer		= swap.vk_render_framebuffer;
			render_pass_begin_info.renderArea		= { { 0, 0 }, { size.x, size.y } };
			render_pass_begin_info.clearValueCount	= uint32_t( std::size( clear_values ) );
			render_pass_begin_info.pClearValues		= clear_values.data();

			vk2d::_internal::CmdInsertCommandBufferCheckpoint(
				command_buffer,
				"RenderTargetTextureImpl",
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

bool vk2d::_internal::RenderTargetTextureImpl::EndRender(
	vk2d::RenderTargetTextureBlurType	blur_type,
	vk2d::Vector2f						blur_amount
)
{
	auto & swap									= swap_buffers[ current_swap_buffer ];
	VkCommandBuffer		render_command_buffer	= swap.vk_render_command_buffer;
	VkCommandBuffer		transfer_command_buffer	= swap.vk_transfer_command_buffer;
	//VkCommandBuffer		blur_command_buffer		= swap.vk_blur_command_buffer;

	// End render pass.
	{
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			render_command_buffer,
			"RenderTargetTextureImpl",
			vk2d::_internal::CommandBufferCheckpointType::END_RENDER_PASS
		);
		vkCmdEndRenderPass( render_command_buffer );
	}
	CmdFinalizeRender( swap, blur_type, blur_amount );

	// End command buffer
	vk2d::_internal::CmdInsertCommandBufferCheckpoint(
		render_command_buffer,
		"RenderTargetTextureImpl",
		vk2d::_internal::CommandBufferCheckpointType::END_COMMAND_BUFFER
	);
	auto result = vkEndCommandBuffer(
		render_command_buffer
	);
	if( result != VK_SUCCESS ) {
		instance->Report( result, "Internal error: Cannot render to RenderTargetTexture, Cannot compile primary render command buffer!" );
		return false;
	}

	RecordTransferCommandBuffer( swap );

	++swap.render_counter;

	previous_graphics_pipeline_settings	= {};
	previous_texture					= {};
	previous_sampler					= {};
	previous_line_width					= 1.0f;

	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::SynchronizeFrame()
{
	auto result = VK_SUCCESS;

	using namespace std::chrono;
	using namespace std::chrono_literals;

	auto & swap = swap_buffers[ current_swap_buffer ];

	if( swap.has_been_submitted ) {
		VkSemaphoreWaitInfo semaphore_wait_info {};
		semaphore_wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		semaphore_wait_info.pNext				= nullptr;
		semaphore_wait_info.flags				= 0;
		semaphore_wait_info.semaphoreCount		= 1;
		//semaphore_wait_info.pSemaphores			= create_info_copy.enable_blur ? &swap.vk_mipmap_complete_semaphore : &swap.vk_render_complete_semaphore;
		semaphore_wait_info.pSemaphores			= &swap.vk_render_complete_semaphore;
		semaphore_wait_info.pValues				= &swap.render_counter;
		result = vkWaitSemaphores(
			instance->GetVulkanDevice(),
			&semaphore_wait_info,
			duration_cast<nanoseconds>( 5s ).count()
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Cannot synchronize RenderTargetTexture frame, Semaphore wait timeout!" );
			return false;
		}

		swap.has_been_submitted = false;
	}
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::WaitIdle()
{
	std::vector<VkSemaphore>	semaphores;
	std::vector<uint64_t>		semaphore_values;
	semaphores.reserve( std::size( swap_buffers ) );
	semaphore_values.reserve( std::size( swap_buffers ) );

	for( auto & s : swap_buffers ) {
		if( s.has_been_submitted ) {
			//semaphores.push_back( create_info_copy.enable_blur ? s.vk_mipmap_complete_semaphore : s.vk_render_complete_semaphore );
			semaphores.push_back( s.vk_render_complete_semaphore );
			semaphore_values.push_back( s.render_counter );
		}
	}
	if( std::size( semaphores ) ) {

		VkSemaphoreWaitInfo wait_info {};
		wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		wait_info.pNext				= nullptr;
		wait_info.flags				= 0;
		wait_info.semaphoreCount	= uint32_t( std::size( semaphores ) );
		wait_info.pSemaphores		= semaphores.data();
		wait_info.pValues			= semaphore_values.data();

		using namespace std::chrono;
		using namespace std::chrono_literals;

		auto result = vkWaitSemaphores(
			instance->GetVulkanDevice(),
			&wait_info,
			duration_cast<nanoseconds>( 5s ).count()
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Error waiting RenderTargetTexture to become idle, timeout while waiting!" );
			return false;
		}
	}
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::CommitRenderTargetTextureRender(
	vk2d::_internal::RenderTargetTextureDependencyInfo	&	dependency_info,
	vk2d::_internal::RenderTargetTextureRenderCollector	&	collector
)
{
	assert( dependency_info.render_target == this );

	auto & swap = swap_buffers[ dependency_info.swap_buffer_index ];

	if( !swap.contains_non_pending_sampled_image ) {

		//std::lock_guard<std::mutex> lock_guard( swap.render_commitment_request_mutex );

		if( swap.render_commitment_request_count == 0 ) {

			for( auto & d : swap.render_target_texture_dependencies ) {
				if( !d.render_target->CommitRenderTargetTextureRender( d, collector ) ) {
					return false;
				}
			}

			// Give this render info to the collector if this render target texture data is out of date.
			// Basically if we're pending render and render commitment count is 0.
			{
				// Get immediate children render complete semaphores.
				std::vector<VkSemaphore>			wait_for_semaphores;				wait_for_semaphores.reserve( std::size( swap.render_target_texture_dependencies ) );
				std::vector<uint64_t>				wait_for_semaphore_timeline_values;	wait_for_semaphore_timeline_values.reserve( std::size( swap.render_target_texture_dependencies ) );
				std::vector<VkPipelineStageFlags>	wait_for_semaphore_pipeline_stages;	wait_for_semaphore_pipeline_stages.reserve( std::size( swap.render_target_texture_dependencies ) );
				for( auto & d : swap.render_target_texture_dependencies ) {
					wait_for_semaphores.push_back( d.render_target->GetAllCompleteSemaphore( d ) );
					wait_for_semaphore_timeline_values.push_back( d.render_target->GetRenderCounter( d ) );
					// TODO: Replace VK_PIPELINE_STAGE_ALL_COMMANDS_BIT with something that narrows down the potential pipeline bubble more.
					wait_for_semaphore_pipeline_stages.push_back( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
				}

				// Update submit info to account for render dependencies.
				if( !UpdateSubmitInfos(
					swap,
					wait_for_semaphores,
					wait_for_semaphore_timeline_values,
					wait_for_semaphore_pipeline_stages
				) ) {
					return false;
				}

				// Append to render collector.
				collector.Append(
					&swap.vk_transfer_submit_info,
					&swap.vk_render_submit_info
					//create_info_copy.enable_blur ? &swap.vk_blur_submit_info : nullptr,
					//create_info_copy.enable_blur ? &swap.vk_mipmap_submit_info : nullptr
				);
			}
		}

		++swap.render_commitment_request_count;
	}
	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::ConfirmRenderTargetTextureRenderSubmission(
	vk2d::_internal::RenderTargetTextureDependencyInfo	&	dependency_info
)
{
	assert( dependency_info.render_target == this );

	auto & swap = swap_buffers[ dependency_info.swap_buffer_index ];
	swap.has_been_submitted = true;

	for( auto & d : swap.render_target_texture_dependencies ) {
		d.render_target->ConfirmRenderTargetTextureRenderSubmission( d );
	}
}

void vk2d::_internal::RenderTargetTextureImpl::ConfirmRenderTargetTextureRenderFinished(
	vk2d::_internal::RenderTargetTextureDependencyInfo	&	dependency_info
)
{
	assert( dependency_info.render_target == this );
	auto & swap = swap_buffers[ dependency_info.swap_buffer_index ];

	swap.contains_non_pending_sampled_image	= true;

	for( auto & d : swap.render_target_texture_dependencies ) {
		d.render_target->ConfirmRenderTargetTextureRenderFinished( d );
	}
}

void vk2d::_internal::RenderTargetTextureImpl::AbortRenderTargetTextureRender(
	vk2d::_internal::RenderTargetTextureDependencyInfo	&	dependency_info
)
{
	assert( dependency_info.render_target == this );

	auto & swap = swap_buffers[ dependency_info.swap_buffer_index ];

	{
		//std::lock_guard<std::mutex> lock_guard( swap.render_commitment_request_mutex );

		swap.has_been_submitted = false;

		--swap.render_commitment_request_count;
		assert( swap.render_commitment_request_count != UINT32_MAX );
	}

	for( auto & d : swap.render_target_texture_dependencies ) {
		d.render_target->AbortRenderTargetTextureRender( d );
	}
}

void vk2d::_internal::RenderTargetTextureImpl::ResetRenderTargetTextureRenderDependencies(
	uint32_t				swap_buffer_index
)
{
	auto & swap = swap_buffers[ swap_buffer_index ];

	//std::lock_guard<std::mutex> lock_guard( swap.render_commitment_request_mutex );

	swap.render_commitment_request_count	= 0;
	swap.render_target_texture_dependencies.clear();
	swap.render_wait_for_semaphores.clear();
	swap.render_wait_for_semaphore_timeline_values.clear();
	swap.render_wait_for_pipeline_stages.clear();
}

void vk2d::_internal::RenderTargetTextureImpl::CheckAndAddRenderTargetTextureDependency(
	uint32_t				swap_buffer_index,
	vk2d::Texture		*	texture
)
{
	auto & swap		= swap_buffers[ swap_buffer_index ];

	// TODO: Investigate a need for reference count of some sort. Render target can have dependencies to multiple different parents.
	if( auto render_target = dynamic_cast<vk2d::_internal::RenderTargetTextureImpl*>( texture->texture_impl ) ) {

		//std::lock_guard<std::mutex> lock_guard( swap.render_commitment_request_mutex );

		if( std::none_of(
			swap.render_target_texture_dependencies.begin(),
			swap.render_target_texture_dependencies.end(),
			[render_target]( vk2d::_internal::RenderTargetTextureDependencyInfo & rt )
			{
				if( render_target == rt.render_target ) return true;
				return false;
			} ) ) {
			swap.render_target_texture_dependencies.push_back( render_target->GetDependencyInfo() );
		}
	}
}

vk2d::_internal::RenderTargetTextureDependencyInfo vk2d::_internal::RenderTargetTextureImpl::GetDependencyInfo()
{
	vk2d::_internal::RenderTargetTextureDependencyInfo dependency_info {};
	dependency_info.render_target		= this;
	dependency_info.swap_buffer_index	= current_swap_buffer;
	return dependency_info;
}

void vk2d::_internal::RenderTargetTextureImpl::DrawTriangleList(
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	bool										filled,
	vk2d::Texture							*	texture,
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
		texture_channel_weights,
		filled,
		texture,
		sampler
	);
}

void vk2d::_internal::RenderTargetTextureImpl::DrawTriangleList(
	const std::vector<uint32_t>				&	raw_indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	bool										solid,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	auto & swap				= swap_buffers[ current_swap_buffer ];
	auto command_buffer		= swap.vk_render_command_buffer;

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( raw_indices.size() );

	if( !texture ) {
		texture = instance->GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance->GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency(
		current_swap_buffer,
		texture
	);

	{
		bool multitextured = texture->GetLayerCount() > 1 &&
			texture_channel_weights.size() >= texture->GetLayerCount() * vertices.size();

		auto graphics_shader_programs = instance->GetCompatibleGraphicsShaderModules(
			multitextured,
			sampler->impl->IsAnyBorderColorEnabled(),
			3
		);

		vk2d::_internal::GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout	= instance->GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass		= vk_attachment_render_pass;
		pipeline_settings.primitive_topology	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline_settings.polygon_mode			= solid ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
		pipeline_settings.shader_programs		= graphics_shader_programs;
		pipeline_settings.samples				= VkSampleCountFlags( samples );
		pipeline_settings.enable_blending		= VK_TRUE;

		CmdBindGraphicsPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		vertices,
		texture_channel_weights
	);

	if( push_result.success ) {
		{
			vk2d::_internal::GraphicsPrimaryRenderPushConstants pc {};
			pc.index_offset				= push_result.location_info.index_offset;
			pc.index_count				= 3;
			pc.vertex_offset			= push_result.location_info.vertex_offset;
			pc.texture_channel_offset	= push_result.location_info.texture_channel_offset;
			pc.texture_channel_count	= texture->GetLayerCount();

			vkCmdPushConstants(
				command_buffer,
				instance->GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::DRAW
		);
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			1,
			push_result.location_info.index_offset,
			int32_t( push_result.location_info.vertex_offset ),
			0
		);
	} else {
		instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}

	#if VK2D_BUILD_OPTION_DEBUG_ALWAYS_DRAW_TRIANGLES_WIREFRAME
	if( solid ) {
		auto vertices_copy = vertices;
		for( auto & v : vertices_copy ) {
			v.color = vk2d::Colorf( 0.2f, 1.0f, 0.4f, 0.25f );
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

void vk2d::_internal::RenderTargetTextureImpl::DrawLineList(
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	vk2d::Texture							*	texture,
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
		texture_channel_weights,
		texture,
		sampler,
		line_width
	);
}

void vk2d::_internal::RenderTargetTextureImpl::DrawLineList(
	const std::vector<uint32_t>				&	raw_indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler,
	float										line_width
)
{
	auto & swap			= swap_buffers[ current_swap_buffer ];
	auto command_buffer	= swap.vk_render_command_buffer;

	auto vertex_count	= uint32_t( vertices.size() );
	auto index_count	= uint32_t( raw_indices.size() );

	if( !texture ) {
		texture = instance->GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance->GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency(
		current_swap_buffer,
		texture
	);

	{
		bool multitextured = texture->GetLayerCount() > 1 &&
			texture_channel_weights.size() >= texture->GetLayerCount() * vertices.size();

		auto graphics_shader_programs = instance->GetCompatibleGraphicsShaderModules(
			multitextured,
			sampler->impl->IsAnyBorderColorEnabled(),
			2
		);

		vk2d::_internal::GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout	= instance->GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass		= vk_attachment_render_pass;
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

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		raw_indices,
		vertices,
		texture_channel_weights
	);

	if( push_result.success ) {
		{
			vk2d::_internal::GraphicsPrimaryRenderPushConstants pc {};
			pc.index_offset				= push_result.location_info.index_offset;
			pc.index_count				= 2;
			pc.vertex_offset			= push_result.location_info.vertex_offset;
			pc.texture_channel_offset	= push_result.location_info.texture_channel_offset;
			pc.texture_channel_count	= texture->GetLayerCount();

			vkCmdPushConstants(
				command_buffer,
				instance->GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::DRAW
		);
		vkCmdDrawIndexed(
			command_buffer,
			index_count,
			1,
			push_result.location_info.index_offset,
			int32_t( push_result.location_info.vertex_offset ),
			0
		);
	} else {
		instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
}

void vk2d::_internal::RenderTargetTextureImpl::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channel_weights,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	auto & swap			= swap_buffers[ current_swap_buffer ];
	auto command_buffer	= swap.vk_render_command_buffer;

	auto vertex_count	= uint32_t( vertices.size() );

	if( !texture ) {
		texture = instance->GetDefaultTexture();
	}
	if( !sampler ) {
		sampler = instance->GetDefaultSampler();
	}

	CheckAndAddRenderTargetTextureDependency(
		current_swap_buffer,
		texture
	);

	{
		bool multitextured = texture->GetLayerCount() > 1 &&
			texture_channel_weights.size() >= texture->GetLayerCount() * vertices.size();

		auto graphics_shader_programs = instance->GetCompatibleGraphicsShaderModules(
			multitextured,
			sampler->impl->IsAnyBorderColorEnabled(),
			1
		);

		vk2d::_internal::GraphicsPipelineSettings pipeline_settings {};
		pipeline_settings.vk_pipeline_layout	= instance->GetGraphicsPrimaryRenderPipelineLayout();
		pipeline_settings.vk_render_pass		= vk_attachment_render_pass;
		pipeline_settings.primitive_topology	= VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		pipeline_settings.polygon_mode			= VK_POLYGON_MODE_POINT;
		pipeline_settings.shader_programs		= graphics_shader_programs;
		pipeline_settings.samples				= VkSampleCountFlags( samples );
		pipeline_settings.enable_blending		= VK_TRUE;

		CmdBindGraphicsPipelineIfDifferent(
			command_buffer,
			pipeline_settings
		);
	}

	CmdBindSamplerIfDifferent(
		command_buffer,
		sampler,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);
	CmdBindTextureIfDifferent(
		command_buffer,
		texture,
		instance->GetGraphicsPrimaryRenderPipelineLayout()
	);

	auto push_result = mesh_buffer->CmdPushMesh(
		command_buffer,
		{},
		vertices,
		texture_channel_weights
	);

	if( push_result.success ) {
		{
			vk2d::_internal::GraphicsPrimaryRenderPushConstants pc {};
			pc.index_offset				= push_result.location_info.index_offset;
			pc.index_count				= 1;
			pc.vertex_offset			= push_result.location_info.vertex_offset;
			pc.texture_channel_offset	= push_result.location_info.texture_channel_offset;
			pc.texture_channel_count	= texture->GetLayerCount();

			vkCmdPushConstants(
				command_buffer,
				instance->GetGraphicsPrimaryRenderPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof( pc ),
				&pc
			);
		}

		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			command_buffer,
			"MeshBuffer",
			vk2d::_internal::CommandBufferCheckpointType::DRAW
		);
		vkCmdDraw(
			command_buffer,
			vertex_count,
			1,
			push_result.location_info.vertex_offset,
			0
		);
	} else {
		instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot push mesh into mesh render queue!" );
	}
}

void vk2d::_internal::RenderTargetTextureImpl::DrawMesh(
	const vk2d::Mesh	&	mesh
)
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

bool vk2d::_internal::RenderTargetTextureImpl::IsGood() const
{
	return is_good;
}

bool vk2d::_internal::RenderTargetTextureImpl::DetermineType()
{
	assert( instance );
	if( samples == vk2d::Multisamples( 0 ) ) {
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Cannot create RenderTargetTexture, multisamples was set to 0, must be 1 or higher!" );
		return false;
	}

	if( samples == vk2d::Multisamples::SAMPLE_COUNT_1 ) {
		// no multisamples
		if( create_info_copy.enable_blur ) {
			// with blur
			type = vk2d::_internal::RenderTargetTextureType::WITH_BLUR;
		} else {
			// no blur
			type = vk2d::_internal::RenderTargetTextureType::DIRECT;
		}
	} else {
		// with multisamples
		if( create_info_copy.enable_blur ) {
			// with blur
			type = vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE_AND_BLUR;
		} else {
			// no blur
			type = vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE;
		}
	}
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::CreateCommandBuffers()
{
	auto render_queue_family_index = instance->GetPrimaryRenderQueue().GetQueueFamilyIndex();
	auto compute_queue_family_index = instance->GetPrimaryComputeQueue().GetQueueFamilyIndex();

	{
		VkCommandPoolCreateInfo command_pool_create_info {};
		command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext				= nullptr;
		command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		command_pool_create_info.queueFamilyIndex	= render_queue_family_index;
		auto result = vkCreateCommandPool(
			instance->GetVulkanDevice(),
			&command_pool_create_info,
			nullptr,
			&vk_graphics_command_pool
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create graphics command pool!" );
			return false;
		}
	}

	// Allocate transfer and render command buffers.
	// We're using the render queue to transfer vertex data to the GPU,
	// this needs to change if we'll ever use transfer queue to transfer the data.
	{
		std::vector<VkCommandBuffer> allocated_command_buffers;
		allocated_command_buffers.resize( swap_buffers.size() * 2 );
		{
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext				= nullptr;
			command_buffer_allocate_info.commandPool		= vk_graphics_command_pool;
			command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount	= uint32_t( allocated_command_buffers.size() );
			auto result = vkAllocateCommandBuffers(
				instance->GetVulkanDevice(),
				&command_buffer_allocate_info,
				allocated_command_buffers.data()
			);
			if( result != VK_SUCCESS ) {
				instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create command pool!" );
				return false;
			}
		}

		{
			uint32_t cb_counter = 0;
			for( auto & s : swap_buffers ) {
				s.vk_render_command_buffer		= allocated_command_buffers[ cb_counter++ ];
			}
			for( auto & s : swap_buffers ) {
				s.vk_transfer_command_buffer	= allocated_command_buffers[ cb_counter++ ];
			}
		}
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroyCommandBuffers()
{
	vkDestroyCommandPool(
		instance->GetVulkanDevice(),
		vk_graphics_command_pool,
		nullptr
	);

	vk_graphics_command_pool	= {};
}

bool vk2d::_internal::RenderTargetTextureImpl::CreateFrameDataBuffers()
{
	// Create staging and device buffers
	{
		VkBufferCreateInfo staging_buffer_create_info {};
		staging_buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_buffer_create_info.pNext					= nullptr;
		staging_buffer_create_info.flags					= 0;
		staging_buffer_create_info.size						= sizeof( vk2d::_internal::FrameData );
		staging_buffer_create_info.usage					= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		staging_buffer_create_info.queueFamilyIndexCount	= 0;
		staging_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_staging_buffer = instance->GetDeviceMemoryPool()->CreateCompleteBufferResource(
			&staging_buffer_create_info,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		if( frame_data_staging_buffer != VK_SUCCESS ) {
			instance->Report( frame_data_staging_buffer.result, "Internal error. Cannot create staging buffer for FrameData!" );
			return false;
		}

		VkBufferCreateInfo device_buffer_create_info {};
		device_buffer_create_info.sType						= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		device_buffer_create_info.pNext						= nullptr;
		device_buffer_create_info.flags						= 0;
		device_buffer_create_info.size						= sizeof( vk2d::_internal::FrameData );
		device_buffer_create_info.usage						= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		device_buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		device_buffer_create_info.queueFamilyIndexCount		= 0;
		device_buffer_create_info.pQueueFamilyIndices		= nullptr;
		frame_data_device_buffer = instance->GetDeviceMemoryPool()->CreateCompleteBufferResource(
			&device_buffer_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if( frame_data_device_buffer != VK_SUCCESS ) {
			instance->Report( frame_data_device_buffer.result, "Internal error. Cannot create device local buffer for FrameData!" );
			return false;
		}
	}

	// Create descriptor set
	{
		frame_data_descriptor_set	= instance->AllocateDescriptorSet(
			instance->GetGraphicsUniformBufferDescriptorSetLayout()
		);
		if( frame_data_descriptor_set != VK_SUCCESS ) {
			instance->Report( frame_data_descriptor_set.result, "Internal error: Cannot allocate descriptor set for FrameData device buffer!" );
			return false;
		}
		VkDescriptorBufferInfo descriptor_write_buffer_info {};
		descriptor_write_buffer_info.buffer	= frame_data_device_buffer.buffer;
		descriptor_write_buffer_info.offset	= 0;
		descriptor_write_buffer_info.range	= sizeof( vk2d::_internal::FrameData );
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
			instance->GetVulkanDevice(),
			1, &descriptor_write,
			0, nullptr
		);
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroyFrameDataBuffers()
{
	instance->FreeDescriptorSet(
		frame_data_descriptor_set
	);
	instance->GetDeviceMemoryPool()->FreeCompleteResource(
		frame_data_device_buffer
	);
	instance->GetDeviceMemoryPool()->FreeCompleteResource(
		frame_data_staging_buffer
	);
}



bool vk2d::_internal::RenderTargetTextureImpl::CreateImages(
	vk2d::Vector2u		new_size
)
{
	assert( instance );

	auto CreateLocalImageResource =[ this ](
		VkImageUsageFlags										usage,
		const std::vector<vk2d::_internal::ResolvedQueue*>	&	used_in_queues,
		VkSampleCountFlagBits									samples				= VK_SAMPLE_COUNT_1_BIT,
		const std::vector<VkExtent2D>						&	mip_levels			= { { 1, 1 } },
		bool													is_array_texture	= false
		) -> vk2d::_internal::CompleteImageResource
	{
		std::vector<uint32_t> unique_queue_family_indices;
		unique_queue_family_indices.clear();
		for( auto q : used_in_queues ) {
			if( std::find( unique_queue_family_indices.begin(), unique_queue_family_indices.end(), q->GetQueueFamilyIndex() ) == unique_queue_family_indices.end() ) {
				unique_queue_family_indices.push_back( q->GetQueueFamilyIndex() );
			}
		}

		VkImageCreateInfo image_create_info {};
		image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.pNext						= nullptr;
		image_create_info.flags						= 0;
		image_create_info.imageType					= VK_IMAGE_TYPE_2D;
		image_create_info.format					= surface_format;
		image_create_info.extent					= { size.x, size.y, 1 };
		image_create_info.mipLevels					= uint32_t( std::size( mip_levels ) );
		image_create_info.arrayLayers				= 1;
		image_create_info.samples					= samples;
		image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage						= usage;
		image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		if( std::size( unique_queue_family_indices ) > 1 ) {
			image_create_info.sharingMode			= VK_SHARING_MODE_CONCURRENT;
			image_create_info.queueFamilyIndexCount	= uint32_t( std::size( unique_queue_family_indices ) );
			image_create_info.pQueueFamilyIndices	= unique_queue_family_indices.data();
		} else {
			image_create_info.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			image_create_info.queueFamilyIndexCount	= 0;
			image_create_info.pQueueFamilyIndices	= nullptr;
		}

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext				= nullptr;
		image_view_create_info.flags				= 0;
		image_view_create_info.image				= VK_NULL_HANDLE;	// Automatically filled by CreateCompleteImageResource().
		image_view_create_info.viewType				= is_array_texture ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format				= surface_format;
		image_view_create_info.components			= {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		image_view_create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel	= 0;
		image_view_create_info.subresourceRange.levelCount		= uint32_t( std::size( mip_levels ) );
		image_view_create_info.subresourceRange.baseArrayLayer	= 0;
		image_view_create_info.subresourceRange.layerCount		= 1;

		vk2d::_internal::CompleteImageResource image = instance->GetDeviceMemoryPool()->CreateCompleteImageResource(
			&image_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&image_view_create_info
		);
		if( image != VK_SUCCESS ) {
			instance->Report( image.result, "Internal error: Cannot create RenderTargetTexture, cannot create image resource!" );
			return {};
		}

		return image;
	};

	auto render_queue			= instance->GetPrimaryRenderQueue();
	auto compute_queue			= instance->GetPrimaryComputeQueue();

	size						= new_size;

	auto granularity			= instance->GetPrimaryRenderQueue().GetQueueFamilyProperties().minImageTransferGranularity;
	if( size.x	% granularity.width		== 0 &&
		size.y	% granularity.height	== 0 &&
		1		% granularity.depth		== 0 ) {
		granularity_aligned = true;
	} else {
		granularity_aligned = false;
	}

	mipmap_levels				= vk2d::_internal::GenerateMipSizes(
		create_info_copy.size
	);

	// Using switch to decouple different paths of what resources to create.
	switch( type ) {
		case vk2d::_internal::RenderTargetTextureType::DIRECT:
			// (Render) -> Attachment -> (Blit) -> Sampled.
			//			   render				   render
			// Image queue family ownership ^

		{
			assert( VkSampleCountFlagBits( samples ) == VK_SAMPLE_COUNT_1_BIT );

			// TODO: Prune Render target texture image usage flags.

			for( auto & s : swap_buffers ) {

				// Create attachment image.
				s.attachment_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					{ &render_queue },
					VkSampleCountFlagBits( samples )
				);

				// Create sampled image.
				s.sampled_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					{ &render_queue },
					VK_SAMPLE_COUNT_1_BIT,
					mipmap_levels,
					true
				);
			}
			break;
		}

		case vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE:
			// (Render) -> Attachment -> (Resolve) -> Buffer1 -> (Blit) -> Sampled.
			//			   render					  render			   render
			// Image queue family ownership ^

		{
			for( auto & s : swap_buffers ) {

				// Create attachment image.
				s.attachment_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					{ &render_queue },
					VkSampleCountFlagBits( samples )
				);

				// Create buffer 1 image.
				s.buffer_1_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
					{ &render_queue },
					VK_SAMPLE_COUNT_1_BIT
				);

				// Create sampled image.
				s.sampled_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					{ &render_queue },
					VK_SAMPLE_COUNT_1_BIT,
					mipmap_levels,
					true
				);
			}

			break;
		}

		case vk2d::_internal::RenderTargetTextureType::WITH_BLUR:
			// (Render) -> Attachment -> (BlurPass1) -> Buffer1 -> (BlurPass2) -> Attachment -> (Blit) -> Sampled.
			//			   concurrent                   blur					  concurrent			  render
			// Image queue family ownership ^

		{
			for( auto & s : swap_buffers ) {

				// Create attachment image.
				s.attachment_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
					{ &render_queue, &compute_queue },
					VkSampleCountFlagBits( samples )
				);

				// Create buffer 1 image.
				s.buffer_1_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
					{ &compute_queue },
					VK_SAMPLE_COUNT_1_BIT
				);

				// Create sampled image.
				s.sampled_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					{ &render_queue },
					VK_SAMPLE_COUNT_1_BIT,
					mipmap_levels,
					true
				);
			}

			break;
		}
		case vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE_AND_BLUR:
			// (Render) -> Attachment -> (Resolve) -> Buffer1 -> (BlurPass1) -> Buffer2 -> (BlurPass2) -> Buffer1 -> (Blit) -> Sampled.
			//			   render					  concurrent				blur					  concurrent		   render
			// Image queue family ownership ^

		{
			for( auto & s : swap_buffers ) {

				// Create attachment image.
				s.attachment_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					{ &render_queue },
					VkSampleCountFlagBits( samples )
				);

				// Create buffer 1 image.
				s.buffer_1_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
					{ &render_queue, &compute_queue },
					VK_SAMPLE_COUNT_1_BIT
				);

				// Create buffer 2 image.
				s.buffer_2_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
					{ &compute_queue },
					VK_SAMPLE_COUNT_1_BIT
				);

				// Create sampled image.
				s.sampled_image = CreateLocalImageResource(
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					{ &render_queue },
					VK_SAMPLE_COUNT_1_BIT,
					mipmap_levels,
					true
				);
			}

			break;
		}
		default:
		{
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, unknown type!" );
			return false;
			break;
		}
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroyImages()
{
	for( auto & s : swap_buffers ) {
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.attachment_image );
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.sampled_image );
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.buffer_1_image );
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.buffer_2_image );
		s.attachment_image	= {};
		s.sampled_image		= {};
		s.buffer_1_image	= {};
		s.buffer_2_image	= {};
	}

}



bool vk2d::_internal::RenderTargetTextureImpl::CreateRenderPasses()
{
	bool use_multisampling = samples != vk2d::Multisamples::SAMPLE_COUNT_1;

	// Create primary render attachment render pass.
	{
		std::vector<VkAttachmentDescription> attachments {};
		attachments.reserve( 2 );
		{
			// Render attachment
			VkAttachmentDescription render_attachment {};
			render_attachment.flags				= 0;
			render_attachment.format			= surface_format;
			render_attachment.samples			= VkSampleCountFlagBits( samples );
			render_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			render_attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			render_attachment.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			render_attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			render_attachment.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			render_attachment.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Do not change layout automatically, we do it later.
			attachments.push_back( render_attachment );
		}
		if( use_multisampling ) {
			// Resolve attachment
			VkAttachmentDescription resolve_attachment {};
			resolve_attachment.flags			= 0;
			resolve_attachment.format			= surface_format;
			resolve_attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			resolve_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			resolve_attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			resolve_attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			resolve_attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			resolve_attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			resolve_attachment.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Do not change layout automatically, we do it later.
			attachments.push_back( resolve_attachment );
		}

		std::array<VkAttachmentReference, 1> color_attachment_references {};
		color_attachment_references[ 0 ].attachment		= 0;
		color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentReference, std::size( color_attachment_references )> resolve_attachment_references {};
		resolve_attachment_references[ 0 ].attachment	= 1;
		resolve_attachment_references[ 0 ].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkSubpassDescription, 1> subpasses {};
		subpasses[ 0 ].flags					= 0;
		subpasses[ 0 ].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[ 0 ].inputAttachmentCount		= 0;
		subpasses[ 0 ].pInputAttachments		= nullptr;
		subpasses[ 0 ].colorAttachmentCount		= uint32_t( color_attachment_references.size() );
		subpasses[ 0 ].pColorAttachments		= color_attachment_references.data();
		subpasses[ 0 ].pResolveAttachments		= use_multisampling ? resolve_attachment_references.data() : nullptr;
		subpasses[ 0 ].pDepthStencilAttachment	= nullptr;
		subpasses[ 0 ].preserveAttachmentCount	= 0;
		subpasses[ 0 ].pPreserveAttachments		= nullptr;

		std::array<VkSubpassDependency, 0> dependencies {};

		VkRenderPassCreateInfo render_pass_create_info {};
		render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.pNext				= nullptr;
		render_pass_create_info.flags				= 0;
		render_pass_create_info.attachmentCount		= uint32_t( attachments.size() );
		render_pass_create_info.pAttachments		= attachments.data();
		render_pass_create_info.subpassCount		= uint32_t( subpasses.size() );
		render_pass_create_info.pSubpasses			= subpasses.data();
		render_pass_create_info.dependencyCount		= uint32_t( dependencies.size() );
		render_pass_create_info.pDependencies		= dependencies.data();

		auto result = vkCreateRenderPass(
			instance->GetVulkanDevice(),
			&render_pass_create_info,
			nullptr,
			&vk_attachment_render_pass
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create render pass!" );
			return false;
		}
	}

	if( create_info_copy.enable_blur ) {

		// Create blur render pass 1
		{
			std::array<VkAttachmentDescription, 1> attachments {};
			// Render attachment
			attachments[ 0 ].flags				= 0;
			attachments[ 0 ].format				= surface_format;
			attachments[ 0 ].samples			= VK_SAMPLE_COUNT_1_BIT;
			attachments[ 0 ].loadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[ 0 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			attachments[ 0 ].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[ 0 ].stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[ 0 ].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[ 0 ].finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Do not change layout automatically, we do it later.

			std::array<VkAttachmentReference, 1> color_attachment_references {};
			color_attachment_references[ 0 ].attachment		= 0;
			color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			std::array<VkSubpassDescription, 1> subpasses {};
			subpasses[ 0 ].flags					= 0;
			subpasses[ 0 ].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpasses[ 0 ].inputAttachmentCount		= 0;
			subpasses[ 0 ].pInputAttachments		= nullptr;
			subpasses[ 0 ].colorAttachmentCount		= uint32_t( std::size( color_attachment_references ) );
			subpasses[ 0 ].pColorAttachments		= color_attachment_references.data();
			subpasses[ 0 ].pResolveAttachments		= nullptr;
			subpasses[ 0 ].pDepthStencilAttachment	= nullptr;
			subpasses[ 0 ].preserveAttachmentCount	= 0;
			subpasses[ 0 ].pPreserveAttachments		= nullptr;

			std::array<VkSubpassDependency, 0> dependencies {};

			VkRenderPassCreateInfo render_pass_create_info {};
			render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_create_info.pNext				= nullptr;
			render_pass_create_info.flags				= 0;
			render_pass_create_info.attachmentCount		= uint32_t( std::size( attachments ) );
			render_pass_create_info.pAttachments		= attachments.data();
			render_pass_create_info.subpassCount		= uint32_t( std::size( subpasses ) );
			render_pass_create_info.pSubpasses			= subpasses.data();
			render_pass_create_info.dependencyCount		= uint32_t( std::size( dependencies ) );
			render_pass_create_info.pDependencies		= dependencies.data();

			auto result = vkCreateRenderPass(
				instance->GetVulkanDevice(),
				&render_pass_create_info,
				nullptr,
				&vk_blur_render_pass_1
			);
			if( result != VK_SUCCESS ) {
				instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create render pass for blur pass 1!" );
				return false;
			}
		}

		// Create blur render pass 2
		{
			std::array<VkAttachmentDescription, 1> attachments {};
			// Render attachment
			attachments[ 0 ].flags				= 0;
			attachments[ 0 ].format				= surface_format;
			attachments[ 0 ].samples			= VK_SAMPLE_COUNT_1_BIT;
			attachments[ 0 ].loadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[ 0 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			attachments[ 0 ].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[ 0 ].stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[ 0 ].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[ 0 ].finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Do not change layout automatically, we do it later.

			std::array<VkAttachmentReference, 1> color_attachment_references {};
			color_attachment_references[ 0 ].attachment		= 0;
			color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			std::array<VkSubpassDescription, 1> subpasses {};
			subpasses[ 0 ].flags					= 0;
			subpasses[ 0 ].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpasses[ 0 ].inputAttachmentCount		= 0;
			subpasses[ 0 ].pInputAttachments		= nullptr;
			subpasses[ 0 ].colorAttachmentCount		= uint32_t( std::size( color_attachment_references ) );
			subpasses[ 0 ].pColorAttachments		= color_attachment_references.data();
			subpasses[ 0 ].pResolveAttachments		= nullptr;
			subpasses[ 0 ].pDepthStencilAttachment	= nullptr;
			subpasses[ 0 ].preserveAttachmentCount	= 0;
			subpasses[ 0 ].pPreserveAttachments		= nullptr;

			std::array<VkSubpassDependency, 0> dependencies {};

			VkRenderPassCreateInfo render_pass_create_info {};
			render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_create_info.pNext				= nullptr;
			render_pass_create_info.flags				= 0;
			render_pass_create_info.attachmentCount		= uint32_t( std::size( attachments ) );
			render_pass_create_info.pAttachments		= attachments.data();
			render_pass_create_info.subpassCount		= uint32_t( std::size( subpasses ) );
			render_pass_create_info.pSubpasses			= subpasses.data();
			render_pass_create_info.dependencyCount		= uint32_t( std::size( dependencies ) );
			render_pass_create_info.pDependencies		= dependencies.data();

			auto result = vkCreateRenderPass(
				instance->GetVulkanDevice(),
				&render_pass_create_info,
				nullptr,
				&vk_blur_render_pass_2
			);
			if( result != VK_SUCCESS ) {
				instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create render pass for blur pass 2!" );
				return false;
			}
		}
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroyRenderPasses()
{
	auto vk_device = instance->GetVulkanDevice();

	vkDestroyRenderPass(
		vk_device,
		vk_attachment_render_pass,
		nullptr
	);
	vk_attachment_render_pass	= VK_NULL_HANDLE;

	vkDestroyRenderPass(
		vk_device,
		vk_blur_render_pass_1,
		nullptr
	);
	vk_blur_render_pass_1		= VK_NULL_HANDLE;

	vkDestroyRenderPass(
		vk_device,
		vk_blur_render_pass_2,
		nullptr
	);
	vk_blur_render_pass_2		= VK_NULL_HANDLE;

}

bool vk2d::_internal::RenderTargetTextureImpl::CreateFramebuffers()
{
	bool use_multisampling = samples != vk2d::Multisamples::SAMPLE_COUNT_1;

	auto CreateLocalFramebuffer =[this](
		const std::vector<VkImageView>	&	attachments,
		VkRenderPass						compatible_render_pass
		) -> VkFramebuffer
	{
		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext			= nullptr;
		framebuffer_create_info.flags			= 0;
		framebuffer_create_info.renderPass		= compatible_render_pass;
		framebuffer_create_info.attachmentCount	= uint32_t( std::size( attachments ) );
		framebuffer_create_info.pAttachments	= attachments.data();
		framebuffer_create_info.width			= size.x;
		framebuffer_create_info.height			= size.y;
		framebuffer_create_info.layers			= 1;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		auto result = vkCreateFramebuffer(
			instance->GetVulkanDevice(),
			&framebuffer_create_info,
			nullptr,
			&framebuffer
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create framebuffer!" );
			return {};
		}

		return framebuffer;
	};

	for( auto & s : swap_buffers ) {

		// Main render framebuffer
		{
			std::vector<VkImageView> attachments;
			attachments.reserve( 2 );
			attachments.push_back( s.attachment_image.view );
			if( use_multisampling ) {
				attachments.push_back( s.buffer_1_image.view );
			}

			s.vk_render_framebuffer = CreateLocalFramebuffer(
				attachments,
				vk_attachment_render_pass
			);
			if( !s.vk_render_framebuffer ) return false;
		}

		if( create_info_copy.enable_blur ) {

			// We need to select rendering path here. Check comments CreateImages() for more info.

			// I no multisample we use
			// (Render) -> Attachment -> (BlurPass1) -> Buffer1 -> (BlurPass2) -> Attachment -> (Blit) -> Sampled.
			// Targets are Buffer1 and Attachment

			// If used multisampling we use
			// (Render) -> Attachment -> (Resolve) -> Buffer1 -> (BlurPass1) -> Buffer2 -> (BlurPass2) -> Buffer1 -> (Blit) -> Sampled.
			// Targets are Buffer2 and Buffer1

			// Create framebuffer for blur pass 1
			{
				std::vector<VkImageView> attachments;
				attachments.reserve( 1 );
				attachments.push_back( use_multisampling ? s.buffer_2_image.view : s.buffer_1_image.view );

				s.vk_blur_framebuffer_1 = CreateLocalFramebuffer(
					attachments,
					vk_blur_render_pass_1
				);
				if( !s.vk_blur_framebuffer_1 ) return false;
			}

			// Create framebuffer for blur pass 2
			{
				std::vector<VkImageView> attachments;
				attachments.reserve( 1 );
				// We need to select rendering path here. Check comments CreateImages() to see what image we should use.
				attachments.push_back( use_multisampling ? s.buffer_1_image.view : s.attachment_image.view );

				s.vk_blur_framebuffer_2 = CreateLocalFramebuffer(
					attachments,
					vk_blur_render_pass_2
				);
				if( !s.vk_blur_framebuffer_2 ) return false;
			}
		}
	}
	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroyFramebuffers()
{
	for( auto & s : swap_buffers ) {
		vkDestroyFramebuffer(
			instance->GetVulkanDevice(),
			s.vk_render_framebuffer,
			nullptr
		);
		s.vk_render_framebuffer	= VK_NULL_HANDLE;

		vkDestroyFramebuffer(
			instance->GetVulkanDevice(),
			s.vk_blur_framebuffer_1,
			nullptr
		);
		s.vk_blur_framebuffer_1 = VK_NULL_HANDLE;

		vkDestroyFramebuffer(
			instance->GetVulkanDevice(),
			s.vk_blur_framebuffer_2,
			nullptr
		);
		s.vk_blur_framebuffer_2 = VK_NULL_HANDLE;
	}
}



bool vk2d::_internal::RenderTargetTextureImpl::CreateSynchronizationPrimitives()
{
	auto result = VK_SUCCESS;

	// We use timeline semaphores to track render completion.
	// the primary difference is that timeline semaphores can be waited on without
	// automatically resetting their value. We can manually reset the value when we
	// begin recording the render target command buffer and count on the GPU / driver
	// to set it once finished and then use the same semaphore to indicate the render
	// target texture is ready to be used. As a bonus we can also wait on the timeline
	// semaphore on CPU, making host synchronization a bit easier to implement.
	VkSemaphoreCreateInfo binary_semaphore_create_info {};
	binary_semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	binary_semaphore_create_info.pNext		= nullptr;
	binary_semaphore_create_info.flags		= 0;

	VkSemaphoreTypeCreateInfo timeline_semaphore_type_create_info {};
	timeline_semaphore_type_create_info.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timeline_semaphore_type_create_info.pNext			= nullptr;
	timeline_semaphore_type_create_info.semaphoreType	= VK_SEMAPHORE_TYPE_TIMELINE;
	timeline_semaphore_type_create_info.initialValue	= 0;

	VkSemaphoreCreateInfo timeline_semaphore_create_info {};
	timeline_semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	timeline_semaphore_create_info.pNext		= &timeline_semaphore_type_create_info;
	timeline_semaphore_create_info.flags		= 0;

	for( auto & s : swap_buffers ) {

		result = vkCreateSemaphore(
			instance->GetVulkanDevice(),
			&binary_semaphore_create_info,
			nullptr,
			&s.vk_transfer_complete_semaphore
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create synchronization primitives!" );
			return false;
		}

		result = vkCreateSemaphore(
			instance->GetVulkanDevice(),
			//create_info_copy.enable_blur ? &binary_semaphore_create_info : &timeline_semaphore_create_info,
			&timeline_semaphore_create_info,
			nullptr,
			&s.vk_render_complete_semaphore
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot create RenderTargetTexture, cannot create synchronization primitives!" );
			return false;
		}
	}
	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroySynchronizationPrimitives()
{
	for( auto & s : swap_buffers ) {
		vkDestroySemaphore(
			instance->GetVulkanDevice(),
			s.vk_transfer_complete_semaphore,
			nullptr
		);
		vkDestroySemaphore(
			instance->GetVulkanDevice(),
			s.vk_render_complete_semaphore,
			nullptr
		);
	}
}

bool vk2d::_internal::RenderTargetTextureImpl::RecordTransferCommandBuffer(
	vk2d::_internal::RenderTargetTextureImpl::SwapBuffer	&	swap
)
{
	assert( swap.vk_transfer_command_buffer );

	auto command_buffer = swap.vk_transfer_command_buffer;

	// Begin command buffer
	{
		VkCommandBufferBeginInfo transfer_command_buffer_begin_info {};
		transfer_command_buffer_begin_info.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		transfer_command_buffer_begin_info.pNext			= nullptr;
		transfer_command_buffer_begin_info.flags			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		transfer_command_buffer_begin_info.pInheritanceInfo	= nullptr;

		auto result = vkBeginCommandBuffer(
			command_buffer,
			&transfer_command_buffer_begin_info
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot render to RenderTargetTexture, Cannot record mesh to GPU transfer command buffer!" );
			return false;
		}
	}

	// Record commands to upload frame data to gpu
	{
		if( !CmdUpdateFrameData(
			command_buffer
		) ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record commands to transfer FrameData to GPU!" );
			return false;
		}
	}

	// Record commands to upload mesh data to gpu
	{
		if( !mesh_buffer->CmdUploadMeshDataToGPU(
			command_buffer
		) ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record commands to transfer mesh data to GPU!" );
			return false;
		}
	}

	// End command buffer
	{
		auto result = vkEndCommandBuffer(
			command_buffer
		);
		if( result != VK_SUCCESS ) {
			instance->Report( result, "Internal error: Cannot render to RenderTargetTexture, Cannot compile mesh to GPU transfer command buffer!" );
			return false;
		}
	}

	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::UpdateSubmitInfos(
	vk2d::_internal::RenderTargetTextureImpl::SwapBuffer	&	swap,
	const std::vector<VkSemaphore>							&	wait_for_semaphores,
	const std::vector<uint64_t>								&	wait_for_semaphore_timeline_values,
	const std::vector<VkPipelineStageFlags>					&	wait_for_semaphore_pipeline_stages
)
{
	assert( std::size( wait_for_semaphores ) == std::size( wait_for_semaphore_timeline_values ) );
	assert( std::size( wait_for_semaphores ) == std::size( wait_for_semaphore_pipeline_stages ) );

	swap.vk_transfer_submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	swap.vk_transfer_submit_info.pNext					= nullptr;
	swap.vk_transfer_submit_info.waitSemaphoreCount		= 0;
	swap.vk_transfer_submit_info.pWaitSemaphores		= nullptr;
	swap.vk_transfer_submit_info.pWaitDstStageMask		= nullptr;
	swap.vk_transfer_submit_info.commandBufferCount		= 1;
	swap.vk_transfer_submit_info.pCommandBuffers		= &swap.vk_transfer_command_buffer;
	swap.vk_transfer_submit_info.signalSemaphoreCount	= 1;
	swap.vk_transfer_submit_info.pSignalSemaphores		= &swap.vk_transfer_complete_semaphore;

	// wait_for_semaphores lists all semaphores the render must wait for.
	// wait_for_semaphore_timeline_values lists values to wait for in the timeline semaphores.
	// These lists need to be same size, each semaphore in wait_for_semaphores has a corresponding
	// timeline value to wait for in wait_for_semaphore_timeline_values.
	// First entry is a transfer semaphore (binary), rest will be from dependant render target render
	// semaphores (timeline). We need a dummy value for every binary semaphore to keep the lists in sync.

	swap.render_wait_for_semaphores.push_back( swap.vk_transfer_complete_semaphore );
	swap.render_wait_for_semaphore_timeline_values.push_back( 1 );
	swap.render_wait_for_pipeline_stages.push_back( VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT );

	for( size_t i = 0; i < std::size( wait_for_semaphores ); ++i ) {
		swap.render_wait_for_semaphores.push_back( wait_for_semaphores[ i ] );
		swap.render_wait_for_semaphore_timeline_values.push_back( wait_for_semaphore_timeline_values[ i ] );
		swap.render_wait_for_pipeline_stages.push_back( wait_for_semaphore_pipeline_stages[ i ] );
	}

	swap.vk_render_timeline_semaphore_submit_info.sType						= VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	swap.vk_render_timeline_semaphore_submit_info.pNext						= nullptr;
	swap.vk_render_timeline_semaphore_submit_info.waitSemaphoreValueCount	= uint32_t( std::size( swap.render_wait_for_semaphore_timeline_values ) );
	swap.vk_render_timeline_semaphore_submit_info.pWaitSemaphoreValues		= swap.render_wait_for_semaphore_timeline_values.data();
	swap.vk_render_timeline_semaphore_submit_info.signalSemaphoreValueCount	= 1;
	swap.vk_render_timeline_semaphore_submit_info.pSignalSemaphoreValues	= &swap.render_counter;		// Ignored if blur is enabled.

	swap.vk_render_submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	swap.vk_render_submit_info.pNext					= &swap.vk_render_timeline_semaphore_submit_info;
	swap.vk_render_submit_info.waitSemaphoreCount		= uint32_t( std::size( swap.render_wait_for_semaphores ) );
	swap.vk_render_submit_info.pWaitSemaphores			= swap.render_wait_for_semaphores.data();
	swap.vk_render_submit_info.pWaitDstStageMask		= swap.render_wait_for_pipeline_stages.data();
	swap.vk_render_submit_info.commandBufferCount		= 1;
	swap.vk_render_submit_info.pCommandBuffers			= &swap.vk_render_command_buffer;
	swap.vk_render_submit_info.signalSemaphoreCount		= 1;
	swap.vk_render_submit_info.pSignalSemaphores		= &swap.vk_render_complete_semaphore;

	return true;
}

vk2d::_internal::TimedDescriptorPoolData & vk2d::_internal::RenderTargetTextureImpl::GetOrCreateDescriptorSetForSampler(
	vk2d::Sampler	*	sampler
)
{
	auto & set = sampler_descriptor_sets[ sampler ];

	// If this descriptor set doesn't exist yet for this
	// sampler texture combo, create one and update it.
	if( set.descriptor_set.descriptorSet == VK_NULL_HANDLE ) {
		set.descriptor_set = instance->AllocateDescriptorSet(
			instance->GetGraphicsSamplerDescriptorSetLayout()
		);
		if( set.descriptor_set != VK_SUCCESS ) {
			return set;
		}

		VkDescriptorImageInfo image_info {};
		image_info.sampler						= sampler->impl->GetVulkanSampler();
		image_info.imageView					= VK_NULL_HANDLE;
		image_info.imageLayout					= VK_IMAGE_LAYOUT_UNDEFINED;

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
			instance->GetVulkanDevice(),
			uint32_t( descriptor_write.size() ), descriptor_write.data(),
			0, nullptr
		);
	}

	return set;
}

vk2d::_internal::TimedDescriptorPoolData & vk2d::_internal::RenderTargetTextureImpl::GetOrCreateDescriptorSetForTexture(
	vk2d::Texture	*	texture
)
{
	auto & set = texture_descriptor_sets[ texture ];

	// If this descriptor set doesn't exist yet for this
	// sampler texture combo, create one and update it.
	if( set.descriptor_set.descriptorSet == VK_NULL_HANDLE ) {
		set.descriptor_set = instance->AllocateDescriptorSet(
			instance->GetGraphicsTextureDescriptorSetLayout()
		);
		if( set.descriptor_set != VK_SUCCESS ) {
			return set;
		}

		if( !texture->WaitUntilLoaded() ) {
			texture = instance->GetDefaultTexture();
		}

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
			instance->GetVulkanDevice(),
			uint32_t( descriptor_write.size() ), descriptor_write.data(),
			0, nullptr
		);
	}

	return set;
}

void vk2d::_internal::RenderTargetTextureImpl::CmdPushBlurTextureDescriptorWritesDirectly(
	VkCommandBuffer		command_buffer,
	VkPipelineLayout	vk_pipeline_layout,
	uint32_t			set_index,
	VkImageView			source_image,
	VkImageLayout		source_image_layout
)
{
	VkDescriptorImageInfo source_image_info {};
	source_image_info.sampler				= VK_NULL_HANDLE;
	source_image_info.imageView				= source_image;
	source_image_info.imageLayout			= source_image_layout;

	std::array<VkWriteDescriptorSet, 1> descriptor_write {};
	descriptor_write[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write[ 0 ].pNext				= nullptr;
	descriptor_write[ 0 ].dstSet			= VK_NULL_HANDLE;	// Ignored when pushing descriptor set directly into the command buffer.
	descriptor_write[ 0 ].dstBinding		= 0;
	descriptor_write[ 0 ].dstArrayElement	= 0;
	descriptor_write[ 0 ].descriptorCount	= 1;
	descriptor_write[ 0 ].descriptorType	= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptor_write[ 0 ].pImageInfo		= &source_image_info;
	descriptor_write[ 0 ].pBufferInfo		= nullptr;
	descriptor_write[ 0 ].pTexelBufferView	= nullptr;

	instance->VkFun_vkCmdPushDescriptorSetKHR(
		command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vk_pipeline_layout,
		set_index,
		uint32_t( std::size( descriptor_write ) ),
		descriptor_write.data()
	);
}

void vk2d::_internal::RenderTargetTextureImpl::CmdFinalizeRender(
	vk2d::_internal::RenderTargetTextureImpl::SwapBuffer		&	swap,
	vk2d::RenderTargetTextureBlurType								blur_type,
	vk2d::Vector2f													blur_amount
)
{
	switch( type ) {
		case vk2d::_internal::RenderTargetTextureType::DIRECT:
			// ( Render ) -> Attachment -> ( Blit ) -> Sampled.

			CmdBlitMipmapsToSampledImage(
				swap.vk_render_command_buffer,
				swap.attachment_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.sampled_image
			);
			break;

		case vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE:
			// (Render) -> Attachment -> (Resolve) -> Buffer1 -> (Blit) -> Sampled.

			CmdBlitMipmapsToSampledImage(
				swap.vk_render_command_buffer,
				swap.buffer_1_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.sampled_image
			);
			break;

		case vk2d::_internal::RenderTargetTextureType::WITH_BLUR:
			// (Render) -> Attachment -> (Render) -> Buffer1 -> (Render) -> Attachment -> (Blit) -> Sampled.

			CmdRecordBlurCommands(
				swap,
				swap.vk_render_command_buffer,
				blur_type,
				blur_amount,
				swap.attachment_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.buffer_1_image,
				swap.attachment_image
			);
			CmdBlitMipmapsToSampledImage(
				swap.vk_render_command_buffer,
				swap.attachment_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from blur render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.sampled_image
			);
			break;

		case vk2d::_internal::RenderTargetTextureType::WITH_MULTISAMPLE_AND_BLUR:
			// (Render) -> Attachment -> (Resolve) -> Buffer1 -> (Render) -> Buffer2 -> (Render) -> Buffer1 -> (Blit) -> Sampled.

			CmdRecordBlurCommands(
				swap,
				swap.vk_render_command_buffer,
				blur_type,
				blur_amount,
				swap.buffer_1_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.buffer_2_image,
				swap.buffer_1_image
			);
			CmdBlitMipmapsToSampledImage(
				swap.vk_render_command_buffer,
				swap.buffer_1_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,	// Coming from blur render pass, the image layout will be color attachment optimal.
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				swap.sampled_image
			);
			break;

		default:
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot finalize render target texture rendering, Invalid RenderTargetTextureType!" );
			break;
	}
}

void vk2d::_internal::RenderTargetTextureImpl::CmdBlitMipmapsToSampledImage(
	VkCommandBuffer												command_buffer,
	vk2d::_internal::CompleteImageResource					&	source_image,
	VkImageLayout												source_image_layout,
	VkPipelineStageFlagBits										source_image_pipeline_barrier_src_stage,
	vk2d::_internal::CompleteImageResource					&	destination_image
)
{
	// Source image only has 1 mip level as it's being rendered
	// to, it cannot have more. We need to generate mip maps for
	// sampled image to be useful as a texture.

	// General view of this function:
	// - Copy the source image to sampled image mip level 0.
	// - Blit from the source image to sampled image mip level 1.
	// - Loop and blit from the previous mip level to the next
	//   until we reach the last mip level. First mip level to
	//   blit from is mip level 1.
	// - Use pipeline barriers between write to read dependencies
	//   and for image layout conversions.

	assert( command_buffer );
	assert( instance );
	assert( std::size( mipmap_levels ) );

	// TODO: Study different ways of blitting the mipmaps.
	// It might be more efficient to copy the first mip level,
	// then blit from the previous layer until the image extent
	// is a certain size, after which we'll use a certain mip
	// level as a base for all smaller mip levels. This might
	// save some frequent pipeline bubbles when generating
	// the smallest mip levels.

	VkImageSubresourceRange subresource_range_mip_0 {};
	subresource_range_mip_0.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range_mip_0.baseMipLevel		= 0;
	subresource_range_mip_0.levelCount			= 1;
	subresource_range_mip_0.baseArrayLayer		= 0;
	subresource_range_mip_0.layerCount			= 1;

	VkImageSubresourceRange subresource_range_all_mips {};
	subresource_range_all_mips.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range_all_mips.baseMipLevel		= 0;
	subresource_range_all_mips.levelCount		= uint32_t( std::size( mipmap_levels ) );
	subresource_range_all_mips.baseArrayLayer	= 0;
	subresource_range_all_mips.layerCount		= 1;

	{
		std::array<VkImageMemoryBarrier, 2> image_memory_barriers;

		// Source image barrier ( mip level 0 only ).
		// Change source image layout and add memory barrier dependency.
		image_memory_barriers[ 0 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 0 ].pNext					= nullptr;
		image_memory_barriers[ 0 ].srcAccessMask			= VK_ACCESS_MEMORY_WRITE_BIT;
		image_memory_barriers[ 0 ].dstAccessMask			= VK_ACCESS_TRANSFER_READ_BIT;
		image_memory_barriers[ 0 ].oldLayout				= source_image_layout;
		image_memory_barriers[ 0 ].newLayout				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		image_memory_barriers[ 0 ].srcQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].dstQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].image					= source_image.image;
		image_memory_barriers[ 0 ].subresourceRange			= subresource_range_mip_0;

		// Sampled image barrier ( all mip levels ).
		// Change sampled image layout.
		VkImageMemoryBarrier sampled_image_memory_barrier {};
		image_memory_barriers[ 1 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 1 ].pNext					= nullptr;
		image_memory_barriers[ 1 ].srcAccessMask			= 0;
		image_memory_barriers[ 1 ].dstAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barriers[ 1 ].oldLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		image_memory_barriers[ 1 ].newLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barriers[ 1 ].srcQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 1 ].dstQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 1 ].image					= destination_image.image;
		image_memory_barriers[ 1 ].subresourceRange			= subresource_range_all_mips;

		vkCmdPipelineBarrier(
			command_buffer,
			source_image_pipeline_barrier_src_stage,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
		);
	}

	if( granularity_aligned ) {

		// Copying from the source image to sampled image mip level 0.
		// The render target image size is aligned to image granularity so we
		// can do a fast copy from source to sampled image.

		VkImageCopy region {};
		region.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel			= 0;
		region.srcSubresource.baseArrayLayer	= 0;
		region.srcSubresource.layerCount		= 1;
		region.srcOffset						= { 0, 0, 0 };
		region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel			= 0;
		region.dstSubresource.baseArrayLayer	= 0;
		region.dstSubresource.layerCount		= 1;
		region.dstOffset						= { 0, 0, 0 };
		region.extent							= { size.x, size.y, 1 };
		vkCmdCopyImage(
			command_buffer,
			source_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			destination_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
		);
	} else {

		// Copying from the source image to sampled image mip level 0.
		// The render target image is not aligned to image granularity
		// so as a shortcut we'll do a 1:1 blit operation.

		// TODO: Investigate if we can get rid of blitting when copying from source to sampled image with unaligned granularity.
		// Force render target extent to be multiple of the primary render queue
		// family granularity. Needs more research of how the render target
		// texture behaves when rendered as a texture on a surface, specifically
		// need to know if we can control the image extent manually when sampling
		// the sampled image in a shader.

		// Blit from source to sampled ( to mip level 0 only ).
		VkImageBlit region {};
		region.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel			= 0;
		region.srcSubresource.baseArrayLayer	= 0;
		region.srcSubresource.layerCount		= 1;
		region.srcOffsets[ 0 ]					= { 0, 0, 0 };
		region.srcOffsets[ 1 ]					= { int32_t( size.x ), int32_t( size.y ), 1 };
		region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel			= 0;
		region.dstSubresource.baseArrayLayer	= 0;
		region.dstSubresource.layerCount		= 1;
		region.dstOffsets[ 0 ]					= { 0, 0, 0 };
		region.dstOffsets[ 1 ]					= { int32_t( size.x ), int32_t( size.y ), 1 };
		vkCmdBlitImage(
			command_buffer,
			source_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			destination_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region,
			VK_FILTER_NEAREST
		);
	}

	// We can directly blit into the second mip level from the source image.
	if( std::size( mipmap_levels ) > 1 ) {
		// Blit from source to sampled ( to mip level 1 only ).
		VkImageBlit region {};
		region.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel			= 0;
		region.srcSubresource.baseArrayLayer	= 0;
		region.srcSubresource.layerCount		= 1;
		region.srcOffsets[ 0 ]					= { 0, 0, 0 };
		region.srcOffsets[ 1 ]					= { int32_t( size.x ), int32_t( size.y ), 1 };
		region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel			= 1;
		region.dstSubresource.baseArrayLayer	= 0;
		region.dstSubresource.layerCount		= 1;
		region.dstOffsets[ 0 ]					= { 0, 0, 0 };
		region.dstOffsets[ 1 ]					= { int32_t( mipmap_levels[ 1 ].width ), int32_t( mipmap_levels[ 1 ].height ), 1 };
		vkCmdBlitImage(
			command_buffer,
			source_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			destination_image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region,
			VK_FILTER_LINEAR
		);
	}

	// Transition sampled image mip level 0 layout to shader use optimal.
	// Since we're blitting the second mip level directly from the
	// source image we can immediately set the mip level 0 of sampled
	// image to final layout.
	{
		std::array<VkImageMemoryBarrier, 1> image_memory_barriers;

		image_memory_barriers[ 0 ].sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 0 ].pNext					= nullptr;
		image_memory_barriers[ 0 ].srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barriers[ 0 ].dstAccessMask			= vk_sampled_image_final_access_mask;
		image_memory_barriers[ 0 ].oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barriers[ 0 ].newLayout				= vk_sampled_image_final_layout;
		image_memory_barriers[ 0 ].srcQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].dstQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].image					= destination_image.image;
		image_memory_barriers[ 0 ].subresourceRange			= subresource_range_mip_0;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
		);
	}

	for( size_t i = 2; i < std::size( mipmap_levels ); ++i ) {

		// Memory barrier from previous mip level to current,
		// Write to Read dependency and layout transition.
		{
			std::array<VkImageMemoryBarrier, 1> image_memory_barriers;

			// memory barrier to change source image layout.
			image_memory_barriers[ 0 ].sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barriers[ 0 ].pNext							= nullptr;
			image_memory_barriers[ 0 ].srcAccessMask					= VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barriers[ 0 ].dstAccessMask					= VK_ACCESS_TRANSFER_READ_BIT;
			image_memory_barriers[ 0 ].oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			image_memory_barriers[ 0 ].newLayout						= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			image_memory_barriers[ 0 ].srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
			image_memory_barriers[ 0 ].dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
			image_memory_barriers[ 0 ].image							= destination_image.image;
			image_memory_barriers[ 0 ].subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			image_memory_barriers[ 0 ].subresourceRange.baseMipLevel	= uint32_t( i - 1 );
			image_memory_barriers[ 0 ].subresourceRange.levelCount		= 1;
			image_memory_barriers[ 0 ].subresourceRange.baseArrayLayer	= 0;
			image_memory_barriers[ 0 ].subresourceRange.layerCount		= 1;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
			);
		}

		// Blit remaining mip levels.
		{
			VkImageBlit region {};
			region.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.mipLevel			= uint32_t( i - 1 );
			region.srcSubresource.baseArrayLayer	= 0;
			region.srcSubresource.layerCount		= 1;
			region.srcOffsets[ 0 ]					= { 0, 0, 0 };
			region.srcOffsets[ 1 ]					= { int32_t( mipmap_levels[ i - 1 ].width ), int32_t( mipmap_levels[ i - 1 ].height ), 1 };
			region.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.mipLevel			= uint32_t( i );
			region.dstSubresource.baseArrayLayer	= 0;
			region.dstSubresource.layerCount		= 1;
			region.dstOffsets[ 0 ]					= { 0, 0, 0 };
			region.dstOffsets[ 1 ]					= { int32_t( mipmap_levels[ i ].width ), int32_t( mipmap_levels[ i ].height ), 1 };
			vkCmdBlitImage(
				command_buffer,
				destination_image.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				destination_image.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &region,
				VK_FILTER_LINEAR
			);
		}

		// Memory barrier from previous mip level to current,
		// Layout transition to final layout.
		{
			std::array<VkImageMemoryBarrier, 1> image_memory_barriers;

			// memory barrier to change source image layout.
			image_memory_barriers[ 0 ].sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barriers[ 0 ].pNext							= nullptr;
			image_memory_barriers[ 0 ].srcAccessMask					= VK_ACCESS_TRANSFER_READ_BIT;
			image_memory_barriers[ 0 ].dstAccessMask					= vk_sampled_image_final_access_mask;
			image_memory_barriers[ 0 ].oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			image_memory_barriers[ 0 ].newLayout						= vk_sampled_image_final_layout;
			image_memory_barriers[ 0 ].srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
			image_memory_barriers[ 0 ].dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
			image_memory_barriers[ 0 ].image							= destination_image.image;
			image_memory_barriers[ 0 ].subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			image_memory_barriers[ 0 ].subresourceRange.baseMipLevel	= uint32_t( i - 1 );
			image_memory_barriers[ 0 ].subresourceRange.levelCount		= 1;
			image_memory_barriers[ 0 ].subresourceRange.baseArrayLayer	= 0;
			image_memory_barriers[ 0 ].subresourceRange.layerCount		= 1;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
			);
		}
	}

	// Memory barrier to convert the last mip image layout to vk_sampled_image_final_layout.
	// After this the complete image is ready to be used.
	if( std::size( mipmap_levels ) > 1 ) {
		std::array<VkImageMemoryBarrier, 1> image_memory_barriers;

		// memory barrier to change source image layout.
		image_memory_barriers[ 0 ].sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 0 ].pNext							= nullptr;
		image_memory_barriers[ 0 ].srcAccessMask					= VK_ACCESS_TRANSFER_READ_BIT;
		image_memory_barriers[ 0 ].dstAccessMask					= vk_sampled_image_final_access_mask;
		image_memory_barriers[ 0 ].oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barriers[ 0 ].newLayout						= vk_sampled_image_final_layout;
		image_memory_barriers[ 0 ].srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].image							= destination_image.image;
		image_memory_barriers[ 0 ].subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_memory_barriers[ 0 ].subresourceRange.baseMipLevel	= uint32_t( std::size( mipmap_levels ) - 1 );
		image_memory_barriers[ 0 ].subresourceRange.levelCount		= 1;
		image_memory_barriers[ 0 ].subresourceRange.baseArrayLayer	= 0;
		image_memory_barriers[ 0 ].subresourceRange.layerCount		= 1;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
		);
	}
}



// [ 0 ] = sigma, [ 1 ] = precomputed normalizer, [ 2 ] = initial coefficient, [ 3 ] = initial natural exponentation.
std::array<float, 4> CalculateBlurShaderInfo(
	float		coverage
)
{
	constexpr double M_PI = 3.1415926535897932384626433832795;

	std::array<float, 4> ret {};

	float	delta		= 1.0f;
	float	sigma		= coverage / 3.0f;
	int		support		= int( sigma * 3.0f );	// Tuncation. Rounding hazard, calculate from sigma to be exact with the shader.
	float	norm		= 0.0f;

	float	g0			= 1.0f / float( sqrt( 2.0 * M_PI ) * sigma );
	float	g1			= std::exp( -0.5f * delta * delta / ( sigma * sigma ) );
	float	g2			= g1 * g1;

	ret[ 0 ]			= sigma;
	ret[ 2 ]			= g0;
	ret[ 3 ]			= g1;

	norm	+= g0;
	g0		*= g1;
	g1		*= g2;
	for( int i = 1; i <= support; i += 2 ) {
		norm	+= g0 * 2.0f;
		g0		*= g1;
		g1		*= g2;
		norm	+= g0 * 2.0f;
		g0		*= g1;
		g1		*= g2;
	}
	float precomp_norm = 1.0f / norm;

	ret[ 1 ]	= precomp_norm;

	return ret;
}



bool vk2d::_internal::RenderTargetTextureImpl::CmdRecordBlurCommands(
	vk2d::_internal::RenderTargetTextureImpl::SwapBuffer	&	swap,
	VkCommandBuffer												command_buffer,
	vk2d::RenderTargetTextureBlurType							blur_type,
	vk2d::Vector2f												blur_amount,
	vk2d::_internal::CompleteImageResource					&	source_image,
	VkImageLayout												source_image_layout,
	VkPipelineStageFlagBits										source_image_pipeline_barrier_src_stage,
	vk2d::_internal::CompleteImageResource					&	intermediate_image,
	vk2d::_internal::CompleteImageResource					&	destination_image
)
{
	// This function records commands to do 2 gaussian blur passes using the primary render queue.
	// Flow of this function depends on specific settings used, rendering attachments differ if
	// multisampling is enabled or not, which attachments are used is decided by the caller.
	// Vertex data is build directly into the shader so we don't have to worry about it.
	//
	// General flow of this function is:
	// - Image memory pipeline barrier.
	// - Render pass 1, horisontal blur.
	//   - Bind descriptor sets, constants.
	//   - Draw.
	// - Image memory pipeline barrier.
	// - Render pass 2, vertical blur.
	//   - Bind descriptor sets, constants.
	//   - Draw.



	auto RecordBlurPass =[ this, command_buffer ](
		VkPipelineLayout							use_pipeline_layout,
		VkRenderPass								use_render_pass,
		VkFramebuffer								use_framebuffer,
		vk2d::_internal::GraphicsShaderProgram	&	graphics_shader_program,
		float										blur_coverage,
		VkImageView									vk_texture,
		VkImageLayout								texture_image_layout
		) -> void
	{
		// Begin the render pass.
		{
			VkRect2D render_area {};
			render_area.offset = { 0, 0 };
			render_area.extent = { size.x, size.y };

			VkClearValue clear_values {};
			clear_values.color.float32[ 0 ]			= 0.0f;
			clear_values.color.float32[ 1 ]			= 0.0f;
			clear_values.color.float32[ 2 ]			= 0.0f;
			clear_values.color.float32[ 3 ]			= 0.0f;

			VkRenderPassBeginInfo render_pass_begin_info {};
			render_pass_begin_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext			= nullptr;
			render_pass_begin_info.renderPass		= use_render_pass;
			render_pass_begin_info.framebuffer		= use_framebuffer;
			render_pass_begin_info.renderArea		= render_area;
			render_pass_begin_info.clearValueCount	= 1;
			render_pass_begin_info.pClearValues		= &clear_values;

			vkCmdBeginRenderPass(
				command_buffer,
				&render_pass_begin_info,
				VK_SUBPASS_CONTENTS_INLINE
			);
		}

		// Record blur render.
		{
			// Bind blur pass pipeline.
			{
				vk2d::_internal::GraphicsPipelineSettings pipeline_settings {};
				pipeline_settings.vk_pipeline_layout	= use_pipeline_layout;
				pipeline_settings.vk_render_pass		= use_render_pass;
				pipeline_settings.primitive_topology	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				pipeline_settings.polygon_mode			= VK_POLYGON_MODE_FILL;
				pipeline_settings.shader_programs		= graphics_shader_program;
				pipeline_settings.samples				= VK_SAMPLE_COUNT_1_BIT;
				pipeline_settings.enable_blending		= VK_FALSE;
				auto pipeline = instance->GetGraphicsPipeline( pipeline_settings );

				vkCmdBindPipeline(
					command_buffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipeline
				);
			}

			// Bind descriptor sets.
			{
				// Bind simple sampler to set 0.
				{
					auto set = instance->GetBlurSamplerDescriptorSet();

					vkCmdBindDescriptorSets(
						command_buffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						use_pipeline_layout,
						0,
						1, &set,
						0, nullptr
					);
				}

				// Bind/push texture to set 1.
				CmdPushBlurTextureDescriptorWritesDirectly(
					command_buffer,
					use_pipeline_layout,
					1,
					vk_texture,
					texture_image_layout
				);
			}

			// Push constants.
			{
				if( blur_coverage < 1.0f ) blur_coverage = 1.0f;

				vk2d::_internal::GraphicsBlurPushConstants push_constants {};
				push_constants.blur_info	= CalculateBlurShaderInfo( blur_coverage );
				push_constants.pixel_size	= { 1.0f / float( size.x ), 1.0f / float( size.y ) };

				vkCmdPushConstants(
					command_buffer,
					use_pipeline_layout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof( push_constants ),
					&push_constants
				);
			}

			vkCmdDraw(
				command_buffer,
				3,
				1,
				0,
				0
			);
		}

		// End the render pass.
		{
			vkCmdEndRenderPass(
				command_buffer
			);
		}
	};



	VkImageSubresourceRange mip_0_only_subresource_range {};
	mip_0_only_subresource_range.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	mip_0_only_subresource_range.baseMipLevel	= 0;
	mip_0_only_subresource_range.levelCount		= 1;
	mip_0_only_subresource_range.baseArrayLayer	= 0;
	mip_0_only_subresource_range.layerCount		= 1;

	// Pipeline barriers.
	{
		std::array<VkImageMemoryBarrier, 1> image_memory_barriers {};

		// Source image.
		image_memory_barriers[ 0 ].sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 0 ].pNext				= nullptr;
		image_memory_barriers[ 0 ].srcAccessMask		= VK_ACCESS_MEMORY_WRITE_BIT;
		image_memory_barriers[ 0 ].dstAccessMask		= VK_ACCESS_SHADER_READ_BIT;
		image_memory_barriers[ 0 ].oldLayout			= source_image_layout;
		image_memory_barriers[ 0 ].newLayout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_memory_barriers[ 0 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].image				= source_image.image;
		image_memory_barriers[ 0 ].subresourceRange		= mip_0_only_subresource_range;

		vkCmdPipelineBarrier(
			command_buffer,
			source_image_pipeline_barrier_src_stage,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
		);
	}

	// First render pass.
	{
		vk2d::_internal::GraphicsShaderProgram shader_program;
		switch( blur_type ) {
			case vk2d::RenderTargetTextureBlurType::BOX:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_BOX_BLUR_HORISONTAL );
				break;
			case vk2d::RenderTargetTextureBlurType::GAUSSIAN:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_GAUSSIAN_BLUR_HORISONTAL );
				break;
			default:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_GAUSSIAN_BLUR_HORISONTAL );
				break;
		}

		RecordBlurPass(
			instance->GetGraphicsBlurPipelineLayout(),
			vk_blur_render_pass_1,
			swap.vk_blur_framebuffer_1,	// This directs the render to the intermediate_image.
			shader_program,
			blur_amount.x,
			source_image.view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	// Pipeline barriers.
	// TODO: Might not need this pipeline barrier between blur render passes in RenderTargetTexture blur. Double check Vulkan implicit synchronization.
	{
		std::array<VkImageMemoryBarrier, 1> image_memory_barriers {};

		image_memory_barriers[ 0 ].sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barriers[ 0 ].pNext				= nullptr;
		image_memory_barriers[ 0 ].srcAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		image_memory_barriers[ 0 ].dstAccessMask		= VK_ACCESS_SHADER_READ_BIT;
		image_memory_barriers[ 0 ].oldLayout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		image_memory_barriers[ 0 ].newLayout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_memory_barriers[ 0 ].srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barriers[ 0 ].image				= intermediate_image.image;
		image_memory_barriers[ 0 ].subresourceRange		= mip_0_only_subresource_range;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			uint32_t( std::size( image_memory_barriers ) ), image_memory_barriers.data()
		);
	}

	// Second render pass.
	{
		vk2d::_internal::GraphicsShaderProgram shader_program;
		switch( blur_type ) {
			case vk2d::RenderTargetTextureBlurType::BOX:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_BOX_BLUR_VERTICAL );
				break;
			case vk2d::RenderTargetTextureBlurType::GAUSSIAN:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_GAUSSIAN_BLUR_VERTICAL );
				break;
			default:
				shader_program = instance->GetGraphicsShaderModules( vk2d::_internal::GraphicsShaderProgramID::RENDER_TARGET_GAUSSIAN_BLUR_VERTICAL );
				break;
		}

		RecordBlurPass(
			instance->GetGraphicsBlurPipelineLayout(),
			vk_blur_render_pass_2,
			swap.vk_blur_framebuffer_2,	// This directs the render to the destination_image.
			shader_program,
			blur_amount.y,
			intermediate_image.view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::CmdBindGraphicsPipelineIfDifferent(
	VkCommandBuffer										command_buffer,
	const vk2d::_internal::GraphicsPipelineSettings	&	pipeline_settings
)
{
	if( previous_graphics_pipeline_settings != pipeline_settings ) {
		auto pipeline = instance->GetGraphicsPipeline( pipeline_settings );

		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline
		);
		previous_graphics_pipeline_settings	= pipeline_settings;
	}
}

void vk2d::_internal::RenderTargetTextureImpl::CmdBindSamplerIfDifferent(
	VkCommandBuffer			command_buffer,
	vk2d::Sampler		*	sampler,
	VkPipelineLayout		use_pipeline_layout
)
{
	assert( sampler );

	// if sampler or texture changed since previous call, bind a different descriptor set.
	if( sampler != previous_sampler ) {
		auto & set = GetOrCreateDescriptorSetForSampler( sampler );
		set.previous_access_time = std::chrono::steady_clock::now();

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			use_pipeline_layout,
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_SAMPLER_AND_SAMPLER_DATA,
			1, &set.descriptor_set.descriptorSet,
			0, nullptr
		);

		previous_sampler		= sampler;
	}
}

void vk2d::_internal::RenderTargetTextureImpl::CmdBindTextureIfDifferent(
	VkCommandBuffer			command_buffer,
	vk2d::Texture		*	texture,
	VkPipelineLayout		use_pipeline_layout
)
{
	assert( texture );

	// if sampler or texture changed since previous call, bind a different descriptor set.
	if( texture != previous_texture ) {
		auto & set = GetOrCreateDescriptorSetForTexture( texture );
		set.previous_access_time = std::chrono::steady_clock::now();

		vkCmdBindDescriptorSets(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			use_pipeline_layout,
			GRAPHICS_DESCRIPTOR_SET_ALLOCATION_TEXTURE,
			1, &set.descriptor_set.descriptorSet,
			0, nullptr
		);

		previous_texture		= texture;
	}
}

void vk2d::_internal::RenderTargetTextureImpl::CmdSetLineWidthIfDifferent(
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

bool vk2d::_internal::RenderTargetTextureImpl::CmdUpdateFrameData(
	VkCommandBuffer command_buffer
)
{
	// Window coordinate system scaling
	vk2d::_internal::WindowCoordinateScaling window_coordinate_scaling {};

	switch( create_info_copy.coordinate_space ) {
		case vk2d::RenderCoordinateSpace::TEXEL_SPACE:
			window_coordinate_scaling.multiplier	= { 1.0f / ( size.x / 2.0f ), 1.0f / ( size.y / 2.0f ) };
			window_coordinate_scaling.offset		= { -1.0f, -1.0f };
			break;
		case vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED:
			window_coordinate_scaling.multiplier	= { 1.0f / ( size.x / 2.0f ), 1.0f / ( size.y / 2.0f ) };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
			break;
		case vk2d::RenderCoordinateSpace::NORMALIZED_SPACE:
		{
			float contained_minimum_dimension		= float( std::min( size.x, size.y ) );
			window_coordinate_scaling.multiplier	= { contained_minimum_dimension / ( size.x / 2.0f ), contained_minimum_dimension / ( size.y / 2.0f ) };
			window_coordinate_scaling.offset		= { -1.0f, -1.0f };
		}
		break;
		case vk2d::RenderCoordinateSpace::NORMALIZED_SPACE_CENTERED:
		{
			float contained_minimum_dimension		= float( std::min( size.x, size.y ) );
			window_coordinate_scaling.multiplier	= { contained_minimum_dimension / size.x, contained_minimum_dimension / size.y };
			window_coordinate_scaling.offset		= { 0.0f, 0.0f };
		}
		break;
		case vk2d::RenderCoordinateSpace::NORMALIZED_VULKAN:
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
		auto frame_data = frame_data_staging_buffer.memory.Map<vk2d::_internal::FrameData>();
		if( !frame_data ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot map FrameData staging buffer memory!" );
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
		copy_region.size		= sizeof( vk2d::_internal::FrameData );
		vkCmdCopyBuffer(
			command_buffer,
			frame_data_staging_buffer.buffer,
			frame_data_device_buffer.buffer,
			1, &copy_region
		);
	}

	return true;
}
