#pragma once

#include "../Core/Common.h"
#include "Resource.h"
#include "../../Include/Interface/RenderPrimitives.h"

#include <memory>

namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class TextureResourceImpl;
class LoadTask;
class UnloadTask;
} // _internal



class TextureResource :
	public Resource
{
	friend class vk2d::_internal::TextureResourceImpl;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::LoadTask;
	friend class vk2d::_internal::UnloadTask;
	friend class vk2d::_internal::WindowImpl;

	// Texture from file constructor.
	VK2D_API																				TextureResource(
		vk2d::Resource										*	parent_resource,
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		std::filesystem::path									file_path );

	// Array texture from files constructor.
	VK2D_API																				TextureResource(
		vk2d::Resource										*	parent_resource,
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		std::vector<std::filesystem::path>						file_paths_listing );

	// Texture from data constructor.
	VK2D_API																				TextureResource(
		vk2d::Resource										*	parent_resource,
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		vk2d::Vector2u											size,
		const std::vector<vk2d::Color8>						&	texels );

	// Array texture from data constructor.
	VK2D_API																				TextureResource(
		vk2d::Resource										*	parent_resource,
		vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent,
		uint32_t												loader_thread,
		vk2d::Vector2u											size,
		const std::vector<std::vector<vk2d::Color8>*>		&	texels_listing );

public:
	VK2D_API																				~TextureResource();

	VK2D_API bool												VK2D_APIENTRY				IsLoaded();
	VK2D_API bool												VK2D_APIENTRY				WaitUntilLoaded();

	VK2D_API uint32_t											VK2D_APIENTRY				GetLayerCount();

protected:
	VK2D_API virtual bool										VK2D_APIENTRY				MTLoad(
		vk2d::_internal::ThreadPrivateResource				*	thread_resource );

	VK2D_API virtual void										VK2D_APIENTRY				MTUnload(
		vk2d::_internal::ThreadPrivateResource				*	thread_resource );

private:
	std::unique_ptr<vk2d::_internal::TextureResourceImpl>		impl;
};



}
