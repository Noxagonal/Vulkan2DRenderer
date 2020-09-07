#pragma once

#include "../Core/SourceCommon.h"

#include "../../Include/Interface/RenderTargetTexture.h"

#include "../Core/VulkanMemoryManagement.h"
#include "../Core/DescriptorSet.h"
#include "../Core/MeshBuffer.h"
#include "RenderTargetCommonImpl.hpp"
#include "../../Include/Interface/Texture.h"
#include "TextureImpl.h"



namespace vk2d {

namespace _internal {

class InstanceImpl;
class MeshBuffer;



// Render target implementation
class RenderTargetTextureImpl :
	public vk2d::_internal::TextureImpl
{
public:
	RenderTargetTextureImpl(
		vk2d::_internal::InstanceImpl				*	instance,
		const vk2d::RenderTargetTextureCreateInfo	&	create_info );

	~RenderTargetTextureImpl();

	void												SetSize(
		vk2d::Vector2u									new_size );
	vk2d::Vector2u										GetSize() const;
	uint32_t											GetLayerCount() const;

	uint32_t											GetCurrentSwapBuffer() const;
	VkImage												GetVulkanImage() const;
	VkImageView											GetVulkanImageView() const;
	VkImageLayout										GetVulkanImageLayout() const;
	VkFramebuffer										GetFramebuffer() const;

	bool												WaitUntilLoaded();

	// Begins the render operations. You must call this before using any drawing commands.
	// For best performance you should calculate game logic first, when you're ready to draw
	// call this function just before your first draw command.
	bool												BeginRender();

	// Ends the rendering operations. You must call this after you're done drawing.
	// This will display the results on screen.
	bool												EndRender();

	void												DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		bool											filled						= true,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	void												DrawLineList(
		const std::vector<vk2d::VertexIndex_2>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	void												DrawPointList(
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	void												DrawMesh(
		const vk2d::Mesh							&	mesh );

	bool												IsGood() const;

private:
	bool												CreateCommandBuffers();
	void												DestroyCommandBuffers();

	bool												CreateFrameDataBuffers();
	void												DestroyFrameDataBuffers();

	bool												CreateSurfaces(
		vk2d::Vector2u									new_size );

	void												DestroySurfaces();

	bool												CreateSynchronizationPrimitives();
	void												DestroySynchronizationPrimitives();

	void												CmdBindPipelineIfDifferent(
		VkCommandBuffer									command_buffer,
		const vk2d::_internal::PipelineSettings		&	pipeline_settings );

	void												CmdBindTextureSamplerIfDifferent(
		VkCommandBuffer									command_buffer,
		vk2d::Sampler								*	sampler,
		vk2d::Texture								*	texture );

	void												CmdSetLineWidthIfDifferent(
		VkCommandBuffer									command_buffer,
		float											line_width );

	bool												CmdUpdateFrameData(
		VkCommandBuffer									command_buffer );

	struct SwapBuffer
	{
		vk2d::_internal::CompleteImageResource			attachment_image							= {};	// Render attachment, Multisampled, 1 mip level
		vk2d::_internal::CompleteImageResource			buffer_1_image								= {};	// Buffer image, used as multisample resolve and blur buffer
		vk2d::_internal::CompleteImageResource			buffer_2_image								= {};	// Buffer image, used as second blur buffer
		vk2d::_internal::CompleteImageResource			sampled_image								= {};	// Output, sampled image with mip mapping
		VkFramebuffer									vk_framebuffer								= {};
		VkImageLayout									vk_render_image_layout						= {};

		VkCommandBuffer									vk_transfer_command_buffer					= {};
		VkCommandBuffer									vk_render_command_buffer					= {};
		VkSubmitInfo									vk_transfer_submit_info						= {};
		VkSubmitInfo									vk_render_submit_info						= {};
		VkTimelineSemaphoreSubmitInfo					vk_transfer_timeline_semaphore_submit_info	= {};
		VkTimelineSemaphoreSubmitInfo					vk_render_timeline_semaphore_submit_info	= {};

		VkSemaphore										vk_transfer_to_render_semaphore				= {};	// Using regular semaphore.
		VkSemaphore										vk_render_complete_semaphore				= {};	// Using timeline semaphore, 0 when pending render, 1 when rendered.
		uint64_t										signal_render_complete_semaphore_value		= {};
		VkPipelineStageFlags							transfer_to_render_wait_dst_stage_mask		= {};
	};

	vk2d::_internal::InstanceImpl					*	instance									= {};
	vk2d::RenderTargetTextureCreateInfo					create_info_copy							= {};

	vk2d::Vector2u										size										= {};
	std::vector<VkExtent2D>								mipmap_levels								= {};

	vk2d::_internal::CompleteBufferResource				frame_data_staging_buffer					= {};
	vk2d::_internal::CompleteBufferResource				frame_data_device_buffer					= {};
	vk2d::_internal::PoolDescriptorSet					frame_data_descriptor_set					= {};

	VkCommandPool										vk_command_pool								= {};
	VkRenderPass										vk_render_pass								= {};

	std::unique_ptr<vk2d::_internal::MeshBuffer>		mesh_buffer;

	uint32_t											current_swap_buffer							= {};
	std::array<vk2d::_internal::RenderTargetTextureImpl::SwapBuffer, 2>
														swap_buffers								= {};

	vk2d::_internal::PipelineSettings					previous_pipeline_settings					= {};
	vk2d::Texture									*	previous_texture							= {};
	vk2d::Sampler									*	previous_sampler							= {};
	float												previous_line_width							= {};

	std::map<vk2d::Sampler*, std::map<vk2d::Texture*, vk2d::_internal::SamplerTextureDescriptorPoolData>>
														sampler_texture_descriptor_sets				= {};

	bool												is_good										= {};
};



} // _internal

} // vk2d
