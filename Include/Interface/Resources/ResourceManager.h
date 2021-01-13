#pragma once

#include "Core/Common.h"

#include "Types/Color.hpp"

#include <memory>
#include <filesystem>

namespace vk2d {

class ThreadPool;
class ResourceBase;
class TextureResource;
class FontResource;

namespace _internal {
class InstanceImpl;
class ResourceManagerImpl;
}



/// @brief		VK2D Is capable of loading and unloading resources in a background thread,
///				and vk2d::ResourceManager is responsible of making it happen.
class ResourceManager {
	friend class vk2d::_internal::InstanceImpl;

	/// @brief		This object should not be directly constructed, it is automatically created by
	///				vk2d::Instance at it's creation.
	/// @note		Multithreading: Main thread only.
	/// @param		parent_instance
	///				Pointer back to owner who created this object.
	VK2D_API																				ResourceManager(
		vk2d::_internal::InstanceImpl													*	parent_instance
	);

public:
	VK2D_API																				~ResourceManager();

	/// @brief		Create a single layer texture resource from data.
	/// @note		Multithreading: Any thread.
	/// @param[in]	size
	///				Size of the texture in texels.
	/// @param[in]	texels
	///				Data to use to construct the texture.
	///				- Must be big enough to contain all texel data at given size.
	///				( must be at least: size.x * size.y ).
	///				- This data is copied over to internal memory before returning so
	///				you do not need to keep the vector around.
	/// @return		Handle to newly created texture resource you can use when rendering.
	VK2D_API TextureResource								*	VK2D_APIENTRY				CreateTextureResource(
		glm::uvec2												size,
		const std::vector<vk2d::Color8>						&	texels );

	/// @brief		Load a single layer texture resource from a file. File format is always
	///				converted to 8 bits-per-channel RGBA format internally regardless of file
	///				format used. No HDRI support.
	/// @note		Multithreading: Any thread.
	/// @param[in]	file_path
	///				File path to the texture. Internally VK2D uses stb image loading library so
	///				some restrictions apply to the format used. See https://github.com/nothings/stb
	///				for more info.
	///				- Supported file formats in VK2D are: <br>
	///				<table>
	///					<tr>
	///						<th>Format</th>	<th>Support level</th>
	///					</tr>
	///					<tr>
	///						<td>JPEG</td>	<td>baseline & progressive supported, 12 bits-per-channel/arithmetic not supported</td>
	///					</tr>
	///					<tr>
	///						<td>PNG</td>	<td>1/2/4/8/16 bit-per-channel supported</td>
	///					</tr>
	///					<tr>
	///						<td>TGA</td>	<td>not sure what subset, if a subset</td>
	///					</tr>
	///					<tr>
	///						<td>BMP</td>	<td>no support for 1bpp or RLE</td>
	///					</tr>
	///					<tr>
	///						<td>PSD</td>	<td>composited view only, no extra channels, 8/16 bits-per-channel</td>
	///					</tr>
	///					<tr>
	///						<td>PIC</td>	<td>Softimage PIC</td>
	///					</tr>
	///					<tr>
	///						<td>PNM</td>	<td>PPM and PGM binary only</td>
	///					</tr>
	///				</table>
	/// @return		Handle to newly created texture resource you can use when rendering.
	VK2D_API vk2d::TextureResource							*	VK2D_APIENTRY				LoadTextureResource(
		const std::filesystem::path							&	file_path );

