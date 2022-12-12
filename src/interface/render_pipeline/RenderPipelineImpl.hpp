#pragma once

#include <core/SourceCommon.hpp>
#include <interface/render_pipeline/RenderPipeline.hpp>



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPipelineImpl
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RenderPipelineImpl(
		RenderPipeline										&	my_interface,
		InstanceImpl										&	instance,
		const RenderPipelineCreateInfo						&	create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~RenderPipelineImpl();

private:

	RenderPipeline											&	my_interface;
	InstanceImpl											&	instance;
	RenderPipelineCreateInfo									create_info;
};



} // vk2d_internal
} // vk2d