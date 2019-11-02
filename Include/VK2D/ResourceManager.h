#pragma once

#include "Common.h"

#include <memory>
#include <filesystem>

namespace vk2d {

class ThreadPool;
class TextureResource;

namespace _internal {
class RendererImpl;
class ResourceManagerImpl;
}

class ResourceManager {
	friend class _internal::RendererImpl;

public:
	VK2D_API											ResourceManager(
		_internal::RendererImpl						*	parent_renderer
	);
	VK2D_API											~ResourceManager();

	VK2D_API TextureResource		*	VK2D_APIENTRY	LoadTextureResource(
		std::filesystem::path							file_path );

private:
	std::unique_ptr<_internal::ResourceManagerImpl>		impl		= {};

	bool												is_good		= {};
};



}

