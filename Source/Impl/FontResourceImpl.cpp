
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/FontResourceImpl.h"
#include "../../Include/Interface/FontResource.h"

#include "../Header/Impl/ResourceManagerImpl.h"
#include "../../Include/Interface/TextureResource.h"

#include <stb_image_write.h>
#include <sstream>

uint32_t RoundToCeilingPowerOfTwo( uint32_t value )
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}


vk2d::_internal::FontResourceImpl::FontResourceImpl(
	vk2d::FontResource						*	font_resource,
	vk2d::_internal::ResourceManagerImpl	*	resource_manager )
{
	font_resource_parent		= font_resource;
	resource_manager_parent		= resource_manager;
	assert( font_resource_parent );
	assert( resource_manager_parent );

	is_good		= true;
}

vk2d::_internal::FontResourceImpl::~FontResourceImpl()
{}

bool vk2d::_internal::FontResourceImpl::IsLoaded()
{
	std::unique_lock<std::mutex>		is_loaded_lock( is_loaded_mutex, std::defer_lock );
	if( !is_loaded_lock.try_lock() ) {
		return false;
	}

	if( is_loaded )									return true;
	if( !is_good )									return false;
	if( !font_resource_parent->load_function_ran )	return false;
	if( font_resource_parent->FailedToLoad() )		return false;

	if( texture_resource ) {
		is_loaded = texture_resource->IsLoaded();
		return is_loaded;
	}
	return false;
}

bool vk2d::_internal::FontResourceImpl::WaitUntilLoaded()
{
	while( true ) {
		if( font_resource_parent->FailedToLoad() ) {
			return false;
		}
		if( font_resource_parent->IsLoaded() ) {
			break;
		} else {
			std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
		}
	}

	{
		bool texture_exists = false;
		{
			std::lock_guard<std::mutex> lock_guard( is_loaded_mutex );
			texture_exists	= !!texture_resource;
		}
		if( texture_exists ) {
			return texture_resource->WaitUntilLoaded();
		}
	}
	return false;
}

