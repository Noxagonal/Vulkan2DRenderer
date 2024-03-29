#pragma once

#include "core/Common.h"



namespace vk2d {
namespace vk2d_internal {
class WindowImpl;
class RenderTargetTextureImpl;
class TextureImpl;
} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is a public texture interface.
///
///				It is used to get common information from an object that can be used when drawing, for example
///				TextureResource.
class Texture
{
	friend class vk2d_internal::WindowImpl;
	friend class vk2d_internal::RenderTargetTextureImpl;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tells if the texture data is ready to be used.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if texture data is ready to be used, false otherwise.
	VK2D_API bool									IsTextureDataReady();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the texel size of the texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Size of the texture in texels.
	VK2D_API glm::uvec2								GetSize() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the texture layer count.
	///
	///				VK2D Textures are multi-layer, meaning that a single texture can be considered more like an array of textures
	///				rather than a singular texture. Different combinations of those layers can be used when rendering, however
	///				texture layers must all have the same texel size.
	/// 
	/// @see		RenderTargetTexture::DrawTriangleList()
	/// @see		Window::DrawTriangleList()
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Number of layers the texture has.
	VK2D_API uint32_t								GetLayerCount() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool									IsGood() const;

protected:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::TextureImpl					*	texture_impl				= {};
};



} // vk2d
