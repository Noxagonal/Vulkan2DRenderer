#pragma once

#include "../../Core/SourceCommon.h"

#include "../../../Include/Types/Rect2.hpp"
#include "../../../Include/Types/Color.hpp"

#include "ResourceImpl.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {

class TextureResource;
class FontResource;

namespace _internal {

class ResourceManagerImpl;
class ThreadPrivateResource;



struct GlyphInfo {
	uint32_t											face_index							= {};
	uint32_t											atlas_index							= {};
	vk2d::Rect2f										uv_coords							= {};
	vk2d::Rect2f										horisontal_coords					= {};
	vk2d::Rect2f										vertical_coords						= {};
	float												horisontal_advance					= {};
	float												vertical_advance					= {};
};



class FontResourceImpl :
	public vk2d::_internal::ResourceImpl
{
public:
	FontResourceImpl(
		vk2d::FontResource							*	my_interface,
		vk2d::_internal::ResourceManagerImpl		*	resource_manager,
		uint32_t										loader_thread,
		vk2d::Resource								*	parent_resource,
		const std::filesystem::path					&	file_path,
		uint32_t										glyph_texel_size,
		bool											use_alpha,
		uint32_t										fallback_character,
		uint32_t										glyph_atlas_padding );

	~FontResourceImpl();

	vk2d::ResourceStatus																	GetStatus();

	vk2d::ResourceStatus																	WaitUntilLoaded(
		std::chrono::nanoseconds						timeout );

	vk2d::ResourceStatus																	WaitUntilLoaded(
		std::chrono::steady_clock::time_point			timeout );

	bool																					MTLoad(
		vk2d::_internal::ThreadPrivateResource		*	thread_resource );

	void																					MTUnload(
		vk2d::_internal::ThreadPrivateResource		*	thread_resource );

	bool																					FaceExists(
		uint32_t										font_face ) const;

	vk2d::TextureResource							*										GetTextureResource();

	const vk2d::_internal::GlyphInfo				*										GetGlyphInfo(
		uint32_t										font_face,
		uint32_t										character ) const;

	bool																					IsGood() const;

private:
	struct AtlasTexture {
		std::vector<vk2d::Color8>						data								= {};
		uint32_t										index								= {};
		uint32_t										previous_row_height					= {};
		uint32_t										current_row_height					= {};
		uint32_t										current_write_location				= {};
	};
	struct AtlasLocation {
		AtlasTexture								*	atlas_ptr							= {};
		uint32_t										atlas_index							= {};
		vk2d::Rect2u									location							= {};
	};
	struct FaceInfo {
		FT_Face											face								= {};
		std::vector<vk2d::_internal::GlyphInfo>			glyph_infos							= {};
		std::map<uint32_t, uint32_t>					charmap								= {};	// link character to a GlyphInfo vector
		uint32_t										fallback_glyph_index				= {};
	};

	AtlasTexture									*										CreateNewAtlasTexture();

	AtlasLocation																			ReserveSpaceForGlyphFromAtlasTextures(
		FT_GlyphSlot									glyph,
		uint32_t										glyph_atlas_padding );

	void																					CopyGlyphTextureToAtlasLocation(
		AtlasLocation									atlas_location,
		const std::vector<vk2d::Color8>				&	converted_texture_data );

	// Does everything.
	AtlasLocation																			AttachGlyphToAtlas(
		FT_GlyphSlot									glyph,
		uint32_t										glyph_atlas_padding,
		const std::vector<vk2d::Color8>				&	converted_texture_data );

	vk2d::FontResource								*	my_interface						= {};
	vk2d::_internal::ResourceManagerImpl			*	resource_manager					= {};

	bool												use_alpha							= {};
	uint32_t											glyph_texel_size					= {};
	uint32_t											glyph_atlas_padding					= {};
	uint32_t											fallback_character					= {};

	uint32_t											atlas_size							= {};

	AtlasTexture									*	current_atlas_texture				= {};
	std::vector<std::unique_ptr<AtlasTexture>>			atlas_textures						= {};
	std::vector<FaceInfo>								face_infos							= {};

	vk2d::TextureResource							*	texture_resource					= {};

	bool												is_good								= {};
};

} // _internal
} // vk2d