bool vk2d::_internal::FontResourceImpl::MTLoad(
	vk2d::_internal::ThreadPrivateResource		*	thread_resource
)
{
	// TODO: additional parameters:
	// Alpha blended glyph atlasses.
	// From raw file.
	// Set font size.

	auto loader_thread_resource		= static_cast<vk2d::_internal::ThreadLoaderResource*>( thread_resource );
	auto renderer					= loader_thread_resource->GetRenderer();
	auto path_str					= font_resource_parent->GetFilePaths()[ 0 ].string();
	auto path						= path_str.c_str();
	auto glyph_size					= uint32_t( 50 );	// TODO: proper parameter
	auto glyph_atlas_padding		= uint32_t( 8 );	// Empty space around the glyph in atlas texture
	auto max_texture_size			= renderer->GetPhysicalDeviceProperties().limits.maxImageDimension2D;
	auto min_texture_size			= std::min( uint32_t( 512 ), max_texture_size );

	auto average_to_max_weight		= 0.05;		// 0 is towards average, 1 is towards maximum
	auto total_glyph_count			= uint64_t( 0 );
	auto maximum_glyph_size			= vk2d::Vector2d( 0.0, 0.0 );
	auto average_glyph_size			= vk2d::Vector2d( 0.0, 0.0 );

	if( font_resource_parent->IsFromFile() ) {
		// Try to load from file.

		// Get amount of faces in the file
		uint32_t face_count = 0;
		{
			FT_Face face {};
			auto ft_error = FT_New_Face(
				loader_thread_resource->GetFreeTypeInstance(),
				path,
				-1,
				&face
			);

			switch( ft_error ) {
			case FT_Err_Ok:
				// All good
				face_count = uint32_t( face->num_faces );
				FT_Done_Face( face );
				break;
			case FT_Err_Cannot_Open_Resource:
				// Cannot open file error
				renderer->Report(
					vk2d::ReportSeverity::NON_CRITICAL_ERROR,
					std::string( "Cannot load font: File not found: " ) + path_str
				);
				return false;
			case FT_Err_Unknown_File_Format:
				// Unknown file format error
				renderer->Report(
					vk2d::ReportSeverity::NON_CRITICAL_ERROR,
					std::string( "Cannot load font: Unknown file format: " ) + path_str
				);
				return false;
			default:
				// Other errors
				renderer->Report(
					vk2d::ReportSeverity::NON_CRITICAL_ERROR,
					std::string( "Cannot load font: " ) + path_str
				);
				return false;
			}
		}

		assert( !face_infos.size() );	// If hit, this function was called twice for some reason, should never happen.
		face_infos.clear();
		face_infos.resize( face_count );
		for( uint32_t i = 0; i < face_count; ++i ) {
			FT_Face face {};
			{
				auto ft_error = FT_New_Face(
					loader_thread_resource->GetFreeTypeInstance(),
					path,
					i,
					&face
				);
				if( ft_error ) {
					renderer->Report(
						vk2d::ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: " ) + path_str
					);
					return false;
				}
			}
			{
				auto ft_error = FT_Set_Pixel_Sizes(
					face,
					0,
					glyph_size
				);
				if( ft_error ) {
					renderer->Report(
						vk2d::ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: " ) + path_str
					);
					return false;
				}
			}

			// Get glyph sizes
			total_glyph_count			+= uint64_t( face->num_glyphs ) + 1;
			for( decltype( face->num_glyphs ) i = 0; i < face->num_glyphs; ++i ) {

				auto ft_load_error = FT_Load_Glyph( face, FT_UInt( i ), FT_LOAD_BITMAP_METRICS_ONLY );
				if( ft_load_error ) {
					renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot load glyph for bitmap metrics!" );
					return false;
				}

				auto glyph_space_occupancy	=
					vk2d::Vector2d(
						double( face->glyph->bitmap.width	+ glyph_atlas_padding ),
						double( face->glyph->bitmap.rows	+ glyph_atlas_padding )
					);

				maximum_glyph_size.x	= std::max( maximum_glyph_size.x, glyph_space_occupancy.x );
				maximum_glyph_size.y	= std::max( maximum_glyph_size.y, glyph_space_occupancy.y );

				average_glyph_size		+= glyph_space_occupancy;
			}

			face_infos[ i ].face	= face;
		}

	} else {
		// Try to load from data.

		assert( 0 && "Not implemented yet!" );
	}

	if( !total_glyph_count ) {
		renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Cannot load font: Font contains no glyphs!" );
		return false;
	}

	// Estimate appropriate atlas size.
	// This code tries to find a tradeoff between texture size and amount so that 1 to 3 textures are
	// created. For example if glyphs do not fit into 3 textures of size 512 * 512, a larger 1024 * 1024
	// texture is created which should be able to contain the glyphs.
	average_glyph_size	+= vk2d::Vector2d( double( glyph_atlas_padding ), double( glyph_atlas_padding ) ) * 2.0;
	average_glyph_size	/= float( total_glyph_count );
	{
		auto estimated_glyph_space_requirements		= average_glyph_size * ( 1.0 - average_to_max_weight ) + maximum_glyph_size * average_to_max_weight;
		auto estimated_average_space_requirements	= estimated_glyph_space_requirements / 1.5;	// aim to have 1 to 4 textures to optimzise memory usage
		atlas_size			=
			RoundToCeilingPowerOfTwo(
				uint32_t(
					std::sqrt(
						std::ceil( estimated_average_space_requirements.x ) *
						std::ceil( estimated_average_space_requirements.y ) *
						double( total_glyph_count )
					)
				)
			);
		if( atlas_size > max_texture_size ) atlas_size = max_texture_size;
		if( atlas_size < min_texture_size ) atlas_size = min_texture_size;
	}

	// Stop if we don't have any font's to work with.
	if( !face_infos.size() ) {
		renderer->Report(
			vk2d::ReportSeverity::NON_CRITICAL_ERROR,
			std::string( "Internal error: Cannot load font: " ) + path_str
		);
		return false;
	}

	current_atlas_texture	= CreateNewAtlasTexture();

	// Process all glyphs from all font faces
	for( size_t face_index = 0; face_index < face_infos.size(); ++face_index ) {
		auto & face = face_infos[ face_index ];
		face.glyph_infos.resize( face.face->num_glyphs );

		for( auto glyph_index = 0; glyph_index < face.face->num_glyphs; ++glyph_index ) {
			{
				auto ft_load_error = FT_Load_Char( face.face, glyph_index, FT_LOAD_DEFAULT );
				if( ft_load_error ) {
					renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot load glyph!" );
					return false;
				}
			}
			{
				auto ft_render_error = FT_Render_Glyph( face.face->glyph, FT_RENDER_MODE_NORMAL );
				if( ft_render_error ) {
					renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot render glyph!" );
					return false;
				}
			}
			auto	ft_glyph	= face.face->glyph;
			auto &	ft_bitmap	= ft_glyph->bitmap;

			std::vector<vk2d::Color8> final_glyph_pixels( size_t( ft_bitmap.rows ) * size_t( ft_bitmap.width ) );

			switch( ft_bitmap.pixel_mode ) {
			case FT_PIXEL_MODE_MONO:
			{
				for( uint32_t y = 0; y < ft_bitmap.rows; ++y ) {
					for( uint32_t x = 0; x < ft_bitmap.width; ++x ) {
						auto	texel_pos	= y * ft_bitmap.width + x;
						auto &	dst			= final_glyph_pixels[ texel_pos ];
						auto	src_bit		= 7 - x % 8;
						auto	src_byte	= ft_bitmap.buffer[ ( y * ft_bitmap.pitch ) + ( x / 8 ) ];

						dst.r				= ( ( src_byte >> src_bit ) & 1 ) * 255;
						dst.g				= dst.r;
						dst.b				= dst.r;
						dst.a				= dst.r;
					}
				}
			}
			break;

			case FT_PIXEL_MODE_GRAY:
			{
				for( uint32_t y = 0; y < ft_bitmap.rows; ++y ) {
					for( uint32_t x = 0; x < ft_bitmap.width; ++x ) {
						auto	texel_pos	= y * ft_bitmap.width + x;
						auto &	dst			= final_glyph_pixels[ texel_pos ];
						auto	src			= ft_bitmap.buffer[ texel_pos ];

						dst.r				= src;
						dst.g				= src;
						dst.b				= src;
						dst.a				= src;
					}
				}
			}
			break;

			case FT_PIXEL_MODE_BGRA:
			{
				for( uint32_t y = 0; y < ft_bitmap.rows; ++y ) {
					for( uint32_t x = 0; x < ft_bitmap.width; ++x ) {
						auto	texel_pos	= y * ft_bitmap.width + x;
						auto &	dst			= final_glyph_pixels[ texel_pos ];
						auto	src			= &ft_bitmap.buffer[ texel_pos * 4 ];

						dst.r				= src[ 1 ];
						dst.g				= src[ 2 ];
						dst.b				= src[ 3 ];
						dst.a				= src[ 0 ];
					}
				}
			}
			break;

			default:
				// Unsupported
				renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, Unsupported pixel format!" );
				return false;
				break;
			}

			// Attach rendered glyph to final texture atlas.
			auto atlas_location			= AttachGlyphToAtlas(
				face.face->glyph,
				glyph_atlas_padding,
				final_glyph_pixels
			);
			if( !atlas_location.atlas_ptr ) {
				renderer->Report(
					vk2d::ReportSeverity::NON_CRITICAL_ERROR,
					"Internal error: Cannot create font, cannot copy glyph image to font texture atlas!"
				);
				return false;
			}

			// Create glyph info structure for the glyph
			{
				vk2d::AABB2f uv_coords		= {
					float( atlas_location.location.top_left.x ) / float( atlas_size ),
					float( atlas_location.location.top_left.y ) / float( atlas_size ),
					float( atlas_location.location.bottom_right.x ) / float( atlas_size ),
					float( atlas_location.location.bottom_right.y ) / float( atlas_size )
				};

				vk2d::_internal::FontResourceImpl::GlyphInfo glyph_info {};
				glyph_info.face_index				= uint32_t( face_index );
				glyph_info.atlas_index				= atlas_location.atlas_index;
				glyph_info.uv_coords				= uv_coords;

				face.glyph_infos[ glyph_index ]		= glyph_info;
			}
		}
	}

	// This should be disabled once the 
#pragma VK2D_WARNING( "REMOVE THIS AFTER THOROUGHLY TESTING FONT LIBRARY!" )
	if( atlas_textures.size() >= 4 ) {
		renderer->Report( vk2d::ReportSeverity::INFO, "Internal info: Created 4 or more textures for font library, could be balanced better." );
		assert( 0 && "Too many textures, should have used one or two larger textures instead!" );
	}

#pragma VK2D_WARNING( "REMOVE THIS AFTER THOROUGHLY TESTING FONT LIBRARY!" )
	if( atlas_size > min_texture_size ) {
		if( atlas_textures.size() == 1 ) {
			if( atlas_textures[ 0 ]->previous_row_height < uint32_t( atlas_size * 0.70 ) ) {
				// Texture is underutilized
				renderer->Report( vk2d::ReportSeverity::INFO, "Internal info: Created a single underutilized texture for font library, could be balanced better." );
				assert( 0 && "Underutilized texture, should have used multiple smaller textures!" );
			}
		}
	}

	// Everything is baked into the atlas, create texture resource to store it.
	std::vector<std::vector<vk2d::Color8>*>		texture_data_array( atlas_textures.size() );
	for( size_t i = 0; i < atlas_textures.size(); ++i ) {
		texture_data_array[ i ]		= &atlas_textures[ i ]->data;

		/*
		// DEBUG: WRITE TO FILE!
		std::stringstream ss;
		ss << "GlyphAtlasTest_"
			<< i
			<< ".jpg";
		stbi_write_jpg( ss.str().c_str(), atlas_size, atlas_size, 4, atlas_textures[ i ]->data.data(), 90 );
		*/
	}

	{
		std::lock_guard<std::mutex> lock_guard( is_loaded_mutex );

		texture_resource = resource_manager_parent->CreateArrayTextureResource(
			vk2d::Vector2u( atlas_size, atlas_size ),
			texture_data_array,
			font_resource_parent
		);
		if( !texture_resource ) {
			renderer->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot create texture resource for font!" );
			return false;
		}
	}

	return true;
}

