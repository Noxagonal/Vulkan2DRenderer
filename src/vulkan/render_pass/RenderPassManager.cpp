
#include <core/SourceCommon.hpp>
#include "RenderPassManager.hpp"

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManager::RenderPassManager(
	vk2d_internal::InstanceImpl		&	instance,
	Device							&	vulkan_device
) :
	instance( instance ),
	vulkan_device( vulkan_device )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManager::~RenderPassManager()
{
	DestroyAllRenderPasss();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassHandle vk2d::vulkan::RenderPassManager::FindRenderPass(
	const RenderPassCreateInfo & render_pass_create_info
)
{
	return FindRenderPass( render_pass_create_info.GetHash() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassHandle vk2d::vulkan::RenderPassManager::FindRenderPass(
	size_t render_pass_hash
)
{
	return render_pass_list(
		[ this, render_pass_hash ]( RenderPassList & list ) -> RenderPassHandle
		{
			auto render_pass = list.find( render_pass_hash );
			if( render_pass == list.end() ) return {};

			return RenderPassHandle(
				this,
				&render_pass->second
			);
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassHandle vk2d::vulkan::RenderPassManager::GetRenderPass(
	const RenderPassCreateInfo & render_pass_create_info
)
{
	auto existing_render_pass = FindRenderPass( render_pass_create_info );
	if( existing_render_pass ) return existing_render_pass;

	return CreateRenderPass( render_pass_create_info );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassHandle vk2d::vulkan::RenderPassManager::CreateRenderPass(
	const RenderPassCreateInfo	&	render_pass_create_info
)
{
	/*
	using MemoryPoolResource					= std::pmr::unsynchronized_pool_resource;
	using MemoryPoolAllocator					= std::pmr::polymorphic_allocator<std::byte>;

	using AttachmentDescriptionList				= std::pmr::vector<VkAttachmentDescription2>;
	using AttachmentReferenceListList			= std::pmr::vector<std::pmr::vector<VkAttachmentReference2>>;
	using AttachmentReferenceList				= std::pmr::vector<VkAttachmentReference2>;
	using PreserveAttachmentReferenceListList	= std::pmr::vector<std::pmr::vector<uint32_t>>;
	using SubpassDepthStencilResolveList		= std::pmr::vector<VkSubpassDescriptionDepthStencilResolve>;
	using SubpassDescriptionList				= std::pmr::vector<VkSubpassDescription2>;

	auto memory_resource = MemoryPoolResource();
	auto allocator = MemoryPoolAllocator( &memory_resource );

	auto attachments = [ &render_pass_create_info, &allocator ]() -> AttachmentDescriptionList
	{
		auto & attachment_infos = render_pass_create_info.GetAttachmentCreateInfos();
		auto attachments = AttachmentDescriptionList( attachment_infos.size(), allocator );

		for( size_t i = 0; i < attachment_infos.size(); i++ )
		{
			attachments[ i ].sType				= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			attachments[ i ].pNext				= nullptr;
			attachments[ i ].flags				= 0; // Can be VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT if attachment aliases another.
			attachments[ i ].format				= attachment_infos[ i ].GetFormat();
			attachments[ i ].samples			= attachment_infos[ i ].GetSamples();
			attachments[ i ].loadOp				= attachment_infos[ i ].GetLoadOp();
			attachments[ i ].storeOp			= attachment_infos[ i ].GetStoreOp();
			attachments[ i ].stencilLoadOp		= attachment_infos[ i ].GetStencilLoadOp();
			attachments[ i ].stencilStoreOp		= attachment_infos[ i ].GetStencilStoreOp();
			attachments[ i ].initialLayout		= attachment_infos[ i ].GetInitialLayout();
			attachments[ i ].finalLayout		= attachment_infos[ i ].GetFinalLayout();
		}
		return attachments;
	}( );

	struct SubpassInfos
	{
		AttachmentReferenceListList				input_attachment_references;
		AttachmentReferenceListList				color_attachment_references;
		AttachmentReferenceListList				resolve_attachment_references;
		AttachmentReferenceList					depth_stencil_resolve_attachment_reference;
		AttachmentReferenceList					depth_stencil_attachment_reference;
		PreserveAttachmentReferenceListList		preserve_attachment_references;
		SubpassDepthStencilResolveList			depth_stencil_resolve;
		SubpassDescriptionList					subpass;
	};
	auto subpass_infos = [ &render_pass_create_info, &allocator ]()
	{
		auto & subpass_create_infos = render_pass_create_info.GetSubpassCreateInfos();

		SubpassInfos ret;
		ret.input_attachment_references					= AttachmentReferenceListList( subpass_create_infos.size(), allocator );
		ret.color_attachment_references					= AttachmentReferenceListList( subpass_create_infos.size(), allocator );
		ret.resolve_attachment_references				= AttachmentReferenceListList( subpass_create_infos.size(), allocator );
		ret.depth_stencil_resolve_attachment_reference	= AttachmentReferenceList( subpass_create_infos.size(), allocator );
		ret.depth_stencil_attachment_reference			= AttachmentReferenceList( subpass_create_infos.size(), allocator );
		ret.preserve_attachment_references				= PreserveAttachmentReferenceListList( subpass_create_infos.size(), allocator );
		ret.depth_stencil_resolve						= SubpassDepthStencilResolveList( subpass_create_infos.size(), allocator );
		ret.subpass										= SubpassDescriptionList( subpass_create_infos.size(), allocator );

		for( size_t s = 0; s < subpass_create_infos.size(); s++ )
		{
			auto FillAttachment = [](
				VkAttachmentReference2									&	attachment_out,
				const RenderPassSubpassAttachmentReference				&	attachment_in
			)
			{
				attachment_out.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
				attachment_out.pNext		= nullptr;
				attachment_out.attachment	= attachment_in.GetAttachmentIndex();
				attachment_out.layout		= attachment_in.GetAttachmentLayout();
				attachment_out.aspectMask	= attachment_in.GetAttachmentAspectMask();
			};

			auto FillAttachments = [ FillAttachment, &allocator ](
				AttachmentReferenceList									&	attachments_out,
				std::span<const RenderPassSubpassAttachmentReference>		attachments_in
			) -> size_t
			{
				attachments_out = AttachmentReferenceList( attachments_in.size(), allocator );
				for( size_t i = 0; i < attachments_out.size(); i++ )
				{
					FillAttachment( attachments_out[ i ], attachments_in[ i ] );
				}
				return attachments_out.size();
			};

			auto & subpass_create_info = subpass_create_infos[ s ];

			FillAttachments(
				ret.input_attachment_references[ s ],
				subpass_create_info.GetInputAttachmentReferences()
			);

			FillAttachments(
				ret.color_attachment_references[ s ],
				subpass_create_info.GetColorAttachmentReferences()
			);

			FillAttachments(
				ret.resolve_attachment_references[ s ],
				subpass_create_info.GetResolveAttachmentReferences()
			);

			// THIS IS WASTED EFFORT, we're just adding a wrapper over render passes and only that.

			//auto has_depth_stencil = false;
			//if(  )
			//{
			//	has_depth_stencil = true;
			//	// TODO: depth stencil resolve attachment references
			//}

			// TODO: depth stencil attachment references
			// TODO: preserve attachment references
			// TODO: depth stencil resolves

			assert(
				ret.resolve_attachment_references[ s ].empty() ||
				ret.resolve_attachment_references[ s ].size() == ret.color_attachment_references[ s ].size()
			);
			ret.subpass[ s ].sType						= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			ret.subpass[ s ].pNext						= &ret.depth_stencil_resolve[ s ]; // TODO: Test if this subpass handles depth stencil attachment at all.
			ret.subpass[ s ].flags						= 0;
			ret.subpass[ s ].pipelineBindPoint			= subpass_create_info.GetPipelineBindPoint();
			ret.subpass[ s ].viewMask					= 0;	// Used mostly in VR, where subpass description is assigned a "view".
			ret.subpass[ s ].inputAttachmentCount		= uint32_t( ret.input_attachment_references[ s ].size() );
			ret.subpass[ s ].pInputAttachments			= ret.input_attachment_references[ s ].data();
			ret.subpass[ s ].colorAttachmentCount		= uint32_t( ret.color_attachment_references[ s ].size() );
			ret.subpass[ s ].pColorAttachments			= ret.color_attachment_references[ s ].data();
			ret.subpass[ s ].pResolveAttachments		= ret.resolve_attachment_references[ s ].data();
			ret.subpass[ s ].pDepthStencilAttachment	= &ret.depth_stencil_attachment_reference[ s ]; // TODO: Test if this subpass handles depth stencil attachments at all.
			ret.subpass[ s ].preserveAttachmentCount	= uint32_t( ret.preserve_attachment_references[ s ].size());
			ret.subpass[ s ].pPreserveAttachments		= ret.preserve_attachment_references[ s ].data();
		}

		return ret;
	}( );

	auto create_info = VkRenderPassCreateInfo2();
	create_info.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	create_info.pNext					= nullptr;
	create_info.flags					= 0;
	create_info.attachmentCount			= uint32_t( attachments.size() );
	create_info.pAttachments			= attachments.data();
	create_info.subpassCount			= uint32_t( subpass_infos.subpass.size() );
	create_info.pSubpasses				= subpass_infos.subpass.data();
	create_info.dependencyCount			= todo;
	create_info.pDependencies			= todo;
	create_info.correlatedViewMaskCount	= 0;	// Used mostly in VR, this tells which subpasses may be rendered simultaneously.
	create_info.pCorrelatedViewMasks	= nullptr;









	// Subpasses
	{
		// pNext can be:
		// - VkAttachmentReferenceStencilLayout		-> If layout only provides depth aspect, this can be used to separate stencil layout individually.
		auto depth_stencil_resolve_attachment = VkAttachmentReference2();
		depth_stencil_resolve_attachment.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		depth_stencil_resolve_attachment.pNext		= nullptr;
		depth_stencil_resolve_attachment.attachment	= todo; // Index to attachment this reference describes.
		depth_stencil_resolve_attachment.layout		= todo; // Layout during of the attachment during subpass.
		depth_stencil_resolve_attachment.aspectMask	= todo; // Mask which aspects of the attachment can be accessed during subpass.

		// This is useful if we need to resolve multisampled stencils after subpass finishes.
		auto depth_stencil_resolve = VkSubpassDescriptionDepthStencilResolve();
		depth_stencil_resolve.sType								= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
		depth_stencil_resolve.pNext								= nullptr;
		depth_stencil_resolve.depthResolveMode					= VK_RESOLVE_MODE_NONE;
		depth_stencil_resolve.stencilResolveMode				= todo;
		depth_stencil_resolve.pDepthStencilResolveAttachment	= &depth_stencil_resolve_attachment;

		auto color_attachment_reference = VkAttachmentReference2();
		color_attachment_reference.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		color_attachment_reference.pNext		= nullptr;
		color_attachment_reference.attachment	= ;
		color_attachment_reference.layout		= ;
		color_attachment_reference.aspectMask	= ;

		// pNext can be:
		// - VkFragmentShadingRateAttachmentInfoKHR		-> Allows multiple texels to be set from a single fragment, making "pixelated" results.
		// - VkMultisampledRenderToSingleSampledInfoEXT	-> Allows using multisampling on single sampled attachment on some platforms, if it is lazily allocated, too much work.
		// - VkRenderPassCreationControlEXT				-> Query if sub-passes are "merged" on some platforms.
		// - VkRenderPassSubpassFeedbackCreateInfoEXT	-> Used with VkRenderPassCreationControlEXT
		// - VkSubpassDescriptionDepthStencilResolve	-> Adds support for automatically resolving multisampled depth/stencil attachments in a subpass in a similar manner as for color attachments.
		auto subpass_description = VkSubpassDescription2();
		subpass_description.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass_description.pNext					= &depth_stencil_resolve;
		subpass_description.flags					= 0;
		subpass_description.pipelineBindPoint		= todo; // likely just going to be VK_PIPELINE_BIND_POINT_GRAPHICS
		subpass_description.viewMask				= 0;	// Used mostly in VR, where subpass description is assigned a "view".
		subpass_description.inputAttachmentCount	= todo;
		subpass_description.pInputAttachments		= todo;
		subpass_description.colorAttachmentCount	= todo;
		subpass_description.pColorAttachments		= todo;
		subpass_description.pResolveAttachments		= todo;
		subpass_description.pDepthStencilAttachment	= todo;
		subpass_description.preserveAttachmentCount	= todo;
		subpass_description.pPreserveAttachments	= todo;
	}

	// subpass dependencies
	{
		auto subpass_dependency = VkSubpassDependency2();
		subpass_dependency.sType					= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		subpass_dependency.pNext					= nullptr;
		subpass_dependency.srcSubpass				= todo;
		subpass_dependency.dstSubpass				= todo;
		subpass_dependency.srcStageMask				= todo;
		subpass_dependency.dstStageMask				= todo;
		subpass_dependency.srcAccessMask			= todo;
		subpass_dependency.dstAccessMask			= todo;
		subpass_dependency.dependencyFlags			= 0;
		subpass_dependency.viewOffset				= 0;	// used to determine which srcSubpass "view" dstSubpass depends on, Used mostly for VR.
	}

	auto create_info = VkRenderPassCreateInfo2();
	create_info.sType								= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	create_info.pNext								= nullptr;
	create_info.flags								= 0;
	create_info.attachmentCount						= uint32_t( attachments.size() );
	create_info.pAttachments						= attachments.data();
	create_info.subpassCount						= todo;
	create_info.pSubpasses							= todo;
	create_info.dependencyCount						= todo;
	create_info.pDependencies						= todo;
	create_info.correlatedViewMaskCount				= 0;	// Used mostly in VR, this tells which subpasses may be rendered simultaneously.
	create_info.pCorrelatedViewMasks				= nullptr;

	return render_pass_list(
		[ this, &render_pass_create_info, &create_info ]( RenderPassList & list ) -> RenderPassHandle
		{
			auto render_pass = VkRenderPass();
			vkCreateRenderPass2(
				instance.GetVulkanDevice(),
				&create_info,
				nullptr,
				&render_pass
			);

			if( render_pass == VK_NULL_HANDLE ) return {};

			auto result_it = list.emplace(
				render_pass_create_info.GetHash(),
				RenderPassManagerRenderPassEntry(
					render_pass,
					render_pass_create_info.GetHash()
				)
			);

			return RenderPassHandle(
				this,
				&result_it.first->second
			);
		}
	);
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::RenderPassManager::IncrementReferenceCount(
	RenderPassManagerRenderPassEntry * render_pass_entry
)
{
	render_pass_entry->reference_count(
		[]( size_t & count )
		{
			++count;
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::RenderPassManager::DecrementReferenceCount(
	RenderPassManagerRenderPassEntry * render_pass_entry
)
{
	bool destruct = false;

	render_pass_entry->reference_count(
		[ this, &destruct ]( size_t & count )
		{
			--count;
			if( count == 0 ) destruct = true;
		}
	);

	if( destruct ) DestroyRenderPass( render_pass_entry->GetHash() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::RenderPassManager::DestroyRenderPass(
	size_t render_pass_hash
)
{
	render_pass_list(
		[ this, render_pass_hash ]( RenderPassList & list )
		{
			auto it = list.find( render_pass_hash );
			if( it == list.end() ) return;

			vkDestroyRenderPass(
				vulkan_device,
				it->second.GetVulkanRenderPass(),
				nullptr
			);

			list.erase( it );
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::RenderPassManager::DestroyAllRenderPasss()
{
	render_pass_list(
		[ this ]( RenderPassList & list )
		{
			for( auto & p : list )
			{
				vkDestroyRenderPass(
					vulkan_device,
					p.second.GetVulkanRenderPass(),
					nullptr
				);
			}

			list.clear();
		}
	);
}
