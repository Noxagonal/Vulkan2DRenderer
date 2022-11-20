#pragma once

#include <core/Common.h>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate rectangular mesh from rectangle coordinates.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	area
///				Area of the rectangle that will be covered, depends on the coordinate system. See RenderCoordinateSpace
///				for more info about what scale is used.
/// 
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateRectangleMesh(
	Rect2f											area,
	bool											filled
)
{
	auto mesh = Mesh<VertexT>();

	mesh.vertices.resize( 4 );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	// 0. Top left
	mesh.vertices[ 0 ].vertex_coords = { area.top_left.x, area.top_left.y };
	mesh.vertices[ 0 ].uv_coords = { 0.0f, 0.0f };

	// 1. Top right
	mesh.vertices[ 1 ].vertex_coords = { area.bottom_right.x, area.top_left.y };
	mesh.vertices[ 1 ].uv_coords = { 1.0f, 0.0f };

	// 2. Bottom left
	mesh.vertices[ 2 ].vertex_coords = { area.top_left.x, area.bottom_right.y };
	mesh.vertices[ 2 ].uv_coords = { 0.0f, 1.0f };

	// 3. Bottom right
	mesh.vertices[ 3 ].vertex_coords = { area.bottom_right.x, area.bottom_right.y };
	mesh.vertices[ 3 ].uv_coords = { 1.0f, 1.0f };

	if( filled ) {
		// Draw filled polygons
		mesh.indices.resize( 2 * 3 );
		mesh.indices[ 0 ] = 0;
		mesh.indices[ 1 ] = 2;
		mesh.indices[ 2 ] = 1;
		mesh.indices[ 3 ] = 1;
		mesh.indices[ 4 ] = 2;
		mesh.indices[ 5 ] = 3;
		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else {
	 // Draw lines
		mesh.indices.resize( 4 * 2 );
		mesh.indices[ 0 ] = 0;
		mesh.indices[ 1 ] = 2;
		mesh.indices[ 2 ] = 2;
		mesh.indices[ 3 ] = 3;
		mesh.indices[ 4 ] = 3;
		mesh.indices[ 5 ] = 1;
		mesh.indices[ 6 ] = 1;
		mesh.indices[ 7 ] = 0;
		mesh.generated_mesh_type = MeshType::LINE;
	}

	mesh.generated = true;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh.aabb = area;

	return mesh;
}



} // vk2d