void vk2d::_internal::FontResourceImpl::MTUnload( vk2d::_internal::ThreadPrivateResource * thread_resource )
{
	for( auto f : face_infos ) {
		FT_Done_Face( f.face );
	}
	face_infos.clear();

}

vk2d::TextureResource * vk2d::_internal::FontResourceImpl::GetTextureResource()
{
	std::lock_guard<std::mutex> lock_guard( is_loaded_mutex );
	return texture_resource;
}

bool vk2d::_internal::FontResourceImpl::IsGood()
{
	return is_good;
}

vk2d::_internal::FontResourceImpl::AtlasTexture * vk2d::_internal::FontResourceImpl::CreateNewAtlasTexture()
{
	auto new_atlas_texture			= std::make_unique<vk2d::_internal::FontResourceImpl::AtlasTexture>();

	new_atlas_texture->data.resize( size_t( atlas_size ) * size_t( atlas_size ) );
	new_atlas_texture->index		= uint32_t( atlas_textures.size() );

	auto new_atlas_texture_ptr		= new_atlas_texture.get();
	atlas_textures.push_back( std::move( new_atlas_texture ) );
	return new_atlas_texture_ptr;
}

vk2d::_internal::FontResourceImpl::AtlasLocation vk2d::_internal::FontResourceImpl::ReserveSpaceForGlyphFromAtlasTextures(
	FT_GlyphSlot		glyph,
	uint32_t			glyph_atlas_padding
)
{
	assert( current_atlas_texture );

	auto FindLocationInAtlasTexture =[](
		vk2d::_internal::FontResourceImpl::AtlasTexture		*	atlas_texture,
		FT_GlyphSlot											glyph,
		uint32_t												atlas_size,
		uint32_t												glyph_atlas_padding
		) -> vk2d::_internal::FontResourceImpl::AtlasLocation
	{
		uint32_t glyph_width		= uint32_t( glyph->bitmap.width )	+ glyph_atlas_padding;
		uint32_t glyph_height		= uint32_t( glyph->bitmap.rows )	+ glyph_atlas_padding;

		// Find space in the current atlas texture.
		if( atlas_texture->previous_row_height + glyph_height + glyph_atlas_padding < atlas_size ) {
			// Fits height wise.

			if( atlas_texture->current_write_location + glyph_width + glyph_atlas_padding < atlas_size ) {
				// Fits width wise, fits completely.

				vk2d::_internal::FontResourceImpl::AtlasLocation new_glyph_location {};
				new_glyph_location.atlas_ptr				= atlas_texture;
				new_glyph_location.atlas_index				= atlas_texture->index;
				new_glyph_location.location.top_left		= {
					atlas_texture->current_write_location	+ glyph_atlas_padding,
					atlas_texture->previous_row_height		+ glyph_atlas_padding
				};
				new_glyph_location.location.bottom_right	=
					new_glyph_location.location.top_left	+
					vk2d::Vector2u( uint32_t( glyph->bitmap.width ), uint32_t( glyph->bitmap.rows ) );

				// update current row height and write locations before returning the result.
				atlas_texture->current_row_height			= std::max( atlas_texture->current_row_height, glyph_height );
				atlas_texture->current_write_location		+= glyph_width;

				return new_glyph_location;
			}

			// Does not fit width wise, go to the next row.
			atlas_texture->previous_row_height				+= atlas_texture->current_row_height;
			atlas_texture->current_row_height				= 0;
			atlas_texture->current_write_location			= 0;

			// Check height again.
			if( atlas_texture->previous_row_height + glyph_height + glyph_atlas_padding < atlas_size ) {
				// Fits height wise.

				if( atlas_texture->current_write_location + glyph_width + glyph_atlas_padding < atlas_size ) {
					// Fits width wise.

					vk2d::_internal::FontResourceImpl::AtlasLocation new_glyph_location {};
					new_glyph_location.atlas_ptr				= atlas_texture;
					new_glyph_location.atlas_index				= atlas_texture->index;
					new_glyph_location.location.top_left		= {
						atlas_texture->current_write_location	+ glyph_atlas_padding,
						atlas_texture->previous_row_height		+ glyph_atlas_padding
					};
					new_glyph_location.location.bottom_right	=
						new_glyph_location.location.top_left	+
						vk2d::Vector2u( uint32_t( glyph->bitmap.width ), uint32_t( glyph->bitmap.rows ) );

					// update current row height and write locations before returning the result.
					atlas_texture->current_row_height		= std::max( atlas_texture->current_row_height, glyph_height );
					atlas_texture->current_write_location	+= glyph_width;

					return new_glyph_location;
				}

				// Does not fit width wise to a new row, this would only happen if a single
				// face glyph is too large to fit into a single atlas. This is an error.
				assert( 0 && "Glyph too big" );
				return {};
			}

			// Does not fit to a new row, need a new atlas texture
			return {};
		}

		// Does not fit at all, need a new atlas texture.
		return {};
	};

	auto new_location = FindLocationInAtlasTexture(
		current_atlas_texture,
		glyph,
		atlas_size,
		glyph_atlas_padding
	);
	if( !new_location.atlas_ptr ) {
		// Not enough space found, create new atlas and try again.

		current_atlas_texture = CreateNewAtlasTexture();
		if( !current_atlas_texture ) {
			// Failed to create new atlas texture.
			resource_manager_parent->GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot create new atlas texture for font!" );
			return {};
		}

		// Got new atlas texture, retry finding space in it.
		new_location	= FindLocationInAtlasTexture(
			current_atlas_texture,
			glyph,
			atlas_size,
			glyph_atlas_padding
		);
		if( !new_location.atlas_ptr ) {
			// Still could not find enough space, a single font face glyph is too large
			// to fit entire atlas, this should not happen so we raise an error.
			resource_manager_parent->GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, a single glyph wont fit into a new atlas." );
			return {};
		}

		return new_location;
	}
	return new_location;
}

