#pragma once

#include "../Core/Common.h"
#include "RenderPrimitives.h"

#include <memory>
#include <filesystem>

namespace vk2d {

class ThreadPool;
class Resource;
class TextureResource;
class FontResource;

namespace _internal {
class RendererImpl;
class ResourceManagerImpl;
}

class ResourceManager {
	friend class vk2d::_internal::RendererImpl;

public:
	VK2D_API																				ResourceManager(
		vk2d::_internal::RendererImpl													*	parent_renderer
	);

	VK2D_API																				~ResourceManager();

	VK2D_API TextureResource								*	VK2D_APIENTRY				CreateTextureResource(
		vk2d::Vector2u											size,
		const std::vector<vk2d::Color8>						&	texels );

	VK2D_API vk2d::TextureResource							*	VK2D_APIENTRY				LoadTextureResource(
		const std::filesystem::path							&	file_path );

	VK2D_API TextureResource								*	VK2D_APIENTRY				CreateArrayTextureResource(
		vk2d::Vector2u											size,
		const std::vector<std::vector<vk2d::Color8>*>		&	texels_listing );

	VK2D_API vk2d::TextureResource							*	VK2D_APIENTRY				LoadArrayTextureResource(
		const std::vector<std::filesystem::path>			&	file_path_listing );

	VK2D_API vk2d::FontResource								*	VK2D_APIENTRY				LoadFontResource(
		const std::filesystem::path							&	file_path );

	VK2D_API void												VK2D_APIENTRY				DestroyResource(
		vk2d::Resource																	*	resource );

	VK2D_API bool												VK2D_APIENTRY				IsGood();

private:
	std::unique_ptr<_internal::ResourceManagerImpl>				impl						= {};

	bool														is_good						= {};
};



}

