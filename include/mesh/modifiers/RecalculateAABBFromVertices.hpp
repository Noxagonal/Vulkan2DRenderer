#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>

#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {
namespace mesh_modifiers {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Recalculates Axis Aligned Bounding Box from vertex coordinates.
/// 
///				Updates Mesh::aabb variable and returns reference to it.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						RecalculateAABBFromVertices(
	Mesh<VertexT>		&	mesh
)
{
	if( std::size( mesh.vertices ) > 0 ) {
		mesh.aabb = vk2d_internal::CalculateAABBFromVertexList( mesh.vertices );
	}
	else {
		mesh.aabb = {};
	}
}



} // mesh_modifiers
} // vk2d