void vk2d::_internal::FontResourceImpl::CopyGlyphTextureToAtlasLocation(
	AtlasLocation							atlas_location,
	const std::vector<vk2d::Color8>		&	converted_texture_data )
{
	assert( atlas_location.atlas_ptr );

	auto glyph_width		= atlas_location.location.bottom_right.x - atlas_location.location.top_left.x;
	auto glyph_height		= atlas_location.location.bottom_right.y - atlas_location.location.top_left.y;
	vk2d::Vector2u location	= atlas_location.location.top_left;

	for( uint32_t gy = 0; gy < glyph_height; ++gy ) {
		for( uint32_t gx = 0; gx < glyph_width; ++gx ) {
			auto ax		= location.x + gx;
			auto ay		= location.y + gy;
			atlas_location.atlas_ptr->data[ ay * atlas_size + ax ]	= converted_texture_data[ gy * glyph_width + gx ];
		}
	}
}

vk2d::_internal::FontResourceImpl::AtlasLocation vk2d::_internal::FontResourceImpl::AttachGlyphToAtlas(
	FT_GlyphSlot							glyph,
	uint32_t								glyph_atlas_padding,
	const std::vector<vk2d::Color8>		&	converted_texture_data )
{
	auto atlas_location = ReserveSpaceForGlyphFromAtlasTextures(
		glyph,
		glyph_atlas_padding
	);
	if( atlas_location.atlas_ptr ) {
		CopyGlyphTextureToAtlasLocation(
			atlas_location,
			converted_texture_data
		);
		return atlas_location;
	} else {
		resource_manager_parent->GetRenderer()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot attach glyph to atlas texture!" );
		return {};
	}
}
