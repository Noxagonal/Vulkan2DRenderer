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

class Mesh;

namespace _internal {

class InstanceImpl;
class MeshBuffer;
class RenderTargetTextureImpl;



// Used to select the implementation. Each works a bit differently from the others. We use
// 2 to 4 Vulkan images depending on the implementation used. "Attachment" is the first
// render, it alone can have multisample enabled, "Sampled" is the final product of this
// pipeline, it alone can have multiple mip maps, Buffer1 and Buffer2 are used as needed
// as intermediate render target images, those must always be 1 sample and 1 mipmap.
// 
// Direct:						(Render) -> Attachment -> (Blit) -> Sampled.
// With multisample:			(Render) -> Attachment -> (Resolve) -> Buffer1 -> (Blit) -> Sampled.
// With blur:					(Render) -> Attachment -> (Render) -> Buffer1 -> (Render) -> Attachment -> (Blit) -> Sampled.
// With multisample and blur:	(Render) -> Attachment -> (Resolve) -> Buffer1 -> (Render) -> Buffer2 -> (Render) -> Buffer1 -> (Blit) -> Sampled.
enum class RenderTargetTextureType
{
	NONE						= 0,	// Not a type, used for error detection.
	DIRECT,
	WITH_MULTISAMPLE,
	WITH_BLUR,
	WITH_MULTISAMPLE_AND_BLUR,
};



// Render target implementation
class RenderTargetTextureImpl :
	public vk2d::_internal::TextureImpl
{
public:
	RenderTargetTextureImpl(
		vk2d::_internal::InstanceImpl								*	instance,
		const vk2d::RenderTargetTextureCreateInfo					&	create_info );

	~RenderTargetTextureImpl();

	void																SetSize(
		vk2d::Vector2u													new_size );
	vk2d::Vector2u														GetSize() const;
	uint32_t															GetLayerCount() const;

	uint32_t															GetCurrentSwapBuffer() const;
	VkImage																GetVulkanImage() const;
	VkImageView															GetVulkanImageView() const;
	VkImageLayout														GetVulkanImageLayout() const;
	VkFramebuffer														GetFramebuffer() const;

	bool																WaitUntilLoaded();

	// Begins the render operations. You must call this before using any drawing commands.
	// For best performance you should calculate game logic first, when you're ready to draw
	// call this function just before your first draw command.
	bool																BeginRender();

	// Ends the rendering operations. You must call this after you're done drawing.
	// This will display the results on screen.
	bool																EndRender();

	bool																SynchronizeFrame();
	bool																WaitIdle();

	// Should be called once render is definitely going to happen. When this is called,
	// SynchronizeFrame() will start blocking until the the contents of the
	// RenderTargerTexture have been fully rendered. BeginRender() can be called however,
	// it will swap the buffers so 2 renders can be queued, however third call to
	// BeginRender() will be blocked until the first BeginRender() call has been rendered.
	// TODO: Figure out how to best track render target texture commitments, a render target can be re-used in multiple places but should only be rendered once while at the same time submissions should be grouped together.
	bool																CommitRenderTargetTextureRender(
		vk2d::_internal::RenderTargetTextureDependencyInfo			&	dependency_info );

	// In case something goes wrong, allows cancelling render commission.
	void																CancelRenderTargetTextureRender(
		vk2d::_internal::RenderTargetTextureDependencyInfo			&	dependency_info );

	void																ResetRenderTargetTextureRenderDependencies(
		uint32_t														swap_buffer_index );

	// Add child dependency, child render targets must render before this.
	void																CheckAndAddRenderTargetTextureDependency(
		uint32_t														swap_buffer_index,
		vk2d::Texture												*	texture );

	vk2d::_internal::RenderTargetTextureDependencyInfo					GetDependencyInfo();

	void																DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>						&	indices,
		const std::vector<vk2d::Vertex>								&	vertices,
		const std::vector<float>									&	texture_channel_weights,
		bool															filled						= true,
		vk2d::Texture												*	texture						= nullptr,
		vk2d::Sampler												*	sampler						= nullptr );

	void																DrawTriangleList(
		const std::vector<uint32_t>									&	raw_indices,
		const std::vector<vk2d::Vertex>								&	vertices,
		const std::vector<float>									&	texture_channel_weights,
		bool															filled						= true,
		vk2d::Texture												*	texture						= nullptr,
		vk2d::Sampler												*	sampler						= nullptr );

	void																DrawLineList(
		const std::vector<vk2d::VertexIndex_2>						&	indices,
		const std::vector<vk2d::Vertex>								&	vertices,
		const std::vector<float>									&	texture_channel_weights,
		vk2d::Texture												*	texture						= nullptr,
		vk2d::Sampler												*	sampler						= nullptr,
		float															line_width					= 1.0f );

	void																DrawLineList(
		const std::vector<uint32_t>									&	raw_indices,
		const std::vector<vk2d::Vertex>								&	vertices,
		const std::vector<float>									&	texture_channel_weights,
		vk2d::Texture												*	texture						= nullptr,
		vk2d::Sampler												*	sampler						= nullptr,
		float															line_width					= 1.0f );

