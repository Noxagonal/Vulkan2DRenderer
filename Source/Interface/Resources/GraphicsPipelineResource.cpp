
#include "Core/SourceCommon.h"

#include "Interface/Resources/GraphicsPipelineResource.h"
#include "Interface/Resources/GraphicsPipelineResourceImpl.h"

#include "Interface/InstanceImpl.h"
#include "Interface/Resources/ResourceManagerImpl.h"







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Interface.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







VK2D_API vk2d::GraphicsPipelineResource::GraphicsPipelineResource(
	vk2d::_internal::ResourceManagerImpl	*	resource_manager,
	uint32_t									loader_thread_index,
	vk2d::ResourceBase						*	parent_resource,
	const std::filesystem::path				&	file_path
)
{
	impl = std::make_unique<vk2d::_internal::GraphicsPipelineResourceImpl>(
		this,
		resource_manager,
		loader_thread_index,
		parent_resource,
		file_path
	);
	if( !impl || impl->IsGood() ) {
		impl = nullptr;
		resource_manager->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create graphics pipeline resource implementation!" );
		return;
	}

	resource_impl = impl.get();
}

VK2D_API vk2d::GraphicsPipelineResource::~GraphicsPipelineResource()
{}







////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////







vk2d::_internal::GraphicsPipelineResourceImpl::GraphicsPipelineResourceImpl(
	vk2d::GraphicsPipelineResource			*	my_interface,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager,
	uint32_t									loader_thread,
	vk2d::ResourceBase						*	parent_resource,
	const std::filesystem::path				&	file_path
) :
	vk2d::_internal::ResourceImplBase( my_interface, loader_thread, resource_manager, parent_resource, { file_path } )
{
	is_good = true;
}

vk2d::_internal::GraphicsPipelineResourceImpl::~GraphicsPipelineResourceImpl()
{}

vk2d::ResourceStatus vk2d::_internal::GraphicsPipelineResourceImpl::GetStatus()
{
	return vk2d::ResourceStatus::FAILED_TO_LOAD;
}

vk2d::ResourceStatus vk2d::_internal::GraphicsPipelineResourceImpl::WaitUntilLoaded(
	std::chrono::nanoseconds timeout
)
{
	return vk2d::ResourceStatus::FAILED_TO_LOAD;
}

vk2d::ResourceStatus vk2d::_internal::GraphicsPipelineResourceImpl::WaitUntilLoaded(
	std::chrono::steady_clock::time_point timeout
)
{
	return vk2d::ResourceStatus::FAILED_TO_LOAD;
}



bool vk2d::_internal::GraphicsPipelineResourceImpl::MTLoad(
	vk2d::_internal::ThreadPrivateResource * thread_resource
)
{
	return false;
}

bool vk2d::_internal::GraphicsPipelineResourceImpl::MTLoadMore(
	vk2d::_internal::ThreadPrivateResource * thread_resource
)
{
	return false;
}

void vk2d::_internal::GraphicsPipelineResourceImpl::MTUnload(
	vk2d::_internal::ThreadPrivateResource * thread_resource
)
{}



bool vk2d::_internal::GraphicsPipelineResourceImpl::IsGood() const
{
	return is_good;
}
