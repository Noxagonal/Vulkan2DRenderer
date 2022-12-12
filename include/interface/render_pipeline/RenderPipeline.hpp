#pragma once

#include <core/Common.hpp>

#include "RenderPipelineCreateInfo.hpp"

#include <memory>



namespace vk2d {
namespace vk2d_internal {

class InstanceImpl;
class RenderPipelineImpl;

} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Render pipeline allows customizing every step of the rendering process.
///
///				Render pipeline allows user to implement post-processing, lighting passes, and special effects.
///
///				Render pipeline includes an arbitary number of GBuffers in where intermediate and final renders are stored, as
///				well as render passes which describe how rendering is done and how GBuffers are combined to create the final
///				render.
class RenderPipeline
{
	friend class vk2d_internal::InstanceImpl;
	friend class vk2d_internal::RenderPipelineImpl;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API RenderPipeline(
		vk2d_internal::InstanceImpl				&	instance,
		const RenderPipelineCreateInfo			&	render_pipeline_create_info
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API ~RenderPipeline();

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::RenderPipelineImpl>		impl;
};



} // vk2d
