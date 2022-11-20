
#include <core/SourceCommon.hpp>

#include <interface/resources/FontResource.hpp>
#include "FontResourceImpl.hpp"

#include "ResourceManagerImpl.hpp"

#include <interface/resources/TextureResource.hpp>
#include <system/ThreadPrivateResources.hpp>
#include <interface/InstanceImpl.hpp>
#include <stb_image_write.h>



namespace vk2d {
namespace vk2d_internal {



// Private function declarations.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t RoundToCeilingPowerOfTwo(
	uint32_t			value
);



}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::FontResourceImpl::FontResourceImpl(
	FontResource							&	my_interface,
	ResourceManagerImpl						&	resource_manager,
	uint32_t									loader_thread_index,
	ResourceBase							*	parent_resource,
	const std::filesystem::path				&	file_path,
	uint32_t									glyph_texel_size,
	bool										use_alpha,
	uint32_t									fallback_character,
	uint32_t									glyph_atlas_padding
) :
	ResourceImplBase(
		my_interface,
		loader_thread_index,
		resource_manager,
		parent_resource,
		{ file_path }
	),
	my_interface( my_interface ),
	glyph_texel_size( glyph_texel_size ),
	use_alpha( use_alpha ),
	fallback_character( fallback_character ),
	glyph_atlas_padding( glyph_atlas_padding )
{
	is_good		= true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::FontResourceImpl::~FontResourceImpl()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::FontResourceImpl::GetStatus()
{
	if( !is_good )				return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED )
	{

		if( load_function_run_fence.IsSet() )
		{
			// "texture_resource" is set by the MTLoad() function so we can access it
			// without further mutex locking. ( "load_function_run_fence" is set )
			status = local_status = texture_resource->GetStatus();
		}
	}

	return local_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::FontResourceImpl::WaitUntilLoaded(
	std::chrono::nanoseconds timeout
)
{
	if( timeout == std::chrono::nanoseconds::max() ) {
		return WaitUntilLoaded( std::chrono::steady_clock::time_point::max() );
	}
	return WaitUntilLoaded( std::chrono::steady_clock::now() + timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::ResourceStatus vk2d::vk2d_internal::FontResourceImpl::WaitUntilLoaded(
	std::chrono::steady_clock::time_point timeout
)
{
	// Make sure timeout is in the future.
	assert( timeout == std::chrono::steady_clock::time_point::max() ||
		timeout + std::chrono::seconds( 5 ) >= std::chrono::steady_clock::now() );

	if( !is_good ) return ResourceStatus::FAILED_TO_LOAD;

	auto local_status = status.load();
	if( local_status == ResourceStatus::UNDETERMINED ) {

		if( load_function_run_fence.Wait( timeout ) ) {
			status = local_status = texture_resource->WaitUntilLoaded( timeout );
		}

	}

	return local_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::FontResourceImpl::MTLoad(
	ThreadPrivateResource * thread_resource
)
{
	// TODO: additional parameters:
	// From raw file.

	assert( thread_resource );
	assert( my_interface.impl->GetFilePaths().size() );

	auto loader_thread_resource		= static_cast<ThreadLoaderResource*>( thread_resource );
	auto & instance					= loader_thread_resource->GetInstance();
	auto path_str					= my_interface.impl->GetFilePaths()[ 0 ].string();
	auto max_texture_size			= instance.GetVulkanDevice().GetVulkanPhysicalDeviceProperties().limits.maxImageDimension2D;
	auto min_texture_size			= std::min( uint32_t( 128 ), max_texture_size );

	// average_to_max_weight variable is used to estimate glyph space requirements on atlas textures.
	// 0 sets the estimation to average size, 1 sets the estimation to max weights. Default is 0.05
	// which should give enough space in the atlas to contain all glyphs in 
	auto average_to_max_weight					= 0.05;
	auto total_glyph_count						= size_t( 0 );
	auto maximum_glyph_size						= glm::dvec2( 0.0, 0.0 );
	auto maximum_glyph_bitmap_size				= glm::dvec2( 0.0, 0.0 );
	auto maximum_glyph_bitmap_occupancy_size	= glm::dvec2( 0.0, 0.0 );
	auto average_glyph_bitmap_occupancy_size	= glm::dvec2( 0.0, 0.0 );

	if( my_interface.impl->IsFromFile() ) {
		// Try to load from file.

		// Get amount of faces in the file
		size_t face_count = 0;
		{
			FT_Face face {};
			auto ft_error = FT_New_Face(
				loader_thread_resource->GetFreeTypeInstance(),
				path_str.c_str(),
				-1,
				&face
			);

			switch( ft_error ) {
				case FT_Err_Ok:
					// All good
					face_count = size_t( face->num_faces );
					FT_Done_Face( face );
					break;
				case FT_Err_Cannot_Open_Resource:
					// Cannot open file error
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: File not found: " ) + path_str
					);
					return false;
				case FT_Err_Unknown_File_Format:
					// Unknown file format error
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: Unknown file format: " ) + path_str
					);
					return false;
				default:
					// Other errors
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: " ) + path_str
					);
					return false;
			}
		}

		assert( !face_infos.size() );	// If hit, this function was called twice for some reason, should never happen.
		face_infos.clear();
		face_infos.resize( face_count );
		for( size_t i = 0; i < face_count; ++i ) {
			FT_Face face {};
			{
				auto ft_error = FT_New_Face(
					loader_thread_resource->GetFreeTypeInstance(),
					path_str.c_str(),
					FT_Long( i ),
					&face
				);
				if( ft_error ) {
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: " ) + path_str
					);
					return false;
				}
			}
			{
				auto ft_error = FT_Set_Pixel_Sizes(
					face,
					0,
					glyph_texel_size
				);
				if( ft_error ) {
					instance.Report(
						ReportSeverity::NON_CRITICAL_ERROR,
						std::string( "Cannot load font: " ) + path_str
					);
					return false;
				}
			}

			// Get glyph sizes
			total_glyph_count			+= size_t( face->num_glyphs ) + 1;
			for( decltype( face->num_glyphs ) i = 0; i < face->num_glyphs; ++i ) {

				auto ft_load_error = FT_Load_Glyph( face, FT_UInt( i ), FT_LOAD_DEFAULT );
				if( ft_load_error ) {
					instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot load glyph for bitmap metrics!" );
					return false;
				}

				auto glyph_size =
					glm::dvec2(
						double( face->glyph->metrics.width ),
						double( face->glyph->metrics.height )
					);

				auto glyph_bitmap_size =
					glm::dvec2(
						double( face->glyph->bitmap.width ),
						double( face->glyph->bitmap.rows )
					);

				auto glyph_bitmap_space_occupancy =
					glm::dvec2(
						double( face->glyph->bitmap.width + glyph_atlas_padding ),
						double( face->glyph->bitmap.rows + glyph_atlas_padding )
					);

				maximum_glyph_size.x					= std::max( maximum_glyph_size.x, glyph_size.x );
				maximum_glyph_size.y					= std::max( maximum_glyph_size.y, glyph_size.y );

				maximum_glyph_bitmap_size.x				= std::max( maximum_glyph_bitmap_size.x, glyph_bitmap_size.x );
				maximum_glyph_bitmap_size.y				= std::max( maximum_glyph_bitmap_size.y, glyph_bitmap_size.y );

				maximum_glyph_bitmap_occupancy_size.x	= std::max( maximum_glyph_bitmap_occupancy_size.x, glyph_bitmap_space_occupancy.x );
				maximum_glyph_bitmap_occupancy_size.y	= std::max( maximum_glyph_bitmap_occupancy_size.y, glyph_bitmap_space_occupancy.y );

				average_glyph_bitmap_occupancy_size		+= glyph_bitmap_space_occupancy;
			}

			face_infos[ i ].face	= face;
		}

	} else {
		// Try to load from data.

		assert( 0 && "Not implemented yet!" );
	}

	if( !total_glyph_count ) {
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Cannot load font: Font contains no glyphs!" );
		return false;
	}

	// Estimate appropriate atlas size.
	// This code tries to find a tradeoff between texture size and amount so that 1 to 3 textures are
	// created. For example if glyphs do not fit into 3 textures of size 512 * 512, a larger 1024 * 1024
	// texture is created which should be able to contain the glyphs.
	average_glyph_bitmap_occupancy_size	+= glm::dvec2( double( glyph_atlas_padding ), double( glyph_atlas_padding ) ) * 2.0;
	average_glyph_bitmap_occupancy_size	/= double( total_glyph_count );
	{
		auto estimated_glyph_space_requirements		= average_glyph_bitmap_occupancy_size * ( 1.0 - average_to_max_weight ) + maximum_glyph_bitmap_occupancy_size * average_to_max_weight;
		auto estimated_average_space_requirements	= estimated_glyph_space_requirements / 1.5;	// aim to have 1 to 4 textures to optimize memory usage
//		auto estimated_average_space_requirements	= estimated_glyph_space_requirements / 5.0;	// aim to have 1 to 4 textures to optimize memory usage
		atlas_size = RoundToCeilingPowerOfTwo(
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
		instance.Report(
			ReportSeverity::NON_CRITICAL_ERROR,
			std::string( "Internal error: Cannot load font: " ) + path_str
		);
		return false;
	}

	current_atlas_texture = CreateNewAtlasTexture();

	auto glyph_size_bitmap_size_ratio_vector	= maximum_glyph_bitmap_size / maximum_glyph_size;
	auto glyph_size_bitmap_size_ratio			= std::max( glyph_size_bitmap_size_ratio_vector.x, glyph_size_bitmap_size_ratio_vector.y );

	// Process all glyphs from all font faces
	for( size_t face_index = 0; face_index < face_infos.size(); ++face_index ) {
		auto & face = face_infos[ face_index ];
		face.glyph_infos.resize( face.face->num_glyphs );

		for( auto glyph_index = 0; glyph_index < face.face->num_glyphs; ++glyph_index ) {
			{
				auto ft_load_error = FT_Load_Glyph( face.face, glyph_index, FT_LOAD_DEFAULT );
				if( ft_load_error ) {
					instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot load glyph!" );
					return false;
				}
			}
			{
				auto ft_render_error = FT_Render_Glyph( face.face->glyph, use_alpha ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO );
				if( ft_render_error ) {
					instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, cannot render glyph!" );
					return false;
				}
			}
			auto	ft_glyph	= face.face->glyph;
			auto &	ft_bitmap	= ft_glyph->bitmap;

			std::vector<Color8> final_glyph_pixels( size_t( ft_bitmap.rows ) * size_t( ft_bitmap.width ) );

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

							dst.r				= src[ 2 ];
							dst.g				= src[ 1 ];
							dst.b				= src[ 0 ];
							dst.a				= src[ 3 ];
						}
					}
				}
				break;

				default:
					// Unsupported
					instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot load font, Unsupported pixel format!" );
					return false;
					break;
			}

			// Attach rendered glyph to final texture atlas.
			auto atlas_location = AttachGlyphToAtlas(
				face.face->glyph,
				glyph_atlas_padding,
				final_glyph_pixels
			);
			if( !atlas_location.atlas_ptr ) {
				instance.Report(
					ReportSeverity::NON_CRITICAL_ERROR,
					"Internal error: Cannot create font, cannot copy glyph image to font texture atlas!"
				);
				return false;
			}

			// Create glyph info structure for the glyph
			{
				Rect2f uv_coords = {
					float( atlas_location.location.top_left.x ) / float( atlas_size ),
					float( atlas_location.location.top_left.y ) / float( atlas_size ),
					float( atlas_location.location.bottom_right.x ) / float( atlas_size ),
					float( atlas_location.location.bottom_right.y ) / float( atlas_size )
				};

				auto & metrics								= face.face->glyph->metrics;
				auto glyph_size								= glm::dvec2( metrics.width, metrics.height ) * glyph_size_bitmap_size_ratio;
				auto glyph_hori_top_left					= glm::dvec2( metrics.horiBearingX, -metrics.horiBearingY ) * glyph_size_bitmap_size_ratio;
				auto glyph_hori_bottom_right				= glyph_hori_top_left + glyph_size;
				auto glyph_vert_top_left					= glm::dvec2( metrics.vertBearingX, metrics.vertBearingY ) * glyph_size_bitmap_size_ratio;
				auto glyph_vert_bottom_right				= glyph_vert_top_left + glyph_size;
				auto hori_advance							= metrics.horiAdvance * glyph_size_bitmap_size_ratio;
				auto vert_advance							= metrics.vertAdvance * glyph_size_bitmap_size_ratio;

				GlyphInfo glyph_info {};
				glyph_info.face_index						= uint32_t( face_index );
				glyph_info.atlas_index						= atlas_location.atlas_index;
				glyph_info.uv_coords						= uv_coords;
				glyph_info.horisontal_coords.top_left		= glm::vec2( float( glyph_hori_top_left.x ), float( glyph_hori_top_left.y ) );
				glyph_info.horisontal_coords.bottom_right	= glm::vec2( float( glyph_hori_bottom_right.x ), float( glyph_hori_bottom_right.y ) );
				glyph_info.vertical_coords.top_left			= glm::vec2( float( glyph_vert_top_left.x ), float( glyph_vert_top_left.y ) );
				glyph_info.vertical_coords.bottom_right		= glm::vec2( float( glyph_vert_bottom_right.x ), float( glyph_vert_bottom_right.y ) );
				glyph_info.horisontal_advance				= float( hori_advance );
				glyph_info.vertical_advance					= float( vert_advance );

				face.glyph_infos[ glyph_index ]				= glyph_info;
			}
		}

		// Create character map and get fallback character
		{
			FT_ULong		charcode				= {};
			FT_UInt			gindex					= {};
			FT_ULong		fallback_glyph_index	= {};

			charcode				= FT_Get_First_Char( face.face, &gindex );
			fallback_glyph_index	= gindex;
			while( gindex != 0 ) {
				face.charmap[ char32_t( charcode ) ] = uint32_t( gindex );

				charcode = FT_Get_Next_Char( face.face, charcode, &gindex );
			}

			if( auto f = FT_Get_Char_Index( face.face, FT_ULong( fallback_character ) ) ) {
				fallback_glyph_index	= f;
			}

			face.fallback_glyph_index	= fallback_glyph_index;
		}
	}

	// Destroy font faces, we don't need them anymore.
	for( auto & f : face_infos ) {
		FT_Done_Face( f.face );
		f.face		= nullptr;
	}

	// Everything is baked into the atlas, create texture resource to store it.
	{
		std::vector<const std::vector<Color8>*>		texture_data_array( atlas_textures.size() );
		for( size_t i = 0; i < atlas_textures.size(); ++i ) {
			texture_data_array[ i ]		= &atlas_textures[ i ]->data;
		}

		texture_resource = resource_manager.CreateArrayTextureResource(
			glm::uvec2( atlas_size, atlas_size ),
			texture_data_array,
			&my_interface
		);
		if( !texture_resource ) {
			instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot create texture resource for font!" );
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::FontResourceImpl::MTUnload(
	ThreadPrivateResource * thread_resource
)
{
	// Make sure font faces are destroyed.
	for( auto f : face_infos ) {
		FT_Done_Face( f.face );
	}
	face_infos.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::Rect2f vk2d::vk2d_internal::FontResourceImpl::CalculateRenderedSize(
	std::string_view	text,
	size_t				font_face,
	float				kerning,
	glm::vec2			scale,
	bool				vertical,
	bool				wait_for_resource_load
)
{
	if( std::size( text ) <= 0 ) return {};

	if( wait_for_resource_load ) {
		WaitUntilLoaded( std::chrono::nanoseconds::max() );
	} else {
		if( GetStatus() == ResourceStatus::UNDETERMINED ) return {};
	}
	if( !FaceExists( font_face ) ) return {};

	Rect2f ret {};
	float advance {};
	if( vertical ) {
		// Vertical text
		// First letter sets defaults
		auto first_gi			= GetGlyphInfo( font_face, text[ 0 ] );
		ret.top_left.x			= first_gi.vertical_coords.top_left.x;
		ret.bottom_right.x		= first_gi.vertical_coords.bottom_right.x;
		if( std::size( text ) > 1 ) {
			advance					+= first_gi.vertical_advance;
		}
		for( size_t i = 1; i < std::size( text ) - 1; ++i ) {
			// Middle letters are spaced by "advance" value
			auto gi				= GetGlyphInfo( font_face, text[ i ] );
			ret.top_left.x		= std::min( ret.top_left.x, gi.vertical_coords.top_left.x );
			ret.bottom_right.x	= std::max( ret.bottom_right.x, gi.vertical_coords.bottom_right.x );
			advance				+= kerning + gi.vertical_advance;
		}
		// Need to get the size of the last letter, not the "advance"
		auto last_gi			= GetGlyphInfo( font_face, text.back() );
		ret.top_left.x			= std::min( ret.top_left.x, last_gi.vertical_coords.top_left.x );
		ret.bottom_right.x		= std::max( ret.bottom_right.x, last_gi.vertical_coords.bottom_right.x );
		ret.top_left.y			= first_gi.vertical_coords.top_left.y;
		ret.bottom_right.y		= advance + last_gi.vertical_coords.bottom_right.y;
	} else {
		// Horisontal text
		// First letter sets defaults
		auto first_gi			= GetGlyphInfo( font_face, text[ 0 ] );
		ret.top_left.y			= first_gi.horisontal_coords.top_left.y;
		ret.bottom_right.y		= first_gi.horisontal_coords.bottom_right.y;
		if( std::size( text ) > 1 ) {
			advance				+= first_gi.horisontal_advance;
		}
		for( size_t i = 1; i < std::size( text ) - 1; ++i ) {
			// Middle letters are spaced by "advance" value
			auto gi				= GetGlyphInfo( font_face, text[ i ] );
			ret.top_left.y		= std::min( ret.top_left.y, gi.horisontal_coords.top_left.y );
			ret.bottom_right.y	= std::max( ret.bottom_right.y, gi.horisontal_coords.bottom_right.y );
			advance				+= kerning + gi.horisontal_advance;
		}
		// Need to get the size of the last letter, not the "advance"
		auto last_gi			= GetGlyphInfo( font_face, text.back() );
		ret.top_left.y			= std::min( ret.top_left.y, last_gi.horisontal_coords.top_left.y );
		ret.bottom_right.y		= std::max( ret.bottom_right.y, last_gi.horisontal_coords.bottom_right.y );
		ret.top_left.x			= first_gi.horisontal_coords.top_left.x;
		ret.bottom_right.x		= advance + last_gi.horisontal_coords.bottom_right.x;
	}

	ret.top_left		*= scale;
	ret.bottom_right	*= scale;
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::vk2d_internal::FontResourceImpl::GetFontFaceCount()
{
	return face_infos.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::FontResourceImpl::FaceExists(
	size_t font_face
) const
{
	if( font_face < face_infos.size() ) {
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::TextureResource *vk2d::vk2d_internal::FontResourceImpl::GetTextureResource()
{
	if( GetStatus() == ResourceStatus::LOADED ) {
		return texture_resource;
	}
	return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const vk2d::GlyphInfo & vk2d::vk2d_internal::FontResourceImpl::GetGlyphInfo(
	char32_t	character,
	size_t		font_face
) const
{
	auto & face_info	= face_infos[ font_face ];
	auto & charmap		= face_info.charmap;
	auto glyph_it		= charmap.find( character );
	auto glyph_index	= uint32_t( 0 );
	if( glyph_it != charmap.end() ) {
		glyph_index		= glyph_it->second;
	} else {
		glyph_index		= face_info.fallback_glyph_index;
	}
	return face_info.glyph_infos[ glyph_index ];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vk2d_internal::FontResourceImpl::IsGood() const
{
	return is_good;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::FontResourceImpl::AtlasTexture *vk2d::vk2d_internal::FontResourceImpl::CreateNewAtlasTexture()
{
	auto new_atlas_texture			= std::make_unique<FontResourceImpl::AtlasTexture>();

	new_atlas_texture->data.resize( size_t( atlas_size ) * size_t( atlas_size ) );
	new_atlas_texture->index		= uint32_t( atlas_textures.size() );
	std::memset( new_atlas_texture->data.data(), 0, new_atlas_texture->data.size() * sizeof( Color8 ) );

	auto new_atlas_texture_ptr		= new_atlas_texture.get();
	atlas_textures.push_back( std::move( new_atlas_texture ) );
	return new_atlas_texture_ptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::FontResourceImpl::AtlasLocation vk2d::vk2d_internal::FontResourceImpl::ReserveSpaceForGlyphFromAtlasTextures(
	FT_GlyphSlot		glyph,
	uint32_t			glyph_atlas_padding
)
{
	assert( current_atlas_texture );

	auto FindLocationInAtlasTexture =[](
		FontResourceImpl::AtlasTexture		*	atlas_texture,
		FT_GlyphSlot											glyph,
		uint32_t												atlas_size,
		uint32_t												glyph_atlas_padding
		) ->vk2d::vk2d_internal::FontResourceImpl::AtlasLocation
	{
		uint32_t glyph_width		= uint32_t( glyph->bitmap.width )	+ glyph_atlas_padding;
		uint32_t glyph_height		= uint32_t( glyph->bitmap.rows )	+ glyph_atlas_padding;

		// Find space in the current atlas texture.
		if( atlas_texture->previous_row_height + glyph_height + glyph_atlas_padding < atlas_size ) {
			// Fits height wise.

			if( atlas_texture->current_write_location + glyph_width + glyph_atlas_padding < atlas_size ) {
				// Fits width wise, fits completely.

				FontResourceImpl::AtlasLocation new_glyph_location {};
				new_glyph_location.atlas_ptr				= atlas_texture;
				new_glyph_location.atlas_index				= atlas_texture->index;
				new_glyph_location.location.top_left		= {
					atlas_texture->current_write_location	+ glyph_atlas_padding,
					atlas_texture->previous_row_height		+ glyph_atlas_padding
				};
				new_glyph_location.location.bottom_right	=
					new_glyph_location.location.top_left	+
					glm::uvec2( uint32_t( glyph->bitmap.width ), uint32_t( glyph->bitmap.rows ) );

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
			if( atlas_texture->previous_row_height + glyph_height + glyph_atlas_padding < atlas_size )
			{
				// Fits height wise.

				if( atlas_texture->current_write_location + glyph_width + glyph_atlas_padding < atlas_size )
				{
					// Fits width wise.

					FontResourceImpl::AtlasLocation new_glyph_location {};
					new_glyph_location.atlas_ptr				= atlas_texture;
					new_glyph_location.atlas_index				= atlas_texture->index;
					new_glyph_location.location.top_left		= {
						atlas_texture->current_write_location	+ glyph_atlas_padding,
						atlas_texture->previous_row_height		+ glyph_atlas_padding
					};
					new_glyph_location.location.bottom_right	=
						new_glyph_location.location.top_left	+
						glm::uvec2( uint32_t( glyph->bitmap.width ), uint32_t( glyph->bitmap.rows ) );

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
			resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot create new atlas texture for font!" );
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
			resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, a single glyph wont fit into a new atlas." );
			return {};
		}

		return new_location;
	}
	return new_location;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vk2d_internal::FontResourceImpl::CopyGlyphTextureToAtlasLocation(
	AtlasLocation						atlas_location,
	const std::vector<Color8>		&	converted_texture_data
)
{
	assert( atlas_location.atlas_ptr );

	auto glyph_width		= atlas_location.location.bottom_right.x - atlas_location.location.top_left.x;
	auto glyph_height		= atlas_location.location.bottom_right.y - atlas_location.location.top_left.y;
	glm::uvec2 location		= atlas_location.location.top_left;

	for( uint32_t gy = 0; gy < glyph_height; ++gy )
	{
		for( uint32_t gx = 0; gx < glyph_width; ++gx )
		{
			auto ax		= location.x + gx;
			auto ay		= location.y + gy;
			atlas_location.atlas_ptr->data[ ay * atlas_size + ax ]	= converted_texture_data[ gy * glyph_width + gx ];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vk2d_internal::FontResourceImpl::AtlasLocation vk2d::vk2d_internal::FontResourceImpl::AttachGlyphToAtlas(
	FT_GlyphSlot							glyph,
	uint32_t								glyph_atlas_padding,
	const std::vector<Color8>			&	converted_texture_data
)
{
	auto atlas_location = ReserveSpaceForGlyphFromAtlasTextures(
		glyph,
		glyph_atlas_padding
	);
	if( atlas_location.atlas_ptr )
	{
		CopyGlyphTextureToAtlasLocation(
			atlas_location,
			converted_texture_data
		);
		return atlas_location;
	}
	else
	{
		resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font, cannot attach glyph to atlas texture!" );
		return {};
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t vk2d::vk2d_internal::RoundToCeilingPowerOfTwo( uint32_t value )
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
