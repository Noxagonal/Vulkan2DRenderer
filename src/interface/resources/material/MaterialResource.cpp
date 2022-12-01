
#include <core/SourceCommon.hpp>

#include <interface/resources/material/MaterialResource.hpp>
#include "MaterialResourceImpl.hpp"

#include <interface/resource_manager/ResourceManagerImpl.hpp>
#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::MaterialResource::MaterialResource(
	vk2d_internal::ResourceManagerImpl			&	resource_manager,
	uint32_t										loader_thread,
	ResourceBase								*	parent_resource,
	std::span<vk2d_internal::ShaderMemberInfo>		vertex_members,
	const MaterialCreateInfo					&	create_info
)
{
	impl = std::make_unique<vk2d_internal::MaterialResourceImpl>(
		*this,
		resource_manager,
		loader_thread,
		parent_resource,
		vertex_members,
		create_info
	);
	if( !impl || !impl->IsGood() ) {
		impl = nullptr;
		resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create material resource implementation!" );
		return;
	}

	resource_impl	= impl.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::MaterialResource::~MaterialResource()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::MaterialResource::GetStatus()
{
	return impl->GetStatus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::MaterialResource::WaitUntilLoaded(
	std::chrono::nanoseconds timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::MaterialResource::WaitUntilLoaded(
	std::chrono::steady_clock::time_point timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::MaterialResource::IsGood() const
{
	return !!impl && impl->IsGood();
}
