#pragma once

#include <core/Common.h>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a lattice mesh, this is useful for distortions.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	area
///				Area of the rectangle. See RenderCoordinateSpace for more info about what scale is used.
/// 
/// @param[in]	subdivisions
///				Number of inside subdivisions, eg. 2*2 will generate lattice with 4*4 rectangles. If fractional values are used
///				then inside subdivisions are moved inward from the bottom right direction, this is to prevent popping in case
///				this value is animated.
/// 
/// @param[in]	filled
///				true if the inside is filled, false to generate a lattice line mesh.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateLatticeMesh(
	Rect2f											area,
	glm::vec2										subdivisions,
	bool											filled
)
{
	auto mesh = Mesh<VertexT>();

	uint32_t vertex_count_x = uint32_t( std::ceil( subdivisions.x ) ) + 2;
	uint32_t vertex_count_y = uint32_t( std::ceil( subdivisions.y ) ) + 2;
	uint32_t total_vertex_count = vertex_count_x * vertex_count_y;

	glm::vec2 mesh_size = area.bottom_right - area.top_left;
	glm::vec2 vertex_spacing = mesh_size / ( subdivisions + glm::vec2( 1.0f, 1.0f ) );
	glm::vec2 uv_spacing = glm::vec2( 1.0f, 1.0f ) / ( subdivisions + glm::vec2( 1.0f, 1.0f ) );

	mesh.vertices.resize( total_vertex_count );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	for( size_t y = 0; y < vertex_count_y - 1; ++y ) {
		for( size_t x = 0; x < vertex_count_x - 1; ++x ) {
			auto & v = mesh.vertices[ y * vertex_count_x + x ];
			v.vertex_coords = { vertex_spacing.x * x + area.top_left.x, vertex_spacing.y * y + area.top_left.y };
			v.uv_coords = { uv_spacing.x * x, uv_spacing.y * y };
		}
		auto & v = mesh.vertices[ y * vertex_count_x + vertex_count_x - 1 ];
		v.vertex_coords = { area.bottom_right.x, vertex_spacing.y * y + area.top_left.y };
		v.uv_coords = { 1.0f, uv_spacing.y * y };
	}
	for( size_t x = 0; x < vertex_count_x - 1; ++x ) {
		auto & v = mesh.vertices[ size_t( vertex_count_y - 1 ) * vertex_count_x + x ];
		v.vertex_coords = { vertex_spacing.x * x + area.top_left.x, area.bottom_right.y };
		v.uv_coords = { uv_spacing.x * x, 1.0f };
	}
	auto & v = mesh.vertices[ size_t( vertex_count_y - 1 ) * vertex_count_x + vertex_count_x - 1 ];
	v.vertex_coords = { area.bottom_right.x, area.bottom_right.y };
	v.uv_coords = { 1.0f, 1.0f };

	if( filled ) {
		// Draw solid mesh with polygons.

		auto reserved_size = ( vertex_count_x - 1 ) * ( vertex_count_y - 1 ) * 2 * 3;
		mesh.indices.reserve( reserved_size );
		for( size_t y = 1; y < vertex_count_y; ++y ) {
			for( size_t x = 1; x < vertex_count_x; ++x ) {
				auto tl = uint32_t( ( y - 1 ) * vertex_count_x + x - 1 );	// top left vertex.
				auto tr = uint32_t( ( y - 1 ) * vertex_count_x + x );		// top right vertex.
				auto bl = uint32_t( ( y - 0 ) * vertex_count_x + x - 1 );	// bottom left vertex.
				auto br = uint32_t( ( y - 0 ) * vertex_count_x + x );		// bottom right vertex.
				mesh.indices.push_back( tl );	// First triangle.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( tr );
				mesh.indices.push_back( tr );	// Second triangle.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( br );
			}
		}

		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else
	{
		// Draw lattice lines.

		auto reserved_size = ( vertex_count_x - 1 ) * ( vertex_count_y - 1 ) * 4 * 2;
		mesh.indices.reserve( reserved_size );
		for( size_t y = 1; y < vertex_count_y; ++y ) {
			for( size_t x = 1; x < vertex_count_x; ++x ) {
				auto tl = uint32_t( ( y - 1 ) * vertex_count_x + x - 1 );	// top left vertex.
				auto tr = uint32_t( ( y - 1 ) * vertex_count_x + x );		// top right vertex.
				auto bl = uint32_t( ( y - 0 ) * vertex_count_x + x - 1 );	// bottom left vertex.
				auto br = uint32_t( ( y - 0 ) * vertex_count_x + x );		// bottom right vertex.
				mesh.indices.push_back( tl );	// First line.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( bl );	// Second line.
				mesh.indices.push_back( br );
				mesh.indices.push_back( br );	// Third line.
				mesh.indices.push_back( tr );
				mesh.indices.push_back( tr );	// Fourth line.
				mesh.indices.push_back( tl );
			}
		}

		mesh.generated_mesh_type = MeshType::LINE;
	}

	mesh.generated = true;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh.RecalculateAABBFromVertices();

	return mesh;
}



} // vk2d
