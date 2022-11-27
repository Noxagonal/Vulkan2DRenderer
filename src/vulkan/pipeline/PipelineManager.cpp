
#include "PipelineManager.hpp"

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PipelineManager::PipelineManager(
	vk2d_internal::InstanceImpl		&	instance,
	Device							&	vulkan_device
) :
	instance( instance ),
	vulkan_device( vulkan_device )
{
	if( !CreateGraphicsPipelineCache() ) return;
	if( !CreateComputePipelineCache() ) return;

	is_good = true;
}

vk2d::vulkan::PipelineManager::~PipelineManager()
{
	DestroyPipelines();
	DestroyComputePipelineCache();
	DestroyGraphicsPipelineCache();
}

VkPipeline vk2d::vulkan::PipelineManager::FindPipeline(
	const vulkan::GraphicsPipelineInfo & graphics_pipeline_info
)
{
	return FindPipeline( graphics_pipeline_info.GetHash() );
}

VkPipeline vk2d::vulkan::PipelineManager::FindPipeline(
	const vulkan::ComputePipelineInfo & compute_pipeline_info
)
{
	return FindPipeline( compute_pipeline_info.GetHash() );
}

VkPipeline vk2d::vulkan::PipelineManager::FindPipeline(
	size_t hash
)
{
	auto it = pipeline_list.find( hash );
	if( it == pipeline_list.end() ) return {};
	return it->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipeline vk2d::vulkan::PipelineManager::CreateGraphicsPipeline(
	const vulkan::GraphicsPipelineInfo & graphics_pipeline_info
)
{
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos {};
	shader_stage_create_infos[ 0 ].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 0 ].pNext				= nullptr;
	shader_stage_create_infos[ 0 ].flags				= 0;
	shader_stage_create_infos[ 0 ].stage				= VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_create_infos[ 0 ].module				= graphics_pipeline_info.GetGraphicsShaderList_DEPRICATED().GetVertexShader();
	shader_stage_create_infos[ 0 ].pName				= "main";
	shader_stage_create_infos[ 0 ].pSpecializationInfo	= nullptr;

	shader_stage_create_infos[ 1 ].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 1 ].pNext				= nullptr;
	shader_stage_create_infos[ 1 ].flags				= 0;
	shader_stage_create_infos[ 1 ].stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_create_infos[ 1 ].module				= graphics_pipeline_info.GetGraphicsShaderList_DEPRICATED().GetFragmentShader();
	shader_stage_create_infos[ 1 ].pName				= "main";
	shader_stage_create_infos[ 1 ].pSpecializationInfo	= nullptr;

	// Make sure this matches Vertex in RenderPrimitives.h
	std::array<VkVertexInputBindingDescription, 0> vertex_input_binding_descriptions {};
	//	vertex_input_binding_descriptions[ 0 ].binding		= 0;
	//	vertex_input_binding_descriptions[ 0 ].stride		= sizeof( Vertex );
	//	vertex_input_binding_descriptions[ 0 ].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 0> vertex_input_attribute_descriptions {};
	//	vertex_input_attribute_descriptions[ 0 ].location	= 0;
	//	vertex_input_attribute_descriptions[ 0 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 0 ].format		= VK_FORMAT_R32G32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 0 ].offset		= offsetof( Vertex, vertex_coords );
	//
	//	vertex_input_attribute_descriptions[ 1 ].location	= 1;
	//	vertex_input_attribute_descriptions[ 1 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 1 ].format		= VK_FORMAT_R32G32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 1 ].offset		= offsetof( Vertex, uv_coords );
	//
	//	vertex_input_attribute_descriptions[ 2 ].location	= 2;
	//	vertex_input_attribute_descriptions[ 2 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 2 ].format		= VK_FORMAT_R32G32B32A32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 2 ].offset		= offsetof( Vertex, color );
	//
	//	vertex_input_attribute_descriptions[ 3 ].location	= 3;
	//	vertex_input_attribute_descriptions[ 3 ].binding	= 0;
	//	vertex_input_attribute_descriptions[ 3 ].format		= VK_FORMAT_R32_SFLOAT;
	//	vertex_input_attribute_descriptions[ 3 ].offset		= offsetof( Vertex, point_size );

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
	input_assembly_state_create_info.topology					= graphics_pipeline_info.GetPrimitiveTopology();
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
	rasterization_state_create_info.polygonMode					= graphics_pipeline_info.GetPolygonMode();
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
	multisample_state_create_info.rasterizationSamples		= VkSampleCountFlagBits( graphics_pipeline_info.GetSampleCount() );
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
	color_blend_attachment_states[ 0 ].blendEnable			= graphics_pipeline_info.GetEnableBlending();
	color_blend_attachment_states[ 0 ].srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].colorBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	color_blend_attachment_states[ 0 ].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;
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
	pipeline_create_info.stageCount				= uint32_t( shader_stage_create_infos.size() );
	pipeline_create_info.pStages				= shader_stage_create_infos.data();
	pipeline_create_info.pVertexInputState		= &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState	= &input_assembly_state_create_info;
	pipeline_create_info.pTessellationState		= nullptr;
	pipeline_create_info.pViewportState			= &viewport_state_create_info;
	pipeline_create_info.pRasterizationState	= &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState		= &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState		= &depth_stencil_state_create_info;
	pipeline_create_info.pColorBlendState		= &color_blend_state_create_info;
	pipeline_create_info.pDynamicState			= &dynamic_state_create_info;
	pipeline_create_info.layout					= graphics_pipeline_info.GetVulkanPipelineLayout();
	pipeline_create_info.renderPass				= graphics_pipeline_info.GetVulkanRenderPass();
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	VkPipeline pipeline {};
	auto result = vkCreateGraphicsPipelines(
		vulkan_device,
		GetGraphicsPipelineCache(),
		1,
		&pipeline_create_info,
		nullptr,
		&pipeline
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan graphics pipeline!" );
		return {};
	}

	pipeline_list.emplace( graphics_pipeline_info.GetHash(), pipeline);
	return pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipeline vk2d::vulkan::PipelineManager::CreateComputePipeline(
	const vulkan::ComputePipelineInfo & compute_pipeline_info
)
{
	VkPipelineShaderStageCreateInfo stage_create_info {};
	stage_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_create_info.pNext						= nullptr;
	stage_create_info.flags						= 0;
	stage_create_info.stage						= VK_SHADER_STAGE_COMPUTE_BIT;
	stage_create_info.module					= compute_pipeline_info.GetVulkanComputeShader();
	stage_create_info.pName						= "main";
	stage_create_info.pSpecializationInfo		= nullptr;

	VkComputePipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType					= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_create_info.pNext					= nullptr;
	pipeline_create_info.flags					= 0;
	pipeline_create_info.stage					= stage_create_info;
	pipeline_create_info.layout					= compute_pipeline_info.GetVulkanPipelineLayout();
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= 0;

	VkPipeline pipeline {};
	auto result = vkCreateComputePipelines(
		vulkan_device,
		GetComputePipelineCache(),
		1,
		&pipeline_create_info,
		nullptr,
		&pipeline
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan compute pipeline!" );
		return {};
	}

	pipeline_list.emplace( compute_pipeline_info.GetHash(), pipeline);
	return pipeline;
}