	void																DrawPointList(
		const std::vector<vk2d::Vertex>								&	vertices,
		const std::vector<float>									&	texture_channel_weights,
		vk2d::Texture												*	texture						= nullptr,
		vk2d::Sampler												*	sampler						= nullptr );

	void																DrawMesh(
		const vk2d::Mesh											&	mesh );

	bool																IsGood() const;

private:
	bool																DetermineType();

	bool																CreateCommandBuffers();
	void																DestroyCommandBuffers();

	bool																CreateFrameDataBuffers();
	void																DestroyFrameDataBuffers();

	bool																CreateSurfaces(
		vk2d::Vector2u													new_size );

	void																DestroySurfaces();

	bool																CreateSynchronizationPrimitives();
	void																DestroySynchronizationPrimitives();

	// Record commands to copy an image to the sampled image, then also record commands to update mip maps via
	// vkCmdBlitImage(). Source image must have been created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT.
	// Source image new layout will be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL after this function returns.
	void																CmdBlitMipmapsToSampledImage(
		VkCommandBuffer													command_buffer,
		vk2d::_internal::CompleteImageResource						&	source_image,
		VkImageLayout													source_image_old_layout,
		VkPipelineStageFlagBits											source_image_pipeline_barrier_src_stage );

	void																CmdBindPipelineIfDifferent(
		VkCommandBuffer													command_buffer,
		const vk2d::_internal::PipelineSettings						&	pipeline_settings );

	void																CmdBindTextureSamplerIfDifferent(
		VkCommandBuffer													command_buffer,
		vk2d::Sampler												*	sampler,
		vk2d::Texture												*	texture );

	void																CmdSetLineWidthIfDifferent(
		VkCommandBuffer													command_buffer,
		float															line_width );

	bool																CmdUpdateFrameData(
		VkCommandBuffer													command_buffer );

	struct SwapBuffer
	{
		vk2d::_internal::CompleteImageResource							attachment_image							= {};	// Render attachment, Multisampled, 1 mip level
		vk2d::_internal::CompleteImageResource							buffer_1_image								= {};	// Buffer image, used as multisample resolve and blur buffer
		vk2d::_internal::CompleteImageResource							buffer_2_image								= {};	// Buffer image, used as second blur buffer
		vk2d::_internal::CompleteImageResource							sampled_image								= {};	// Output, sampled image with mip mapping
		VkFramebuffer													vk_framebuffer								= {};
		VkImageLayout													vk_render_image_layout						= {};

		VkCommandBuffer													vk_transfer_command_buffer					= {};
		//VkCommandBuffer													vk_blit_command_buffer						= {};
		VkCommandBuffer													vk_render_command_buffer					= {};
		VkSubmitInfo													vk_transfer_submit_info						= {};
		VkSubmitInfo													vk_render_submit_info						= {};
		VkTimelineSemaphoreSubmitInfo									vk_transfer_timeline_semaphore_submit_info	= {};
		VkTimelineSemaphoreSubmitInfo									vk_render_timeline_semaphore_submit_info	= {};

		VkSemaphore														vk_transfer_to_render_semaphore				= {};	// Using regular semaphore.
		//VkSemaphore														vk_blit_complete_semaphore					= {};	// Using regular semaphore.
		VkSemaphore														vk_render_complete_semaphore				= {};	// Using timeline semaphore, 0 when pending render, 1 when rendered.
		uint64_t														signal_render_complete_semaphore_value		= {};
		VkPipelineStageFlags											transfer_to_render_wait_dst_stage_mask		= {};

		std::vector<vk2d::_internal::RenderTargetTextureDependencyInfo>	render_target_texture_dependencies			= {};

		uint32_t														render_commitment_count						= {};
		std::mutex														render_commitment_mutex						= {};
	};

	vk2d::_internal::InstanceImpl									*	instance									= {};
	vk2d::RenderTargetTextureCreateInfo									create_info_copy							= {};

	vk2d::_internal::RenderTargetTextureType							type										= {};

	VkFormat															surface_format								= {};
	vk2d::Vector2u														size										= {};
	std::vector<VkExtent2D>												mipmap_levels								= {};
	bool																granularity_aligned							= {};

	vk2d::_internal::CompleteBufferResource								frame_data_staging_buffer					= {};
	vk2d::_internal::CompleteBufferResource								frame_data_device_buffer					= {};
	vk2d::_internal::PoolDescriptorSet									frame_data_descriptor_set					= {};

	VkCommandPool														vk_command_pool								= {};
	VkRenderPass														vk_attachment_render_pass					= {};

	std::unique_ptr<vk2d::_internal::MeshBuffer>						mesh_buffer;

	uint32_t															current_swap_buffer							= {};
	std::array<vk2d::_internal::RenderTargetTextureImpl::SwapBuffer, 2>	swap_buffers								= {};

	vk2d::_internal::PipelineSettings									previous_pipeline_settings					= {};
	vk2d::Texture													*	previous_texture							= {};
	vk2d::Sampler													*	previous_sampler							= {};
	float																previous_line_width							= {};

	std::map<vk2d::Sampler*, std::map<vk2d::Texture*, vk2d::_internal::SamplerTextureDescriptorPoolData>>
																		sampler_texture_descriptor_sets				= {};

	bool																is_good										= {};
};



} // _internal

} // vk2d
