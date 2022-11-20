#pragma once

#include <core/Common.h>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate triangle mesh from points and connections.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	points
///				A list of coordinates representing corners of triangles.
/// 
/// @param[in]	indices
///				Indices defining which points should form a triangle.
/// 
/// @param[in]	filled
///				true if triangle mesh is filled, false for wireframe.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateTriangleMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_3>			&	indices,
	bool											filled
)
{
	auto mesh = GeneratePointMeshFromList<VertexT>( points );
	mesh.indices.resize( indices.size() * 3 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 3 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
		mesh.indices[ d + 2 ] = indices[ i ].indices[ 2 ];
	}

	if( filled ) {
		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else {
		mesh.generated_mesh_type = MeshType::TRIANGLE_WIREFRAME;
	}
	mesh.mesh_type = mesh.generated_mesh_type;

	return mesh;
}



} // mesh_generators
} // vk2d
