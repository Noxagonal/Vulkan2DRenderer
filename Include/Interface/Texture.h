#pragma once

#include "../Core/Common.h"
#include "../../Include/Interface/RenderPrimitives.h"



namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
class TextureImpl;
} // _internal

class Instance;



// Public texture interface
class Texture
{
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

protected:
	VK2D_API																		Texture();

public:
	VK2D_API																		~Texture();

	VK2D_API virtual bool								VK2D_APIENTRY				WaitUntilLoaded()				= 0;
	VK2D_API virtual bool								VK2D_APIENTRY				IsLoaded()						= 0;

	VK2D_API virtual vk2d::Vector2u						VK2D_APIENTRY				GetSize() const					= 0;
	VK2D_API virtual uint32_t							VK2D_APIENTRY				GetLayerCount() const			= 0;

	VK2D_API virtual bool								VK2D_APIENTRY				IsGood() const					= 0;

protected:
	vk2d::_internal::TextureImpl					*	texture_impl				= {};
};



} // vk2d
