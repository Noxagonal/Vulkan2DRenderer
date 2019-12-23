
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/ResourceManager.h"
#include "../Header/Impl/ResourceManagerImpl.h"

#include <memory>



VK2D_API vk2d::ResourceManager::ResourceManager(
	vk2d::_internal::RendererImpl		*	parent_renderer
)
{
	impl = std::make_unique<vk2d::_internal::ResourceManagerImpl>( parent_renderer );
	if( !impl )	return;
	if( !impl->IsGood() ) {
		impl	= nullptr;
		return;
	}
	is_good		= true;
}

VK2D_API vk2d::ResourceManager::~ResourceManager()
{}

VK2D_API vk2d::TextureResource *VK2D_APIENTRY vk2d::ResourceManager::CreateTextureResource(
	vk2d::Vector2u						size,
	const std::vector<vk2d::Color8>	&	texels
)
{
	if( impl ) return impl->CreateTextureResource(
		size,
		texels
	);
	return nullptr;
}

VK2D_API vk2d::TextureResource * VK2D_APIENTRY vk2d::ResourceManager::LoadTextureResource(
	std::filesystem::path		file_path
)
{
	if( impl ) return impl->LoadTextureResource( file_path );
	return nullptr;
}

VK2D_API void VK2D_APIENTRY vk2d::ResourceManager::DestroyResource(
	vk2d::Resource		*	resource
)
{
	if( impl ) impl->DestroyResource( resource );
}

VK2D_API bool VK2D_APIENTRY vk2d::ResourceManager::IsGood()
{
	return is_good;
}
