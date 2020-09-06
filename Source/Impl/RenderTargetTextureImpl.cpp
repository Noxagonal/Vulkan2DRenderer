
#include "../Header/Core/SourceCommon.h"

#include "../Header/Impl/RenderTargetTextureImpl.h"
#include "../Header/Impl/InstanceImpl.h"
#include "../Header/Core/CommonTools.h"
#include "../Header/Core/ShaderInterface.h"
#include "../Header/Core/MeshBuffer.h"

#include <iostream>
#include <memory>


vk2d::_internal::RenderTargetTextureImpl::RenderTargetTextureImpl(
	vk2d::_internal::InstanceImpl				*	instance,
	const vk2d::RenderTargetTextureCreateInfo	&	create_info
) :
vk2d::_internal::TextureImpl(
	instance
)
{
	this->instance			= instance;
	assert( this->instance );

	create_info_copy		= create_info;

	if( !CreateCommandBuffers() ) return;
	if( !CreateFrameDataBuffers() ) return;
	if( !CreateSurfaces( create_info.size ) ) return;
	if( !CreateSynchronizationPrimitives() ) return;

	mesh_buffer		= std::make_unique<vk2d::_internal::MeshBuffer>(
		instance,
		instance->GetVulkanDevice(),
		instance->GetVulkanPhysicalDeviceProperties().limits,
		instance->GetDeviceMemoryPool()
	);

	is_good					= true;
}


vk2d::_internal::RenderTargetTextureImpl::~RenderTargetTextureImpl()
{
	DestroySynchronizationPrimitives();
	DestroySurfaces();
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
				semaphores[ i ]			= swap_buffers[ i ].vk_semaphore;
				semaphore_values[ i ]	= 1;
			}
			VkSemaphoreWaitInfo wait_info {};
			wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
			wait_info.pNext				= nullptr;
			wait_info.flags				= 0;
			wait_info.semaphoreCount	= uint32_t( std::size( semaphores ) );
			wait_info.pSemaphores		= semaphores.data();
			wait_info.pValues			= semaphore_values.data();
			if( vkWaitSemaphores(
				instance->GetVulkanDevice(),
				&wait_info,
				5 * 1000 * 1000 * uint64_t( 1000 )	// wait for 5 seconds
			) != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Error destroying RenderTargetTexture, timeout while waiting for render to finish!" );
			}
		}

		DestroySurfaces();
		CreateSurfaces(
			new_size
		);
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
	return swap_buffers[ current_swap_buffer ].surface.render_image.image;
}

VkImageView vk2d::_internal::RenderTargetTextureImpl::GetVulkanImageView() const
{
	return swap_buffers[ current_swap_buffer ].surface.render_image.view;
}

VkImageLayout vk2d::_internal::RenderTargetTextureImpl::GetVulkanImageLayout() const
{
	return swap_buffers[ current_swap_buffer ].surface.vk_render_image_layout;
}

VkFramebuffer vk2d::_internal::RenderTargetTextureImpl::GetFramebuffer() const
{
	return swap_buffers[ current_swap_buffer ].surface.vk_framebuffer;
}

