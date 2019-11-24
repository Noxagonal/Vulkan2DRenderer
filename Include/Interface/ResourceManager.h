#pragma once

#include "../Core/Common.h"
#include "../Interface/TextureResource.h"

#include <memory>
#include <filesystem>

namespace vk2d {

class ThreadPool;
class Resource;

namespace _internal {
class RendererImpl;
class ResourceManagerImpl;
}

class ResourceManager {
	friend class _internal::RendererImpl;

public:
	VK2D_API																ResourceManager(
		_internal::RendererImpl											*	parent_renderer
	);

	VK2D_API																~ResourceManager();

	VK2D_API TextureResource				*	VK2D_APIENTRY				CreateTextureResource(
		uint32_t								extent_x,
		uint32_t								extent_y,
		const std::vector<vk2d::Texel>		&	texels );

	VK2D_API TextureResource				*	VK2D_APIENTRY				LoadTextureResource(
		std::filesystem::path												file_path );

	VK2D_API void								VK2D_APIENTRY				DestroyResource(
		Resource														*	resource );

	VK2D_API bool								VK2D_APIENTRY				IsGood();

private:
	std::unique_ptr<_internal::ResourceManagerImpl>							impl		= {};

	bool																	is_good		= {};
};



}

