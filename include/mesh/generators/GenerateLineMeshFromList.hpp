#pragma once

#include <core/Common.hpp>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>
#include "GeneratePointMeshFromList.hpp"



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate line mesh from points and connections.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	points
///				A list of coordinates representing end points of lines.
/// 
/// @param[in]	indices
///				Indices defining which points should be connected with a line.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateLineMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_2>			&	indices
)
{
	auto aabb = vk2d_internal::CalculateAABBFromPointList( points );
	auto aabb_origin = aabb.top_left;
	auto aabb_size = aabb.bottom_right - aabb_origin;

	auto mesh = GeneratePointMeshFromList<VertexT>( points );
	mesh.indices.resize( indices.size() * 2 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 2 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
	}

	mesh.generated_mesh_type	= MeshType::LINE;
	mesh.mesh_type				= mesh.generated_mesh_type;

	return mesh;
}



} // mesh_generators
} // vk2d
