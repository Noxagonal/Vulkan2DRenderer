#pragma once

#include "Core/Common.h"

#include "Types/Vector2.hpp"



namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
class TextureImpl;
} // _internal



// Public texture interface

/// @brief		This is a public texture interface and it's used to get common information from
///				an object that can be used when drawing, for example vk2d::TextureResource.
class Texture
{
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

public:
	/// @brief		Tells if the texture data is ready to be used.
	/// @note		Multithreading: Main thread only.
	/// @return		true if texture data is ready to be used, false otherwise.
	VK2D_API bool										VK2D_APIENTRY				IsTextureDataReady();

	/// @brief		Gets the texel size of the texture.
	/// @note		Multithreading: Main thread only.
	/// @return		Size of the texture in texels.
	VK2D_API vk2d::Vector2u								VK2D_APIENTRY				GetSize() const;

	/// @brief		Gets the texture layer count. VK2D Textures are multi-layer, meaning that
	///				a single texture can be considered more like an array of textures rather
	///				than a singular texture. Different combinations of those layers can be
	///				used when rendering, however texture layers must all have the same texel
	///				size.
	/// @see		vk2d::RenderTargetTexture::DrawTriangleList(), vk2d::Window::DrawTriangleList().
	/// @note		Multithreading: Any thread.
	/// @return		Number of layers the texture has.
	VK2D_API uint32_t									VK2D_APIENTRY				GetLayerCount() const;

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool										VK2D_APIENTRY				IsGood() const;

protected:
	vk2d::_internal::TextureImpl					*	texture_impl				= {};
};



} // vk2d