void vk2d::vulkan::PipelineManager::DestroyPipeline(
	VkPipeline pipeline
)
{
	for( auto it = pipeline_list.begin(); it != pipeline_list.end(); )
	{
		if( it->second == pipeline )
		{
			pipeline_list.erase( it );

			vkDestroyPipeline(
				vulkan_device,
				pipeline,
				nullptr
			);

			return;
		}

		++it;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipelineCache vk2d::vulkan::PipelineManager::GetGraphicsPipelineCache() const
{
	return vulkan_graphics_pipeline_cache;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipelineCache vk2d::vulkan::PipelineManager::GetComputePipelineCache() const
{
	return vulkan_compute_pipeline_cache;
}

bool vk2d::vulkan::PipelineManager::IsGood()
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vulkan::PipelineManager::CreateGraphicsPipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info {};
	pipeline_cache_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.pNext				= nullptr;
	pipeline_cache_create_info.flags				= 0;
	pipeline_cache_create_info.initialDataSize		= 0;
	pipeline_cache_create_info.pInitialData			= nullptr;

	auto result = vkCreatePipelineCache(
		vulkan_device,
		&pipeline_cache_create_info,
		nullptr,
		&vulkan_graphics_pipeline_cache
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan graphics pipeline cache!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vulkan::PipelineManager::CreateComputePipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info {};
	pipeline_cache_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.pNext				= nullptr;
	pipeline_cache_create_info.flags				= 0;
	pipeline_cache_create_info.initialDataSize		= 0;
	pipeline_cache_create_info.pInitialData			= nullptr;

	auto result = vkCreatePipelineCache(
		vulkan_device,
		&pipeline_cache_create_info,
		nullptr,
		&vulkan_compute_pipeline_cache
	);
	if( result != VK_SUCCESS ) {
		instance.Report( result, "Internal error: Cannot create Vulkan compute pipeline cache!" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::PipelineManager::DestroyGraphicsPipelineCache()
{
	vkDestroyPipelineCache(
		vulkan_device,
		vulkan_graphics_pipeline_cache,
		nullptr
	);
	vulkan_graphics_pipeline_cache = {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::PipelineManager::DestroyComputePipelineCache()
{
	vkDestroyPipelineCache(
		vulkan_device,
		vulkan_compute_pipeline_cache,
		nullptr
	);
	vulkan_compute_pipeline_cache = {};
}

void vk2d::vulkan::PipelineManager::DestroyPipelines()
{
	for( auto & o : pipeline_list )
	{
		vkDestroyPipeline(
			vulkan_device,
			o.second,
			nullptr
		);
	}
	pipeline_list.clear();
}
