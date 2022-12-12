
#include <core/SourceCommon.hpp>
#include "RenderPipelineImpl.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::RenderPipeline::RenderPipeline(
	vk2d_internal::InstanceImpl		&	instance,
	const RenderPipelineCreateInfo	&	render_pipeline_create_info
)
{
	impl = std::make_unique<vk2d_internal::RenderPipelineImpl>(
		*this,
		instance,
		render_pipeline_create_info
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::RenderPipeline::~RenderPipeline()
{}
