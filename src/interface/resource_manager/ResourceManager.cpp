
#include <core/SourceCommon.hpp>

#include <interface/resource_manager/ResourceManager.hpp>
#include "ResourceManagerImpl.hpp"

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceManager::ResourceManager(
	vk2d_internal::InstanceImpl		&	instance
)
{
	impl = std::make_unique<vk2d_internal::ResourceManagerImpl>(
		*this,
		instance
	);
	if( !impl || !impl->IsGood() ) {
		impl	= nullptr;
		instance.Report( ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create resource manager implementation!" );
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceManager::~ResourceManager()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource * vk2d::ResourceManager::DoCreateTextureResource(
	glm::uvec2						size,
	const std::vector<Color8>	&	texels
)
{
	return impl->DoCreateTextureResource(
		size,
		texels,
		nullptr
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource * vk2d::ResourceManager::DoLoadTextureResource(
	const std::filesystem::path		&	file_path
)
{
	return impl->DoLoadTextureResource(
		file_path,
		nullptr
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource * vk2d::ResourceManager::DoCreateArrayTextureResource(
	glm::uvec2											size,
	const std::vector<const std::vector<Color8>*>	&	texels_listing
)
{
	return impl->DoCreateArrayTextureResource(
		size,
		texels_listing,
		nullptr
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource * vk2d::ResourceManager::DoLoadArrayTextureResource(
	const std::vector<std::filesystem::path>		&	file_path_listing
)
{
	return impl->DoLoadArrayTextureResource(
		file_path_listing,
		nullptr
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::FontResource * vk2d::ResourceManager::DoLoadFontResource(
	const std::filesystem::path		&	file_path,
	uint32_t							glyph_texel_size,
	bool								use_alpha,
	uint32_t							fallback_character,
	uint32_t							glyph_atlas_padding
)
{
	return impl->DoLoadFontResource(
		file_path,
		nullptr,
		glyph_texel_size,
		use_alpha,
		fallback_character,
		glyph_atlas_padding
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::MaterialResource * vk2d::ResourceManager::DoCreateMaterialResource(
	std::span<vk2d_internal::ShaderMemberInfo>		vertex_members,
	const MaterialCreateInfo					&	create_info
)
{
	return impl->DoCreateMaterialResource(
		nullptr,
		vertex_members,
		create_info
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::ResourceManager::DestroyResource(
	ResourceBase		*	resource
)
{
	impl->DestroyResource( resource );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::ResourceManager::IsGood() const
{
	return !!impl;
}
