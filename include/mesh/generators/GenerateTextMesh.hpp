#pragma once

#include <core/Common.hpp>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>
#include <interface/resources/font/FontResource.hpp>



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a text mesh that can be used to render text.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	font
///				A pointer to font resource to use.
/// 
/// @param[in]	origin
///				Left or top coordinate of what is considered the starting point for the text.
///				- For horisontal text origin is left side beginning of text and vertical origin is the baseline.
///				- For vertical text origin is top beginning of text and horisontal offset is middle of the symbols.
/// 
/// @param[in]	text
///				Text characters to generate.
/// 
/// @param[in]	kerning
///				Spacing between letters. Positive values are farther apart. Value is based on size of the font texel size.
/// 
/// @param[in]	scale
///				Scale of the mesh to be generated. Value {1.0, 1.0} will map 1:1 to the font size when render target texture or
///				window coordinate space is either RenderCoordinateSpace::TEXEL_SPACE or
///				RenderCoordinateSpace::TEXEL_SPACE_CENTERED.
/// 
/// @param[in]	vertical
///				true if text is generated vertically, false if horisontally.
/// 
/// @param[in]	font_face
///				Certain fonts may contain multiple font faces, this allows you to select which one to use.
/// 
/// @param[in]	wait_for_resource_load
///				When generating text mesh you usually want it to be available right away. Setting this option to true will wait
///				for the font resource to load before allowing execution to continue. If you're in a time critical situation
///				where you are generating the text every frame and you cannot afford a few millisecond wait for the font to load
///				up you can set this value to false. <br>
///				This function cannot generate text until the font has been loaded so setting this to false will return an empty
///				mesh until the font has been fully loaded by the resource manager.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT> &&
	vk2d_internal::VertexHasSingleTextureLayer<VertexT>
)
Mesh<VertexT>										GenerateTextMesh(
	FontResource								*	font,
	glm::vec2										origin,
	std::string										text,
	float											kerning,
	glm::vec2										scale,
	bool											vertical,
	uint32_t										font_face,
	bool											wait_for_resource_load
)
{
	auto mesh = Mesh<VertexT>();

	if( std::size( text ) <= 0 ) return;
	if( !font ) return;
	if( wait_for_resource_load )
	{
		font->WaitUntilLoaded( std::chrono::nanoseconds::max() );
	}
	else
	{
		if( font->GetStatus() == ResourceStatus::UNDETERMINED ) return;
	}
	if( !font->FaceExists( font_face ) ) return;

	mesh.vertices.reserve( text.size() * 4 );
	mesh.indices.reserve( text.size() * 6 );

	auto AppendBox = [ &mesh, scale ](
		const glm::vec2		&	location,
		const Rect2f		&	coords,
		const Rect2f		&	uv_coords,
		uint32_t				texture_channel
		)
	{
		auto vertex_offset = mesh.vertices.size();
		auto index_offset = mesh.indices.size();

		auto tcoords = coords;
		tcoords.top_left *= scale;
		tcoords.bottom_right *= scale;
		tcoords += location;

		mesh.aabb.top_left.x = std::min( mesh.aabb.top_left.x, tcoords.top_left.x );
		mesh.aabb.top_left.y = std::min( mesh.aabb.top_left.y, tcoords.top_left.y );
		mesh.aabb.bottom_right.x = std::max( mesh.aabb.bottom_right.x, tcoords.bottom_right.x );
		mesh.aabb.bottom_right.y = std::max( mesh.aabb.bottom_right.y, tcoords.bottom_right.y );

		mesh.vertices.resize( vertex_offset + 4, vk2d_internal::CreateDefaultValueVertex() );
		mesh.vertices[ vertex_offset + 0 ].vertex_coords = glm::vec2( tcoords.top_left.x, tcoords.top_left.y );
		mesh.vertices[ vertex_offset + 0 ].uv_coords = glm::vec2( uv_coords.top_left.x, uv_coords.top_left.y );
		mesh.vertices[ vertex_offset + 0 ].single_texture_layer = texture_channel;

		mesh.vertices[ vertex_offset + 1 ].vertex_coords = glm::vec2( tcoords.bottom_right.x, tcoords.top_left.y );
		mesh.vertices[ vertex_offset + 1 ].uv_coords = glm::vec2( uv_coords.bottom_right.x, uv_coords.top_left.y );
		mesh.vertices[ vertex_offset + 1 ].single_texture_layer = texture_channel;

		mesh.vertices[ vertex_offset + 2 ].vertex_coords = glm::vec2( tcoords.top_left.x, tcoords.bottom_right.y );
		mesh.vertices[ vertex_offset + 2 ].uv_coords = glm::vec2( uv_coords.top_left.x, uv_coords.bottom_right.y );
		mesh.vertices[ vertex_offset + 2 ].single_texture_layer = texture_channel;

		mesh.vertices[ vertex_offset + 3 ].vertex_coords = glm::vec2( tcoords.bottom_right.x, tcoords.bottom_right.y );
		mesh.vertices[ vertex_offset + 3 ].uv_coords = glm::vec2( uv_coords.bottom_right.x, uv_coords.bottom_right.y );
		mesh.vertices[ vertex_offset + 3 ].single_texture_layer = texture_channel;

		mesh.indices.resize( index_offset + 6 );
		mesh.indices[ index_offset + 0 ] = uint32_t( vertex_offset + 0 );
		mesh.indices[ index_offset + 1 ] = uint32_t( vertex_offset + 2 );
		mesh.indices[ index_offset + 2 ] = uint32_t( vertex_offset + 1 );
		mesh.indices[ index_offset + 3 ] = uint32_t( vertex_offset + 1 );
		mesh.indices[ index_offset + 4 ] = uint32_t( vertex_offset + 2 );
		mesh.indices[ index_offset + 5 ] = uint32_t( vertex_offset + 3 );
	};

	auto location = origin;
	if( vertical ) {
		// Writing vertical text
		{
			auto & gi = font->GetGlyphInfo( font_face, text[ 0 ] );
			mesh.aabb.top_left = gi.vertical_coords.top_left * scale + location;
			mesh.aabb.bottom_right = gi.vertical_coords.bottom_right * scale + location;
		}
		for( auto c : text ) {
			auto & gi = font->GetGlyphInfo( font_face, c );
			AppendBox( location, gi.vertical_coords, gi.uv_coords, gi.atlas_index );
			location.y += ( gi.vertical_advance + kerning ) * scale.y;
		}
	}
	else
	{
	 // Writing horisontal text
		{
			auto & gi = font->GetGlyphInfo( font_face, text[ 0 ] );
			mesh.aabb.top_left = gi.horisontal_coords.top_left * scale + location;
			mesh.aabb.bottom_right = gi.horisontal_coords.bottom_right * scale + location;
		}
		for( auto c : text ) {
			auto & gi = font->GetGlyphInfo( font_face, c );
			AppendBox( location, gi.horisontal_coords, gi.uv_coords, gi.atlas_index );
			location.x += ( gi.horisontal_advance + kerning ) * scale.x;
		}
	}

	mesh.texture = font->GetTextureResource();

	return mesh;
}



} // mesh_generators
} // vk2d
