#pragma once

#include "Common.h"

#include <memory>

namespace vk2d {

namespace _internal {
class RendererImpl;
class ResourceManagerImpl;
}

class TextureResource;

class ResourceManager {
	friend class _internal::RendererImpl;

public:
	ResourceManager(
		_internal::RendererImpl			*	parent_renderer
	);
	~ResourceManager();

private:
	std::unique_ptr<_internal::ResourceManagerImpl>		impl		= {};

	bool												is_good		= {};
};



}

