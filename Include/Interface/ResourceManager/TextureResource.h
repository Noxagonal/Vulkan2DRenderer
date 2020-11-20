#pragma once

#include "../../Core/Common.h"

#include "../../Types/Vector2.h"
#include "../../Types/Color.h"

#include "Resource.h"
#include "../Texture.h"

#include <memory>

namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class TextureResourceImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
} // _internal



class TextureResource :
	public vk2d::Texture,
	public vk2d::Resource
{
	friend class vk2d::_internal::TextureResourceImpl;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::ResourceThreadLoadTask;
	friend class vk2d::_internal::ResourceThreadUnloadTask;
	friend class vk2d::_internal::WindowImpl;

	// Array texture from files constructor.
	VK2D_API																				TextureResource(
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		vk2d::Resource										*	parent_resource,
		std::vector<std::filesystem::path>						file_paths_listing );

	// Texture from data constructor.
	VK2D_API																				TextureResource(
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		vk2d::Resource										*	parent_resource,
		vk2d::Vector2u											size,
		const std::vector<vk2d::Color8>						&	texels );

	// Array texture from data constructor.
	VK2D_API																				TextureResource(
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		vk2d::Resource										*	parent_resource,
		vk2d::Vector2u											size,
		const std::vector<std::vector<vk2d::Color8>*>		&	texels_listing );

public:
	VK2D_API																				~TextureResource();

	VK2D_API bool												VK2D_APIENTRY				IsLoaded();
	VK2D_API bool												VK2D_APIENTRY				WaitUntilLoaded();

	VK2D_API vk2d::Vector2u										VK2D_APIENTRY				GetSize() const;
	VK2D_API uint32_t											VK2D_APIENTRY				GetLayerCount() const;

public:
	VK2D_API bool												VK2D_APIENTRY				IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::TextureResourceImpl>		impl;
};



}
