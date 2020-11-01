#pragma once

#include "../Core/SourceCommon.h"

#include "../Core/ThreadPrivateResources.h"
#include "../../../Include/Interface/RenderPrimitives.h"
#include "../../Include/Interface/TextureResource.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {

class TextureResource;
class FontResource;

namespace _internal {

class ResourceManagerImpl;



struct GlyphInfo {
	uint32_t											face_index							= {};
	uint32_t											atlas_index							= {};
	vk2d::AABB2f										uv_coords							= {};
	vk2d::AABB2f										horisontal_coords					= {};
	vk2d::AABB2f										vertical_coords						= {};
	float												horisontal_advance					= {};
	float												vertical_advance					= {};
};



class FontResourceImpl {
public:
	FontResourceImpl(
		vk2d::FontResource							*	font_resource,
		vk2d::_internal::ResourceManagerImpl		*	resource_manager,
		uint32_t										glyph_texel_size,
		bool											use_alpha,
		uint32_t										fallback_character,
		uint32_t										glyph_atlas_padding );

	~FontResourceImpl();

	bool																					IsLoaded();

	bool																					WaitUntilLoaded();

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

	bool																					IsGood();

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
		vk2d::AABB2u									location							= {};
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

	vk2d::FontResource								*	font_resource_parent				= {};
	vk2d::_internal::ResourceManagerImpl			*	resource_manager_parent				= {};

	bool												use_alpha							= {};
	uint32_t											glyph_texel_size					= {};
	uint32_t											glyph_atlas_padding					= {};
	uint32_t											fallback_character					= {};

	uint32_t											atlas_size							= {};

	AtlasTexture									*	current_atlas_texture				= {};
	std::vector<std::unique_ptr<AtlasTexture>>			atlas_textures						= {};
	std::vector<FaceInfo>								face_infos							= {};

	vk2d::TextureResource							*	texture_resource					= {};

	std::mutex											is_loaded_mutex;

	bool												is_loaded							= {};
	bool												is_good								= {};
};

} // _internal
} // vk2d
