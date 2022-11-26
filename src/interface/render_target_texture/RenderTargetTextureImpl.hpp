#pragma once

#include <core/SourceCommon.hpp>

#include <interface/RenderTargetTexture.hpp>

#include <containers/BlurType.hpp>

#include <system/CommonTools.hpp>
#include <vulkan/shaders/ShaderInterface.hpp>
#include <system/MeshBuffer.hpp>
#include <system/RenderTargetTextureDependecyGraphInfo.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>

#include <interface/instance/InstanceImpl.hpp>

#include <interface/Texture.hpp>
#include "../texture/TextureImpl.hpp"

#include "../sampler/SamplerImpl.hpp"




namespace vk2d {
namespace vk2d_internal {

class InstanceImpl;
class MeshBuffer;
class RenderTargetTextureImpl;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Render target texture type.
///
///				Used to select the implementation. Each works a bit differently from the others. We use 2 to 4 Vulkan images
///				depending on the implementation used. "Attachment" is the first render, it alone can have multisample enabled,
///				"Sampled" is the final product of this pipeline, it alone can have multiple mip maps, Buffer1 and Buffer2 are
///				used as needed as intermediate render target images, those must always be 1 sample and 1 mipmap. <br>
///				Depending on the type of render target texture used, different rendering paths are used with different amount of
///				images.<br>
///				<table>
///				<caption>Images</caption>
///				<tr><th> Name					<th> Description
///				<tr><td> Attachment				<td> Render attachment image where everything is rendered by the main pass.
///				<tr><td> Sampled				<td> Image that's presentable in a shader. The part that can be used as a texture.
///				<tr><td> Buffer 1				<td> Intermediate image that's used to temporarily store image data for further processing.
///				<tr><td> Buffer 2				<td> Same as Buffer 1.
///				</table>
///				<br>
///				<table>
///				<caption>Finalization pipeline</caption>
///				<tr><th> No multisample, No blur	<th> With multisample, No blur	<th> No multisample, With blur	<th> With multisample, With blur
///				<tr><td> (Attachment)				<td> (Attachment)				<td> (Attachment)				<td> (Attachment)
///				<tr><td><b> Generate mip maps		<td> Resolve multisamples		<td> Blur pass 1				<td> Resolve multisamples	</b>
///				<tr><td> (Sampled)					<td> (Buffer 1)					<td> (Buffer 1)					<td> (Buffer 1)
///				<tr><td><b>							<td> Generate mip maps			<td> Blur pass 2				<td> Blur pass 1			</b>
///				<tr><td>							<td> (Sampled)					<td> (Attachment)				<td> (Buffer 2)
///				<tr><td><b>							<td>							<td> Generate mip maps			<td> Blur pass 2			</b>
///				<tr><td>							<td>							<td> (Sampled)					<td> (Buffer 1)
///				<tr><td><b>							<td>							<td>							<td> Generate mip maps		</b>
///				<tr><td>							<td>							<td>							<td> (Sampled)
///				</table>
enum class RenderTargetTextureType
{
	/// @brief		Not a type, used for error detection.
	NONE						= 0,

	/// @brief		No multisample, No blur.
	DIRECT,

	/// @brief		With multisample, No blur.
	WITH_MULTISAMPLE,

	/// @brief		No multisample, With blur.
	WITH_BLUR,

