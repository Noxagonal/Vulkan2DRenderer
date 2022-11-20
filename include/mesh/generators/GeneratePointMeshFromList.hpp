#pragma once

#include <core/Common.h>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate point mesh from point list.
///
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	points
///				A list of coordinates representing point locations.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GeneratePointMeshFromList(
	const std::vector<glm::vec2>				&	points
)
{
	auto mesh = Mesh<VertexT>();
	auto aabb = vk2d_internal::CalculateAABBFromPointList( points );
	auto aabb_origin = aabb.top_left;
	auto aabb_size = aabb.bottom_right - aabb_origin;

	mesh.vertices.resize( points.size() );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );
	for( size_t i = 0; i < points.size(); ++i )
	{
		mesh.vertices[ i ].vertex_coords = points[ i ];
		mesh.vertices[ i ].uv_coords = ( points[ i ] - aabb_origin ) / aabb_size;
	}

	mesh.generated = true;
	mesh.generated_mesh_type = MeshType::POINT;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh.aabb = aabb;

	return mesh;
}



} // mesh_generators
} // vk2d
