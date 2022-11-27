
#include <core/SourceCommon.hpp>

#include <interface/resources/TextureResource.hpp>
#include "TextureResourceImpl.hpp"

#include <interface/resource_manager/ResourceManagerImpl.hpp>

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource::TextureResource(
	vk2d_internal::ResourceManagerImpl			&	resource_manager,
	uint32_t										loader_thread,
	ResourceBase								*	parent_resource,
	const std::vector<std::filesystem::path>	&	file_paths_listing
)
{
	impl = std::make_unique<vk2d_internal::TextureResourceImpl>(
		*this,
		resource_manager,
		loader_thread,
		nullptr,
		file_paths_listing
	);
	if( !impl || !impl->IsGood() ) {
		impl = nullptr;
		resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}

	resource_impl	= impl.get();
	texture_impl	= impl.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource::TextureResource(
	vk2d_internal::ResourceManagerImpl				&	resource_manager,
	uint32_t											loader_thread,
	ResourceBase									*	parent_resource,
	glm::uvec2											size,
	const std::vector<const std::vector<Color8>*>	&	texels_listing
)
{
	impl = std::make_unique<vk2d_internal::TextureResourceImpl>(
		*this,
		resource_manager,
		loader_thread,
		nullptr,
		size,
		texels_listing
	);
	if( !impl || !impl->IsGood() ) {
		impl = nullptr;
		resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture resource implementation!" );
		return;
	}

	resource_impl	= impl.get();
	texture_impl	= impl.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource::~TextureResource()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::TextureResource::GetStatus()
{
	return impl->GetStatus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::TextureResource::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::TextureResource::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API glm::uvec2 vk2d::TextureResource::GetSize() const
{
	return impl->GetSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API uint32_t vk2d::TextureResource::GetLayerCount() const
{
	return impl->GetLayerCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::TextureResource::IsGood() const
{
	return !!impl && impl->IsGood();
}