	/// @brief		With multisample, with blur
	WITH_MULTISAMPLE_AND_BLUR,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderTargetTextureImpl :
	public TextureImpl
{
private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct SwapBuffer
	{
		vulkan::CompleteImageResource							attachment_image							= {};	// Render attachment, Multisampled, 1 mip level
		vulkan::CompleteImageResource							buffer_1_image								= {};	// Buffer image, used as multisample resolve and blur buffer
		vulkan::CompleteImageResource							buffer_2_image								= {};	// Buffer image, used as second blur buffer
		vulkan::CompleteImageResource							sampled_image								= {};	// Output, sampled image with mip mapping
		VkFramebuffer											vk_render_framebuffer						= {};	// Framebuffer for the main render
		VkFramebuffer											vk_blur_framebuffer_1						= {};	// Framebuffer for blur pass 1
		VkFramebuffer											vk_blur_framebuffer_2						= {};	// Framebuffer for blur pass 2

		VkCommandBuffer											vk_transfer_command_buffer					= {};	// Data transfer command buffer, this transfers vertex, index, etc... data in the primary render queue.
		VkCommandBuffer											vk_render_command_buffer					= {};	// Primary render, if no blur is used then also embeds mipmap generation.

		VkSubmitInfo											vk_transfer_submit_info						= {};
		VkSubmitInfo											vk_render_submit_info						= {};

		VkTimelineSemaphoreSubmitInfo							vk_render_timeline_semaphore_submit_info	= {};

		std::vector<VkSemaphore>								render_wait_for_semaphores;
		std::vector<uint64_t>									render_wait_for_semaphore_timeline_values;			// Used with render_wait_for_semaphores.
		std::vector<VkPipelineStageFlags>						render_wait_for_pipeline_stages;

		VkSemaphore												vk_transfer_complete_semaphore				= {};	// Binary
		VkSemaphore												vk_render_complete_semaphore				= {};	// Binary if blur enabled, Timeline if blur enabled.

		uint64_t												render_counter								= {};	// Used with the vk_render_complete_semaphore to determine value to wait for.

		std::vector<RenderTargetTextureDependencyInfo>			render_target_texture_dependencies			= {};

		uint32_t												render_commitment_request_count				= {};
		//std::mutex											render_commitment_request_mutex				= {};

		bool													has_been_submitted							= {};
		bool													contains_non_pending_sampled_image			= {};	// Sampled image ready to be used anywhere without checks or barriers.
	};

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RenderTargetTextureImpl(
		RenderTargetTexture									&	my_interface,
		InstanceImpl										&	instance,
		const RenderTargetTextureCreateInfo					&	create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~RenderTargetTextureImpl();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void														SetRenderCoordinateSpace(
		RenderCoordinateSpace									coordinate_space );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void														SetSize(
		glm::uvec2												new_size
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	glm::uvec2													GetSize() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t													GetLayerCount() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t													GetCurrentSwapBuffer() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkImage														GetVulkanImage() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkImageView													GetVulkanImageView() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkImageLayout												GetVulkanImageLayout() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkFramebuffer												GetFramebuffer(
		RenderTargetTextureDependencyInfo					&	dependency_info
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkSemaphore													GetAllCompleteSemaphore(
		RenderTargetTextureDependencyInfo					&	dependency_info
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint64_t													GetRenderCounter(
		RenderTargetTextureDependencyInfo					&	dependency_info
	) const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool														IsTextureDataReady();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Begins the render of the render target texture.
	///
	/// @see		vk2d::Window::BeginRender()
	///
	/// @return		true if successful, false if something went wrong.
	bool														BeginRender();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Ends the render of the render target texture.
	///
	/// @see		vk2d::Window::EndRender()
	///
	/// @return		true if successful, false if something went wrong.
	bool														EndRender(
		BlurType												blur_type,
		glm::vec2												blur_amount );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool														SynchronizeFrame();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool														WaitIdle();

	// TODO: Figure out how to best track render target texture commitments, a render target can be re-used in multiple places but should only be rendered once while at the same time submissions should be grouped together.

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Signals that the render target texture is going to be rendered.
	///
	///				Should be called once render is definitely going to happen.
	///
	/// @note		Recursively commits the all render target textures that this render target texure depends on.
	///
	/// @note		When this is called, SynchronizeFrame() will start blocking until the the contents of the RenderTargerTexture
	///				have been fully rendered. BeginRender() can be called however, it will swap the buffers so 2 renders can be
	///				queued, however third call to BeginRender() will be blocked until the first BeginRender() call has been
	///				rendered.
	///
	/// @param		dependency_info
	///				Tells which part of the render target texture should be rendered.
	/// 
	/// @param		collector
	///				List of render target textures that will be rendered.
	///
	/// @return 
	bool														CommitRenderTargetTextureRender(
		RenderTargetTextureDependencyInfo					&	dependency_info,
		RenderTargetTextureRenderCollector					&	collector
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		This sets a flag that the render target texture has been submitted for render.
	///
	/// @note		Recursively sets the same flag for all render target textures that this render target texure depends on.
	///
	/// @param		dependency_info 
	///				Tells which part of the render target texture should be confirmed.
	void														ConfirmRenderTargetTextureRenderSubmission(
		RenderTargetTextureDependencyInfo					&	dependency_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		This sets a flag that the render target texture has been successfully rendered.
	/// 
	/// @note		Recursively sets the same flag for all render target textures that this render target texure depends on.
	///
	/// @param		dependency_info 
	///				Tells which part of the render target texture has completed rendering.
	void														ConfirmRenderTargetTextureRenderFinished(
		RenderTargetTextureDependencyInfo					&	dependency_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Aborts render target texture rendering.
	/// 
	/// @note		Recursively clears render target from render queue for all render target textures that this render target
	///				texure depends on.
	///
	/// @param		dependency_info 
	///				Tells which part of the render target texture is aborted.
	void														AbortRenderTargetTextureRender(
		RenderTargetTextureDependencyInfo					&	dependency_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Resets render target texture dependencies.
	///
	///				This is used at the end of the render to clear all dependencies. Once the render has completed, there's no need
	///				to keep track of which render target texture depends on another.
	///
	/// @note		This is not a recursive operation.
	///
	/// @param		swap_buffer_index
	///				Each swap buffer tracks it's own dependencies, this tells which swap buffer should be cleared of dependencies.
	void														ResetRenderTargetTextureRenderDependencies(
		uint32_t												swap_buffer_index
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Add dependency to another render target texture if it isn't already added.
	///
	///				Added dependency is rendered before this render target texture.
	/// 
	/// @param		swap_buffer_index
	///				Each swap buffer tracks it's own dependencies, this tells which swap buffer the dependency should be added to.
	/// 
	/// @param		texture
	///				Handle to render target texture that must render before this render target texture.
	void														CheckAndAddRenderTargetTextureDependency(
		uint32_t												swap_buffer_index,
		Texture												*	texture
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RenderTargetTextureDependencyInfo							GetDependencyInfo();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void														DrawPointList(
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		Texture												*	texture,
		Sampler												*	sampler
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void														DrawLineList(
		std::span<const uint32_t>								raw_indices,
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		Texture												*	texture,
		Sampler												*	sampler,
		float													line_width
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void														DrawTriangleList(
		std::span<const uint32_t>								raw_indices,
		const RawVertexData									&	raw_vertex_data,
		std::span<const glm::mat4>								transformations,
		bool													solid,
		Texture												*	texture,
		Sampler												*	sampler
	);

	bool														IsGood() const;

private:
	bool														DetermineType();

	bool														CreateCommandBuffers();
	void														DestroyCommandBuffers();

	bool														CreateFrameDataBuffers();
	void														DestroyFrameDataBuffers();

	bool														CreateImages(
		glm::uvec2												new_size );
	void														DestroyImages();

	bool														CreateRenderPasses();
	void														DestroyRenderPasses();

	bool														CreateFramebuffers();
	void														DestroyFramebuffers();

	bool														CreateSynchronizationPrimitives();
	void														DestroySynchronizationPrimitives();

	bool														RecordTransferCommandBuffer(
		RenderTargetTextureImpl::SwapBuffer					&	swap
	);

	bool														UpdateSubmitInfos(
		RenderTargetTextureImpl::SwapBuffer					&	swap,
		const std::vector<VkSemaphore>						&	wait_for_semaphores,
		const std::vector<uint64_t>							&	wait_for_semaphore_timeline_values,
		const std::vector<VkPipelineStageFlags>				&	wait_for_semaphore_pipeline_stages
	);

	TimedDescriptorPoolData									&	GetOrCreateDescriptorSetForSampler(
		Sampler												*	sampler
	);

	TimedDescriptorPoolData									&	GetOrCreateDescriptorSetForTexture(
		Texture												*	texture
	);

	void														CmdPushBlurTextureDescriptorWritesDirectly(
		VkCommandBuffer											command_buffer,
		VkPipelineLayout										use_pipeline_layout,
		uint32_t												set_index,
		VkImageView												source_image,
		VkImageLayout											source_image_layout
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Record commands to finalize render into the sampled image.
	/// 
	///				This includes resolving multisamples, blur, mipmap generation and storing the result into sampled image to be
	///				used later as a texture.
	///				<br>
	///				Finalization goes through multiple stages and uses multiple buffers.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		RenderTargetTextureType
	/// 
	/// @param[in]	swap
	///				Reference to internal structure which contains all the information about the current frame.
	void														CmdFinalizeRender(
		RenderTargetTextureImpl::SwapBuffer					&	swap,
		BlurType												blur_type,
		glm::vec2												blur_amount
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Record commands to copy an image to the final sampled image, then generate mipmaps for it.
	///
	///				Called by RenderTargetTextureImpl::CmdFinalizeNonBlurredRender().
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	command_buffer
	///				Command buffer where to record mipmap blit commands to.
	/// 
	/// @param[in]	source_image
	///				Reference to image object from where data is copied and blitted from. Only mip level 0 is accessed. Must have
	///				been created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT flag. After this function returns source image layout will be
	///				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
	/// 
	/// @param[in]	source_image_layout
	///				Source image current layout. See Vulkan documentation about VkImageLayout.
	/// 
	/// @param[in]	source_image_pipeline_barrier_src_stage
	///				Vulkan pipeline stage flags that must complete before source image data is accessed.
	/// 
	/// @param[in]	destination_image
	///				Sampled image to be used as shader read only optimal, has to have correct amount of mip levels.
	void														CmdBlitMipmapsToSampledImage(
		VkCommandBuffer											command_buffer,
		vulkan::CompleteImageResource						&	source_image,
		VkImageLayout											source_image_layout,
		VkPipelineStageFlagBits									source_image_pipeline_barrier_src_stage,
		vulkan::CompleteImageResource						&	destination_image
	);

	bool														CmdRecordBlurCommands(
		RenderTargetTextureImpl::SwapBuffer					&	swap,
		VkCommandBuffer											command_buffer,
		BlurType												blur_type,
		glm::vec2												blur_amount,
		vulkan::CompleteImageResource						&	source_image,
		VkImageLayout											source_image_layout,
		VkPipelineStageFlagBits									source_image_pipeline_barrier_src_stage,
		vulkan::CompleteImageResource						&	intermediate_image,
		vulkan::CompleteImageResource						&	destination_image
	);

	void														CmdBindGraphicsPipelineIfDifferent(
		VkCommandBuffer											command_buffer,
		const vulkan::GraphicsPipelineSettings				&	pipeline_settings
	);

	void														CmdBindSamplerIfDifferent(
		VkCommandBuffer											command_buffer,
		Sampler												*	sampler,
		VkPipelineLayout										use_pipeline_layout
	);

	void														CmdBindTextureIfDifferent(
		VkCommandBuffer											command_buffer,
		Texture												*	texture,
		VkPipelineLayout										use_pipeline_layout
	);

	void														CmdSetLineWidthIfDifferent(
		VkCommandBuffer											command_buffer,
		float													line_width
	);

	bool														CmdUpdateFrameData(
		VkCommandBuffer											command_buffer
	);

	RenderTargetTexture										&	my_interface;
	InstanceImpl											&	instance;
	RenderTargetTextureCreateInfo								create_info_copy							= {};

	RenderTargetTextureType										type										= {};

	VkFormat													surface_format								= {};
	glm::uvec2													size										= {};
	RenderCoordinateSpace										coordinate_space							= {};
	Multisamples												samples										= {};
	std::vector<VkExtent2D>										mipmap_levels								= {};
	bool														granularity_aligned							= {};

	vulkan::CompleteBufferResource								frame_data_staging_buffer					= {};
	vulkan::CompleteBufferResource								frame_data_device_buffer					= {};
	vulkan::PoolDescriptorSet									frame_data_descriptor_set					= {};

	VkCommandPool												vk_graphics_command_pool					= {};
	//VkCommandPool												vk_compute_command_pool						= {};

	VkRenderPass												vk_attachment_render_pass					= {};
	VkRenderPass												vk_blur_render_pass_1						= {};
	VkRenderPass												vk_blur_render_pass_2						= {};

	std::unique_ptr<MeshBuffer>									mesh_buffer;

	uint32_t													current_swap_buffer							= {};
	std::array<RenderTargetTextureImpl::SwapBuffer, 2>			swap_buffers								= {};

	VkImageLayout												vk_attachment_image_final_layout			= {};
	VkImageLayout												vk_sampled_image_final_layout				= {};
	VkAccessFlags												vk_sampled_image_final_access_mask			= {};

	vulkan::GraphicsPipelineSettings							previous_graphics_pipeline_settings			= {};
	Texture													*	previous_texture							= {};
	Sampler													*	previous_sampler							= {};
	float														previous_line_width							= {};

	std::map<Sampler*, TimedDescriptorPoolData>					sampler_descriptor_sets						= {};
	std::map<Texture*, TimedDescriptorPoolData>					texture_descriptor_sets						= {};
	std::map<VkImageView, std::map<VkImageView, TimedDescriptorPoolData>>
																image_descriptor_sets						= {};

	bool														is_good										= {};
};



} // vk2d_internal

} // vk2d
