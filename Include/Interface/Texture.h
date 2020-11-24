#pragma once

#include "../Core/Common.h"

#include "../Types/Vector2.hpp"



namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
class TextureImpl;
} // _internal



// Public texture interface
class Texture
{
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

public:
	VK2D_API bool										VK2D_APIENTRY				IsTextureDataReady();

	VK2D_API vk2d::Vector2u								VK2D_APIENTRY				GetSize() const;
	VK2D_API uint32_t									VK2D_APIENTRY				GetLayerCount() const;

	VK2D_API bool										VK2D_APIENTRY				IsGood() const;

protected:
	vk2d::_internal::TextureImpl					*	texture_impl				= {};
};



} // vk2d
