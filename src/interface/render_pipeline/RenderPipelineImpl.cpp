
#include <core/SourceCommon.hpp>
#include "RenderPipelineImpl.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::RenderPipelineImpl::RenderPipelineImpl(
	RenderPipeline					&	my_interface,
	InstanceImpl					&	instance,
	const RenderPipelineCreateInfo	&	create_info
) :
	my_interface( my_interface ),
	instance( instance ),
	create_info( create_info )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::RenderPipelineImpl::~RenderPipelineImpl()
{}