bool vk2d::_internal::RenderTargetTextureImpl::WaitUntilLoaded()
{
	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::BeginRender()
{
	++current_swap_buffer;
	if( current_swap_buffer >= uint32_t( std::size( swap_buffers ) ) ) current_swap_buffer = 0;

	{
		using namespace std::chrono;
		using namespace std::chrono_literals;

		#if 0
		// Debugging code
		{
			uint64_t semaphore_value = 0;
			vkGetSemaphoreCounterValue(
				instance->GetVulkanDevice(),
				swap_buffers[ current_swap_buffer ].vk_semaphore,
				&semaphore_value
			);
			std::cout << "Semaphore value: " << semaphore_value << "\n";
		}
		#endif

		// Wait here for the semaphore to signal, this takes care of synchronization
		// just in case buffer we're trying to modify hasn't finished rendering.
		uint64_t semaphore_wait_value = 1;
		VkSemaphoreWaitInfo semaphore_wait_info {};
		semaphore_wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		semaphore_wait_info.pNext				= nullptr;
		semaphore_wait_info.flags				= 0;
		semaphore_wait_info.semaphoreCount		= 1;
		semaphore_wait_info.pSemaphores			= &swap_buffers[ current_swap_buffer ].vk_semaphore;
		semaphore_wait_info.pValues				= &semaphore_wait_value;
		if( vkWaitSemaphores(
			instance->GetVulkanDevice(),
			&semaphore_wait_info,
			duration_cast<nanoseconds>( 5s ).count()
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Semaphore wait timeout!" );
			return false;
		}

		// Clear semaphore to indicate it's pending execution.
		VkSemaphoreSignalInfo semaphore_signal_info {};
		semaphore_signal_info.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		semaphore_signal_info.pNext			= nullptr;
		semaphore_signal_info.semaphore		= swap_buffers[ current_swap_buffer ].vk_semaphore;
		semaphore_signal_info.value			= 0;
		if( vkSignalSemaphore(
			instance->GetVulkanDevice(),
			&semaphore_signal_info
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot signal semaphore at begin render." );
			return false;
		}

		#if 0
		// Debugging code
		{
			uint64_t semaphore_value = 0;
			vkGetSemaphoreCounterValue(
				instance->GetVulkanDevice(),
				swap_buffers[ current_swap_buffer ].vk_semaphore,
				&semaphore_value
			);
			std::cout << "Semaphore value: " << semaphore_value << "\n";
		}
		#endif
	}

	// Begin command buffer
	{
		VkCommandBuffer		command_buffer			= swap_buffers[ current_swap_buffer ].vk_render_command_buffer;
		auto			&	surface					= swap_buffers[ current_swap_buffer ].surface;

		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext				= nullptr;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo	= nullptr;

		if( vkBeginCommandBuffer(
			command_buffer,
			&command_buffer_begin_info
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record primary render command buffer!" );
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
				instance->GetVulkanPipelineLayout(),
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
			render_pass_begin_info.renderPass		= vk_render_pass;
			render_pass_begin_info.framebuffer		= surface.vk_framebuffer;
			render_pass_begin_info.renderArea		= { { 0, 0 }, { size.x, size.y } };
			render_pass_begin_info.clearValueCount	= 1;
			render_pass_begin_info.pClearValues		= &clear_value;

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

bool vk2d::_internal::RenderTargetTextureImpl::EndRender()
{
	VkCommandBuffer		render_command_buffer	= swap_buffers[ current_swap_buffer ].vk_render_command_buffer;
	VkCommandBuffer		transfer_command_buffer	= swap_buffers[ current_swap_buffer ].vk_transfer_command_buffer;

	// End render pass.
	{
		vk2d::_internal::CmdInsertCommandBufferCheckpoint(
			render_command_buffer,
			"RenderTargetTextureImpl",
			vk2d::_internal::CommandBufferCheckpointType::END_RENDER_PASS
		);
		vkCmdEndRenderPass( render_command_buffer );
	}

	// Record commands to blur the image if blurring was activated.
	{
		// TODO Record commands to blur the image if blurring was activated.
	}

	// End command buffer
	vk2d::_internal::CmdInsertCommandBufferCheckpoint(
		render_command_buffer,
		"RenderTargetTextureImpl",
		vk2d::_internal::CommandBufferCheckpointType::END_COMMAND_BUFFER
	);
	if( vkEndCommandBuffer( render_command_buffer ) != VK_SUCCESS ) {
		instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot compile primary render command buffer!" );
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
				transfer_command_buffer,
				&transfer_command_buffer_begin_info
			) != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record mesh to GPU transfer command buffer!" );
				return false;
			}
		}

		// Record commands to upload frame data to gpu
		{
			if( !CmdUpdateFrameData(
				transfer_command_buffer
			) ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record commands to transfer FrameData to GPU!" );
				return false;
			}
		}

		// Record commands to upload mesh data to gpu
		{
			if( !mesh_buffer->CmdUploadMeshDataToGPU(
				transfer_command_buffer
			) ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot record commands to transfer mesh data to GPU!" );
				return false;
			}
		}

		// End command buffer
		{
			if( vkEndCommandBuffer(
				transfer_command_buffer
			) != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot render to RenderTargetTexture, Cannot compile mesh to GPU transfer command buffer!" );
				return false;
			}
		}
	}

	// Now that everything is recorded into a command buffer we'll
	// store the command buffers until they're used in a render.
	// Window will have to order the render target command buffers
	// so that the deepest render target gets processed first.
	// Render targets allow multi-core processing as well by using
	// multiple CPU cores to render different render targets,
	// Window object will combine everything for us and take care
	// of render target dependency tree.

	return true;
}

bool vk2d::_internal::RenderTargetTextureImpl::IsGood() const
{
	return is_good;
}

bool vk2d::_internal::RenderTargetTextureImpl::CreateCommandBuffers()
{
	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext				= nullptr;
	command_pool_create_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex	= instance->GetPrimaryRenderQueue().GetQueueFamilyIndex();
	if( vkCreateCommandPool(
		instance->GetVulkanDevice(),
		&command_pool_create_info,
		nullptr,
		&vk_command_pool
	) != VK_SUCCESS ) {
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create command pool!" );
		return false;
	}

	{
		std::vector<VkCommandBuffer> allocated_command_buffers( swap_buffers.size() * 2 );
		{
			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext				= nullptr;
			command_buffer_allocate_info.commandPool		= vk_command_pool;
			command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount	= uint32_t( allocated_command_buffers.size() );
			if( vkAllocateCommandBuffers(
				instance->GetVulkanDevice(),
				&command_buffer_allocate_info,
				allocated_command_buffers.data()
			) != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create command pool!" );
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
		vk_command_pool,
		nullptr
	);
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
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error. Cannot create staging buffer for FrameData!" );
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
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error. Cannot create device local buffer for FrameData!" );
			return false;
		}
	}

	// Create descriptor set
	{
		frame_data_descriptor_set	= instance->GetDescriptorPool()->AllocateDescriptorSet(
			instance->GetUniformBufferDescriptorSetLayout()
		);
		if( frame_data_descriptor_set != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot allocate descriptor set for FrameData device buffer!" );
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
	instance->GetDescriptorPool()->FreeDescriptorSet(
		frame_data_descriptor_set
	);
	instance->GetDeviceMemoryPool()->FreeCompleteResource(
		frame_data_device_buffer
	);
	instance->GetDeviceMemoryPool()->FreeCompleteResource(
		frame_data_staging_buffer
	);
}



bool vk2d::_internal::RenderTargetTextureImpl::CreateSurfaces(
	vk2d::Vector2u		new_size
)
{
	constexpr VkFormat surface_format = VK_FORMAT_R8G8B8A8_UNORM;

	size						= new_size;

	mipmap_levels				= vk2d::_internal::GenerateMipSizes(
		create_info_copy.size
	);
	bool use_multisampling		= create_info_copy.samples != vk2d::Multisamples::SAMPLE_COUNT_1;

	{
		std::vector<VkAttachmentDescription> attachments {};
		if( use_multisampling ) {
			attachments.resize( 2 );
		} else {
			attachments.resize( 1 );
		}

		attachments[ 0 ].flags				= 0;
		attachments[ 0 ].format				= surface_format;
		attachments[ 0 ].samples			= VkSampleCountFlagBits( create_info_copy.samples );
		attachments[ 0 ].loadOp				= VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[ 0 ].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachments[ 0 ].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[ 0 ].stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[ 0 ].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[ 0 ].finalLayout		= use_multisampling ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if( use_multisampling ) {
			// Create resolve target
			attachments[ 1 ].flags			= 0;
			attachments[ 1 ].format			= surface_format;
			attachments[ 1 ].samples		= VK_SAMPLE_COUNT_1_BIT;
			attachments[ 1 ].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[ 1 ].storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
			attachments[ 1 ].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[ 1 ].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[ 1 ].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[ 1 ].finalLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		std::array<VkAttachmentReference, 1> color_attachment_references {};
		color_attachment_references[ 0 ].attachment		= 0;
		color_attachment_references[ 0 ].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference resolve_attachment_reference {};
		resolve_attachment_reference.attachment			= 1;	// points to color_attachment_descriptions[ 1 ]
		resolve_attachment_reference.layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		std::array<VkSubpassDescription, 1> subpasses {};
		subpasses[ 0 ].flags					= 0;
		subpasses[ 0 ].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[ 0 ].inputAttachmentCount		= 0;
		subpasses[ 0 ].pInputAttachments		= nullptr;
		subpasses[ 0 ].colorAttachmentCount		= uint32_t( color_attachment_references.size() );
		subpasses[ 0 ].pColorAttachments		= color_attachment_references.data();
		subpasses[ 0 ].pResolveAttachments		= use_multisampling ? &resolve_attachment_reference : nullptr;
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

		if( vkCreateRenderPass(
			instance->GetVulkanDevice(),
			&render_pass_create_info,
			nullptr,
			&vk_render_pass
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create render pass!" );
			return false;
		}
	}

	// TODO: Add blur render passes.

	for( auto & s : swap_buffers ) {
		VkImageCreateInfo image_create_info {};
		image_create_info.sType						= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.pNext						= nullptr;
		image_create_info.flags						= 0;
		image_create_info.imageType					= VK_IMAGE_TYPE_2D;
		image_create_info.format					= surface_format;
		image_create_info.extent					= { size.x, size.y, 1 };
		image_create_info.mipLevels					= uint32_t( mipmap_levels.size() );
		image_create_info.arrayLayers				= 1;
		image_create_info.samples					= VkSampleCountFlagBits( create_info_copy.samples );
		image_create_info.tiling					= VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage						=
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT |
			( use_multisampling ? 0 : ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT ) );
		image_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.queueFamilyIndexCount		= 0;
		image_create_info.pQueueFamilyIndices		= nullptr;
		image_create_info.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext				= nullptr;
		image_view_create_info.flags				= 0;
		image_view_create_info.image				= VK_NULL_HANDLE;
		image_view_create_info.viewType				= VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format				= surface_format;
		image_view_create_info.components			= {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		image_view_create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel	= 0;
		image_view_create_info.subresourceRange.levelCount		= uint32_t( mipmap_levels.size() );
		image_view_create_info.subresourceRange.baseArrayLayer	= 0;
		image_view_create_info.subresourceRange.layerCount		= 1;

		s.surface.render_image	= instance->GetDeviceMemoryPool()->CreateCompleteImageResource(
			&image_create_info,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&image_view_create_info
		);
		if( s.surface.render_image != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create render image!" );
			return false;
		}

		// Create multisampling resolve image.
		if( use_multisampling ) {
			image_create_info.samples		= VK_SAMPLE_COUNT_1_BIT;
			image_create_info.usage			=
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			s.surface.resolve_image					= instance->GetDeviceMemoryPool()->CreateCompleteImageResource(
				&image_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&image_view_create_info
			);
			if( s.surface.resolve_image != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create multisample resolve image!" );
				return false;
			}
		}

		// Create blur buffer image
		if( create_info_copy.enable_blur ) {
			image_create_info.samples		= VK_SAMPLE_COUNT_1_BIT;
			image_create_info.usage			=
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			s.surface.blur_buffer_image				= instance->GetDeviceMemoryPool()->CreateCompleteImageResource(
				&image_create_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&image_view_create_info
			);
			if( s.surface.blur_buffer_image != VK_SUCCESS ) {
				instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create multisample resolve image!" );
				return false;
			}
		}


		VkImageView attachment					= s.surface.render_image.view;
		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.pNext			= nullptr;
		framebuffer_create_info.flags			= 0;
		framebuffer_create_info.renderPass		= vk_render_pass;
		framebuffer_create_info.attachmentCount	= 1;
		framebuffer_create_info.pAttachments	= &attachment;
		framebuffer_create_info.width			= size.x;
		framebuffer_create_info.height			= size.y;
		framebuffer_create_info.layers			= 1;

		if( vkCreateFramebuffer(
			instance->GetVulkanDevice(),
			&framebuffer_create_info,
			nullptr,
			&s.surface.vk_framebuffer
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create framebuffer!" );
			return false;
		}

		s.surface.vk_render_image_layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroySurfaces()
{
	for( auto & s : swap_buffers ) {
		vkDestroyFramebuffer(
			instance->GetVulkanDevice(),
			s.surface.vk_framebuffer,
			nullptr
		);

		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.surface.render_image );
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.surface.resolve_image );
		instance->GetDeviceMemoryPool()->FreeCompleteResource( s.surface.blur_buffer_image );
	}

	vkDestroyRenderPass(
		instance->GetVulkanDevice(),
		vk_render_pass,
		nullptr
	);
}

bool vk2d::_internal::RenderTargetTextureImpl::CreateSynchronizationPrimitives()
{
	// We need timeline semaphores, even if we don't really use the timeline feature,
	// the primary difference is that timeline semaphores can be waited on without
	// automatically resetting their value. We can manually reset the value when we
	// begin recording the render target command buffer and count on the GPU / driver
	// to set it once finished and then use the same semaphore to indicate the render
	// target texture is ready to be used. As a bonus we can also wait on the timeline
	// semaphore on CPU, making host synchronization a bit easier to implement.
	VkSemaphoreTypeCreateInfo semaphore_type_create_info {};
	semaphore_type_create_info.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	semaphore_type_create_info.pNext			= nullptr;
	semaphore_type_create_info.semaphoreType	= VK_SEMAPHORE_TYPE_TIMELINE;
	semaphore_type_create_info.initialValue		= 1;

	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType		= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext		= &semaphore_type_create_info;
	semaphore_create_info.flags		= 0;

	for( auto & s : swap_buffers ) {
		if( vkCreateSemaphore(
			instance->GetVulkanDevice(),
			&semaphore_create_info,
			nullptr,
			&s.vk_semaphore
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create RenderTargetTexture, cannot create synchronization primitives!" );
			return false;
		}
	}
	return true;
}

void vk2d::_internal::RenderTargetTextureImpl::DestroySynchronizationPrimitives()
{
	{
		std::array<VkSemaphore,	std::tuple_size_v<decltype( swap_buffers )>> semaphores;
		std::array<uint64_t,	std::tuple_size_v<decltype( swap_buffers )>> semaphore_values;
		for( size_t i = 0; i < std::size( swap_buffers ); ++i ) {
			semaphores[ i ]			= swap_buffers[ i ].vk_semaphore;
			semaphore_values[ i ]	= 1;
		}
		VkSemaphoreWaitInfo wait_info {};
		wait_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		wait_info.pNext				= nullptr;
		wait_info.flags				= 0;
		wait_info.semaphoreCount	= uint32_t( std::size( semaphores ) );
		wait_info.pSemaphores		= semaphores.data();
		wait_info.pValues			= semaphore_values.data();
		if( vkWaitSemaphores(
			instance->GetVulkanDevice(),
			&wait_info,
			5 * 1000 * 1000 * uint64_t( 1000 )	// wait for 5 seconds
		) != VK_SUCCESS ) {
			instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Error destroying RenderTargetTexture, timeout while waiting for render to finish!" );
		}
	}

	for( auto & s : swap_buffers ) {
		vkDestroySemaphore(
			instance->GetVulkanDevice(),
			s.vk_semaphore,
			nullptr
		);
	}
}

bool vk2d::_internal::RenderTargetTextureImpl::CmdUpdateFrameData( VkCommandBuffer command_buffer )
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
