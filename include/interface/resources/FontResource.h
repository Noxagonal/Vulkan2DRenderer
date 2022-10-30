#pragma once

#include "core/Common.h"

#include "interface/resources/ResourceBase.h"
#include <types/MeshGenerators.hpp>

#include <filesystem>

namespace vk2d {

class TextureResource;
class FontResource;
class MeshBase;



namespace vk2d_internal {

class ResourceManagerImpl;
class FontResourceImpl;

} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		FontResource is a resource that contains a font which is used to draw text.
///
///				FontResource can be loaded with ResourceManager.
///
///				Font is needed for printing text in the realtime scene / window. To use a font to print text in the window
///				you'll first need to load the font from a disk, then generate text mesh using the font resource, then finally
///				render the text mesh to a window or render target texture using either Window::DrawMesh() or
///				RenderTargetTexture::DrawMesh() functions.
class FontResource
	: public ResourceBase
{
	friend class vk2d_internal::ResourceManagerImpl;
	friend class vk2d_internal::FontResourceImpl;

	friend VK2D_API void vk2d::vk2d_internal::GenerateTextMeshImpl(
		MeshBase		&	mesh,
		FontResource	*	font,
		glm::vec2			origin,
		std::string			text,
		float				kerning,
		glm::vec2			scale,
		bool				vertical,
		uint32_t			font_face,
		bool				wait_for_resource_load
	);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		This constructor is meant for internal use only.
	///
	/// @note		All resources are created from ResourceManager only.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	resource_manager
	///				Pointer to resource manager implementation object responsible for creating this resource.
	/// 
	/// @param[in]	loader_thread_index
	///				Index to thread pool thread index. Tells which thread pool thread is responsible for creation and destruction
	///				of the internal data of this resource.
	/// 
	/// @param[in]	parent_resource
	///				Pointer to a resource that owns this resource. Resources can have parent and child resources, this is just used
	///				to keep track of resources that are used by other resources and should not be removed unless the parent
	///				resources are removed.
	/// 
	/// @param[in]	file_path
	///				File path where to load the font resource from.
	/// 
	/// @param[in]	glyph_texel_size
	///				Glyph texel size tells the desired nominal height of a single letter in pixels/texels, however you should keep
	///				in mind that this value is only a guideline, the actual size will vary from font to font and a lot depends on
	///				what the font creator decided. Testing is the only way to know for sure.
	///				Since it's easy to scale the text at render time, this could be used as the quality of the letters, though for
	///				the perfect results you should use 1:1 texel mapping from the font to the window where you want the letters to
	///				be rendered to. ( that is to say, for perfect results use scale 1.0, 1.0 when rendering the text and use this
	///				parameter to control the size. )
	/// 
	/// @param[in]	use_alpha
	///				Use alpha channel instead of color on edge of the letters when rendering.
	/// 
	/// @param[in]	fallback_character
	///				Which character to use when desired character was not found in the font resource, eg. <B>&Delta;</B> might not
	///				appear on all fonts, so instead we use another in it's place.
	/// 
	/// @param[in]	glyph_atlas_padding
	///				Font glyphs (Letters, numbers, markings) are internally stored in a texture atlas for efficiency, however if
	///				you render the text much smaller than the actual size of the glyphs in the texture atlas, mipmapping is often
	///				used to make text appear less grainy. However if mipmapping is used, then the glyphs may eventually start to
	///				mix together in the final render, to decrease the amount of this "UV bleeding" you can increase the gap
	///				between glyphs in the texture atlas here.
	VK2D_API												FontResource(
		vk2d_internal::ResourceManagerImpl				*	resource_manager,
		uint32_t											loader_thread_index,
		ResourceBase									*	parent_resource,
		const std::filesystem::path						&	file_path,
		uint32_t											glyph_texel_size,
		bool												use_alpha,
		uint32_t											fallback_character,
		uint32_t											glyph_atlas_padding
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API												~FontResource();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the resource has been loaded or is in the process of being loaded.
	///
	///				This function will not wait but returns immediately with the result.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus									GetStatus();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block.
	///				As soon as the resource state becomes determined this function will return and code execution can continue.
	///
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined. Default value is std::chrono::nanoseconds::max() which
	///				makes this function wait indefinitely.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	///				Resource status can only be undetermined if timeout was given.
	VK2D_API ResourceStatus									WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max()
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block. As soon
	///				as the resource state becomes determined this function will return and code execution can continue.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus									WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Calculate how much space a given text string would occupy if it was rendered.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	text
	///				Text to get area size for.
	/// 
	/// @param[in]	kerning
	///				Spacing between letters. Positive values are farther apart. Value is based on size of the font texel size.
	/// 
	/// @param[in]	scale
	///				Value {1.0, 1.0} will map 1:1 to the font texel size.
	/// 
	/// @param[in]	vertical
	///				true if text should be rendered vertical, false if horisontal.
	/// 
	/// @param[in]	font_face
	///				Certain fonts may contain multiple font faces, this allows you to select which one to use.
	/// 
	/// @param[in]	wait_for_resource_load
	///				Setting this option to true will wait for the font resource to load before allowing execution to continue.
	///				If you're in a time critical situation where you are generating the text every frame and you cannot afford a few
	///				millisecond wait for the font to load up you can set this value to false. <br> This function will return empty
	///				rectangle area {} until the font has been fully loaded by the resource manager.
	/// 
	/// @return		Area the text would occupy if rendered.
	VK2D_API Rect2f											CalculateRenderedSize(
		std::string_view									text,
		float												kerning								= 0.0f,
		glm::vec2											scale								= glm::vec2( 1.0f, 1.0f ),
		bool												vertical							= false,
		uint32_t											font_face							= 0,
		bool												wait_for_resource_load				= true );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		VK2D class object checker function.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool											IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::FontResourceImpl>		impl;
};



} // vk2d
