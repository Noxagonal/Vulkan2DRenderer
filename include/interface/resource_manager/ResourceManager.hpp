#pragma once

#include <core/Common.hpp>

#include <containers/Color.hpp>
#include <containers/NameArray.hpp>

#include <interface/resources/texture/TextureResourceHandle.hpp>
#include <interface/resources/font/FontResourceHandle.hpp>
#include <interface/resources/material/MaterialResourceHandle.hpp>

#include <memory>
#include <filesystem>
#include <span>



namespace vk2d {

class ThreadPool;
class ResourceBase;
class TextureResource;
class FontResource;

namespace vk2d_internal {

class InstanceImpl;
class ResourceManagerImpl;

} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Resource manager for handling different user resources, eg. textures, fonts...
///
///				VK2D Is capable of loading and unloading resources in a background thread, and ResourceManager is
///				responsible of making it happen.
class ResourceManager
{
	friend class vk2d_internal::InstanceImpl;

	template<typename ResourceT>
	friend class ResourceHandleBase;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		This object should not be directly constructed, it is automatically created by Instance at it's creation.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param		instance
	///				Pointer back to owner who created this object.
	VK2D_API													ResourceManager(
		vk2d_internal::InstanceImpl							&	instance
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API ~ResourceManager();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a single layer texture resource from data.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	size
	///				Size of the texture in texels.
	/// 
	/// @param[in]	texels
	///				Data to use to construct the texture.
	///				- Must be big enough to contain all texel data at given size. ( must be at least: size.x * size.y ).
	///				- This data is copied over to internal memory before returning so you do not need to keep the vector around.
	/// 
	/// @return		Handle to newly created texture resource you can use when rendering.
	inline TextureResourceHandle								CreateTextureResource(
		glm::uvec2												size,
		const std::vector<Color8>							&	texels
	)
	{
		auto resource = DoCreateTextureResource(
			size,
			texels
		);
		if( !resource ) return {};

		return resource;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Load a single layer texture resource from a file.
	///
	///				File format is always converted to 8 bits-per-channel RGBA format internally regardless of file format used.
	///
	/// @warning	No HDRI support.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	file_path
	///				File path to the texture. Internally VK2D uses stb image loading library so some restrictions apply to the
	///				format used. See https://github.com/nothings/stb for supported formats.
	///				<br>
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
	///
	/// @return		Handle to newly created texture resource you can use when rendering.
	inline TextureResourceHandle								LoadTextureResource(
		const std::filesystem::path							&	file_path
	)
	{
		auto resource = DoLoadTextureResource(
			file_path
		);
		if( !resource ) return {};

		return resource;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a multi-layer texture resource from data.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	size
	///				Size of the texture in texels.
	/// 
	/// @param[in]	texels_listing
	///				Data to use to construct the texture.
	///				<br>
	///				- Each texel data vector must be big enough to contain all texel data at given size. ( must be at least:
	///				size.x * size.y ).
	///				- Lenght of the top vector determines the number of arrays the texture has.
	///				- Each top vector index corresponds to the texture array index. Eg. input is {{data1}{data2}} then "data1" is
	///				texture array layer 0 and "data2" is texture array layer 1.
	///				- Each texture layer must be the same size.
	///				- This data is copied over to internal memory before returning so you do not need to keep the vector around.
	/// 
	/// @return		Handle to newly created texture resource you can use when rendering.
	inline TextureResourceHandle								CreateArrayTextureResource(
		glm::uvec2												size,
		const std::vector<const std::vector<Color8>*>		&	texels_listing
	)
	{
		auto resource = DoCreateArrayTextureResource(
			size,
			texels_listing
		);
		if( !resource ) return {};

		return resource;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Load a multi-layer texture resource from files.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	file_path_listing
	///				A vector of file paths to use when creating the texture.
	///				<br>
	///				- Supported file formats are listed in ResourceManager::CreateTextureResource().
	///				- Each file path corresponds to a texture layer in given order. Eg. input is {{path1}{path2}} then "path1" is
	///				texture array layer 0 and "path2" is texture array layer 1.
	///				- Each texture layer must be the same size. If images in these file paths are not same size then texture
	///				loading will fail.
	/// 
	/// @return		Handle to newly created texture resource you can use when rendering.
	inline TextureResourceHandle								LoadArrayTextureResource(
		const std::vector<std::filesystem::path>			&	file_path_listing
	)
	{
		auto resource = DoLoadArrayTextureResource(
			file_path_listing
		);
		if( !resource ) return {};

		return resource;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Load a font resource from file.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	file_path
	///				File path to a font file. VK2D uses Freetype 2 so most common fonts are supported. See
	///				https://www.freetype.org/freetype2/docs/ for more info.
	///				<br>
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
	/// 
	/// @param[in]	glyph_texel_size
	///				Glyph texel size tells the desired nominal height of a single letter in pixels/texels, however you should
	///				keep in mind that this value is only a guideline, the actual size will vary from font to font and a lot
	///				depends on what the font creator decided. Testing is the only way to know for sure. Since it's easy to scale
	///				the text at render time, this could be used as the quality of the letters, though for the perfect results you
	///				should use 1:1 texel mapping from the font to the window where you want the letters to be rendered to. ( that
	///				is to say, for perfect results use scale 1.0, 1.0 when rendering the text and use this parameter to control
	///				the size. )
	/// 
	/// @param[in]	use_alpha
	///				Use alpha channel instead of color on edge of the letters when rendering.
	/// 
	/// @param[in]	fallback_character
	///				Which character to use when desired character was not found in the font resource, eg. <B>&Delta;</B> might
	///				not appear on all fonts, so instead we use another in it's place.
	/// 
	/// @param[in]	glyph_atlas_padding
	///				Font glyphs (Letters, numbers, markings) are internally stored in a texture atlas for efficiency, however if
	///				you render the text much smaller than the actual size of the glyphs in the texture atlas, mipmapping is often
	///				used to make text appear less grainy. However if mipmapping is used, then the glyphs may eventually start to
	///				mix together in the final render, to decrease the amount of this "UV bleeding" you can increase the gap
	///				between glyphs in the texture atlas here.
	/// 
	/// @return		Handle to newly created font resource you can use when rendering text.
	inline FontResourceHandle									LoadFontResource(
		const std::filesystem::path							&	file_path,
		uint32_t												glyph_texel_size			= 32,
		bool													use_alpha					= true,
		uint32_t												fallback_character			= '*',
		uint32_t												glyph_atlas_padding			= 8
	)
	{
		auto resource = DoLoadFontResource(
			file_path,
			glyph_texel_size,
			use_alpha,
			fallback_character,
			glyph_atlas_padding
		);
		if( !resource ) return {};

		return resource;
	}

	// TODO: Define material resource shader interface with template parameters

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<
		vk2d_internal::VertexBaseOrDerivedType					VertexT
		// typename												DrawT,
		// size_t												SamplerCount,
		// size_t												TextureCount,
		// size_t												ColorOutCount
	>
	MaterialResourceHandle<VertexT>								CreateMaterialResource(
		NameArray<VertexT::GetMemberCount()>					vertex_member_names,
		const MaterialCreateInfo							&	create_info					= {}
	)
	{
		// Using this template function allows us to transfer template parameters to the resource handle, which allows us to do
		// compile time error checking in some locations later.

		auto vertex_member_types = vk2d_internal::GetShaderMemberTypesAsString<VertexT>();
		auto vertex_members = std::array<vk2d_internal::ShaderMemberInfo, VertexT::GetMemberCount()>();
		for( size_t i = 0; i < vertex_member_types.size(); ++i )
		{
			if( vertex_member_names[ i ].empty() )
			{
				// TODO: Throw here instead of asserting.
				assert( 0 && "Vertex member name must not be empty." );
				return {};
			}

			vertex_members[ i ].type = vertex_member_types[ i ];
			vertex_members[ i ].name = vertex_member_names[ i ];
		}

		auto resource = DoCreateMaterialResource(
			vertex_members,
			create_info
		);
		if( !resource ) return {};

		return MaterialResourceHandle<VertexT>(
			resource
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool												IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API TextureResource								*	DoCreateTextureResource(
		glm::uvec2												size,
		const std::vector<Color8>							&	texels
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API TextureResource								*	DoLoadTextureResource(
		const std::filesystem::path							&	file_path
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API TextureResource								*	DoCreateArrayTextureResource(
		glm::uvec2												size,
		const std::vector<const std::vector<Color8>*>		&	texels_listing
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API TextureResource								*	DoLoadArrayTextureResource(
		const std::vector<std::filesystem::path>			&	file_path_listing
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API FontResource									*	DoLoadFontResource(
		const std::filesystem::path							&	file_path,
		uint32_t												glyph_texel_size			= 32,
		bool													use_alpha					= true,
		uint32_t												fallback_character			= '*',
		uint32_t												glyph_atlas_padding			= 8
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API MaterialResource								*	DoCreateMaterialResource(
		std::span<vk2d_internal::ShaderMemberInfo>				vertex_members,
		const MaterialCreateInfo							&	create_info					= {}
	);

public:
	// TODO: Put DestroyResource() in private section after all the handles have been implemented.

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy a resource.
	///
	///				VK2D does not track resource usage and it does not have a garbage collector, it is up to the host application to
	///				manually destroy resources that are no longer being used anywhere. Although destroying the resource manager
	///				itself does destroy all resources for you, it can be a good idea to manually free some unused resources from
	///				time to time to save on memory usage, especially for larger applications.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	resource
	///				Pointer to Resource to destroy. After this the resource is no longer valid and trying to use it will crash
	///				your application.
	VK2D_API void												DestroyResource(
		ResourceBase										*	resource
	);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::ResourceManagerImpl>			impl;
};



}

