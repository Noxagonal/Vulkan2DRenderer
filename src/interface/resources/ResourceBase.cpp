
#include <core/SourceCommon.h>

#include <interface/resources/ResourceBase.h>
#include "ResourceImplBase.h"

#include "ResourceManagerImpl.h"

#include <interface/InstanceImpl.h>



VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::GetStatus()
{
	assert( resource_impl );
	return resource_impl->GetStatus();
}

VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceStatus vk2d::ResourceBase::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	assert( resource_impl );
	return resource_impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceBase * vk2d::ResourceBase::GetParentResource()
{
	assert( resource_impl );
	return resource_impl->GetParentResource();
}

VK2D_API bool vk2d::ResourceBase::IsFromFile() const
{
	assert( resource_impl );
	return resource_impl->IsFromFile();
}

VK2D_API const std::vector<std::filesystem::path> & vk2d::ResourceBase::GetFilePaths() const
{
	assert( resource_impl );
	return resource_impl->GetFilePaths();
}

VK2D_API bool vk2d::ResourceBase::IsGood() const
{
	if( !resource_impl ) return false;
	return resource_impl->IsGood();
}
