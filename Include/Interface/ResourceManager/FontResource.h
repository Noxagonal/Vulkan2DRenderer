#pragma once

#include "../../Core/Common.h"

#include "Resource.h"
#include "../../Types/Vector2.hpp"

#include <filesystem>

namespace vk2d {

class TextureResource;
class Mesh;
class FontResource;



VK2D_API vk2d::Mesh VK2D_APIENTRY GenerateTextMesh(
	vk2d::FontResource						*	font,
	vk2d::Vector2f								origin,
	std::string									text,
	float										kerning,
	vk2d::Vector2f								scale,
	bool										vertical,
	uint32_t									font_face,
	bool										wait_for_resource_load );



namespace _internal {

class ResourceManagerImpl;
class FontResourceImpl;

} // _internal



/// @brief		vk2d::FontResource is a resource that can be loaded with vk2d::ResourceManager.
///				It is needed for printing text in the realtime scene / window.
///				To use a font to print text in the window you'll first need to load the
///				font from a disk, then generate text mesh using the font resource, then
///				finally render the text mesh to a window or render target texture using
///				either Window::DrawMesh() or RenderTargetTexture::DrawMesh() functions.
class FontResource
	: public vk2d::Resource
{
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::FontResourceImpl;
	friend VK2D_API vk2d::Mesh								VK2D_APIENTRY						vk2d::GenerateTextMesh(
		vk2d::FontResource								*	font,
		vk2d::Vector2f										origin,
		std::string											text,
		float												kerning,
		vk2d::Vector2f										scale,
		bool												vertical,
		uint32_t											font_face,
		bool												wait_for_resource );

	/// @brief		This constructor is meant for internal use only. All resources are
	///				created from vk2d::ResourceManager only.
	/// @note		Multithreading: Any thread, selected managed by vk2d::ResourceManager.
	/// @param[in]	resource_manager
	///				Pointer to resource manager implementation object responsible for
	///				creating this resource.
	/// @param[in]	loader_thread_index
	///				Index to thread pool thread index. Tells which thread pool thread
	///				is responsible for creation and destruction of the internal data
	///				of this resource.
	/// @param[in]	parent_resource
	///				Pointer to a resource that owns this resource. Resources can have
	///				parent and child resources, this is just used to keep track of
	///				resources that are used by other resources and should not be removed
	///				unless the parent resources are removed.
	/// @param[in]	file_path
	///				File path where to load the font resource from.
	/// @param[in]	glyph_texel_size
	///				Glyph texel size tells the desired nominal height of a single letter
	///				in pixels/texels, however you should keep in mind that this value is
	///				only a guideline, the actual size will vary from font to font and a
	///				lot depends on what the font creator decided. Testing is the only way
	///				to know for sure.
	///				Since it's easy to scale the text at render time, this could be used
	///				as the quality of the letters, though for the perfect results you
	///				should use 1:1 texel mapping from the font to the window where you
	///				want the letters to be rendered to. ( that is to say, for perfect
	///				results use scale 1.0, 1.0 when rendering the text and use this
	///				parameter to control the size. )
	/// @param[in]	use_alpha
	///				Use alpha channel instead of color on edge of the letters when rendering.
	/// @param[in]	fallback_character
	///				Which character to use when desired character was not found in the
	///				font resource, eg. <B>&Delta;</B> might not appear on all fonts, so
	///				instead we use another in it's place.
	/// @param[in]	glyph_atlas_padding
	///				Font glyphs (Letters, numbers, markings) are internally stored in
	///				a texture atlas for efficiency, however if you render the text much
	///				smaller than the actual size of the glyphs in the texture atlas,
	///				mipmapping is often used to make text appear less grainy. However
	///				if mipmapping is used, then the glyphs may eventually start to mix
	///				together in the final render, to decrease the amount of this
	///				"UV bleeding" you can increase the gap between glyphs in the texture
	///				atlas here.
	VK2D_API																					FontResource(
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread_index,
		vk2d::Resource									*	parent_resource,
		const std::filesystem::path						&	file_path,
		uint32_t											glyph_texel_size,
		bool												use_alpha,
		uint32_t											fallback_character,
		uint32_t											glyph_atlas_padding );

public:
	VK2D_API																					~FontResource();

	/// @brief		Checks if the resource has been loaded, failed to load or is yet
	///				to be loaded / determined. This function will not wait but returns
	///				immediately with the result.
	/// @note		Multithreading: Any thread.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						GetStatus();

	/// @brief		Waits for the resource to load on the calling thread before
	///				continuing execution. For as long as the resource status is
	///				undetermined or timeout hasn't been met this function will block.
	///				As soon as the resource state becomes determined this function
	///				will return and code execution can continue.
	/// @note		Multithreading: Any thread.
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state
	///				at timeout it will return anyways and the result
	///				will tell that the resource is still undetermined.
	///				Default value is std::chrono::nanoseconds::max() which makes
	///				this function wait indefinitely.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	///				Resource status can only be undetermined if timeout was given.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max() );

	/// @brief		Waits for the resource to load on the calling thread before
	///				continuing execution. For as long as the resource status is
	///				undetermined or timeout hasn't been met this function will block.
	///				As soon as the resource state becomes determined this function
	///				will return and code execution can continue.
	/// @note		Multithreading: Any thread.
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state
	///				at timeout it will return anyways and the result
	///				will tell that the resource is still undetermined.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool											VK2D_APIENTRY						IsGood() const;

private:

	std::unique_ptr<vk2d::_internal::FontResourceImpl>		impl;
};



} // vk2d
