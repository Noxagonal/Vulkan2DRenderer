#pragma once

#include <core/Common.hpp>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>
#include <mesh/modifiers/RecalculateAABBFromVertices.hpp>



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate ellipse mesh from rectangle area and edge count.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
///				used.
/// 
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// 
/// @param[in]	edge_count
///				Number of outer edges, this is a floating point value to prevent popping in case it's animated.
///
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateEllipseMesh(
	Rect2f											area,
	bool											filled,
	float											edge_count
)
{
	auto mesh = Mesh<VertexT>();

	if( edge_count < 3.0f ) edge_count = 3.0f;

	glm::vec2 center_point = {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x = area.bottom_right.x - center_point.x;
	float center_to_edge_y = area.bottom_right.y - center_point.y;

	float rotation_step = 0.0f;
	float rotation_step_size = float( RAD / edge_count );

	uint32_t edge_count_ceil = uint32_t( std::ceil( edge_count ) );

	mesh.vertices.resize( edge_count_ceil );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	for( uint32_t i = 0; i < edge_count_ceil; ++i ) {
		mesh.vertices[ i ].vertex_coords = {
			std::cos( rotation_step ) * center_to_edge_x + center_point.x,
			std::sin( rotation_step ) * center_to_edge_y + center_point.y
		};
		mesh.vertices[ i ].uv_coords = {
			std::cos( rotation_step ) * 0.5f + 0.5f,
			std::sin( rotation_step ) * 0.5f + 0.5f
		};
		rotation_step += rotation_step_size;
	}

	if( filled ) {
		// Draw filled polygons
		mesh.indices.resize( size_t( edge_count_ceil - 2 ) * 3 );
		{
			for( uint32_t i = 2, a = 0; i < edge_count_ceil; ++i, a += 3 ) {
				assert( i < edge_count_ceil );

				mesh.indices[ size_t( a ) + 0 ] = 0;
				mesh.indices[ size_t( a ) + 1 ] = i - 1;
				mesh.indices[ size_t( a ) + 2 ] = i;
			}
		}
		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else {
		// Draw lines
		mesh.indices.resize( size_t( edge_count_ceil ) * 2 );
		{
			for( uint32_t i = 1, a = 0; i < edge_count_ceil; ++i, a += 2 ) {
				assert( i < edge_count_ceil );

				mesh.indices[ size_t( a ) + 0 ] = i - 1;
				mesh.indices[ size_t( a ) + 1 ] = i;
			}
			mesh.indices[ size_t( edge_count_ceil ) * 2LL - 2 ] = edge_count_ceil - 1;
			mesh.indices[ size_t( edge_count_ceil ) * 2LL - 1 ] = 0;
		}
		mesh.generated_mesh_type = MeshType::LINE;
	}

	mesh.generated = true;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh_modifiers::RecalculateAABBFromVertices( mesh );

	return mesh;
}



} // mesh_generators
} // vk2d