	/// @brief		Create a multi-layer texture resource from data.
	/// @note		Multithreading: Any thread.
	/// @param[in]	size
	///				Size of the texture in texels.
	/// @param[in]	texels_listing
	///				Data to use to construct the texture.
	///				- Each texel data vector must be big enough to contain all texel data at
	///				given size. ( must be at least: size.x * size.y ).
	///				- Lenght of the top vector determines the number of arrays the texture has.
	///				- Each top vector index corresponds to the texture array index. Eg. input
	///				is {{data1}{data2}} then "data1" is texture array layer 0 and "data2" is
	///				texture array layer 1.
	///				- Each texture layer must be the same size.
	///				- This data is copied over to internal memory before returning so you do
	///				not need to keep the vector around.
	/// @return		Handle to newly created texture resource you can use when rendering.
	VK2D_API TextureResource								*	VK2D_APIENTRY				CreateArrayTextureResource(
		glm::uvec2												size,
		const std::vector<const std::vector<vk2d::Color8>*>	&	texels_listing );

	/// @brief		Load a multi-layer texture resource from files.
	/// @note		Multithreading: Any thread.
	/// @param[in]	file_path_listing
	///				A vector of file paths to use when creating the texture.
	///				- Supported file formats are listed in
	///				vk2d::ResourceManager::CreateTextureResource().
	///				- Each file path corresponds to a texture layer in given order. Eg. input
	///				is {{path1}{path2}} then "path1" is texture array layer 0 and "path2" is
	///				texture array layer 1.
	///				- Each texture layer must be the same size. If images in these file paths
	///				are not same size then texture loading will fail.
	/// @return		Handle to newly created texture resource you can use when rendering.
	VK2D_API vk2d::TextureResource							*	VK2D_APIENTRY				LoadArrayTextureResource(
		const std::vector<std::filesystem::path>			&	file_path_listing );

	/// @brief		Load a font resource from file, which is needed to render text in a window.
	/// @note		Multithreading: Any thread.
	/// @param[in]	file_path
	///				File path to a font file. VK2D uses Freetype 2 so most common fonts are
	///				supported. See https://www.freetype.org/freetype2/docs/ for more info. <br>
	///				<table>
	///				<tr> <th> Supported formats
	///				<tr> <td> TrueType fonts (TTF) 
	///				<tr> <td> TrueType collections (TTC)
	///				<tr> <td> CFF fonts
	///				<tr> <td> WOFF fonts
	///				<tr> <td> OpenType fonts (OTF, both TrueType and CFF variants)
	///				<tr> <td> OpenType collections (OTC)
	///				<tr> <td> Type 1 fonts (PFA and PFB)
	///				<tr> <td> CID-keyed Type 1 fonts
	///				<tr> <td> SFNT-based bitmap fonts, including color Emoji
	///				<tr> <td> X11 PCF fonts
	///				<tr> <td> Windows FNT fonts
	///				<tr> <td> BDF fonts (including anti-aliased ones)
	///				<tr> <td> PFR fonts
	///				<tr> <td> Type 42 fonts (limited support)
	///				</table>
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
	/// @return		Handle to newly created font resource you can use when rendering text.
	VK2D_API vk2d::FontResource								*	VK2D_APIENTRY				LoadFontResource(
		const std::filesystem::path							&	file_path,
		uint32_t												glyph_texel_size			= 32,
		bool													use_alpha					= true,
		uint32_t												fallback_character			= '*',
		uint32_t												glyph_atlas_padding			= 8 );

	/// @brief		Destroy a resource. VK2D does not track resource usage and it does
	///				not have a garbage collector, it is up to the host application to
	///				manually destroy resources that are no longer being used anywhere.
	///				Although destroying the resource manager itself does destroy all
	///				resources for you, it can be a good idea to manually free some unused
	///				resources from time to time to save on memory usage, especially for
	///				larger applications.
	/// @note		Multithreading: Any thread.
	/// @param[in]	resource
	///				Pointer to vk2d::Resource to destroy. After this the resource is no
	///				longer valid and trying to use it will crash your application.
	VK2D_API void												VK2D_APIENTRY				DestroyResource(
		vk2d::ResourceBase																*	resource );

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool												VK2D_APIENTRY				IsGood() const;

private:
	std::unique_ptr<_internal::ResourceManagerImpl>				impl;
};



}

