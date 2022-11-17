#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>

#include <containers/Transform.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Rotate the mesh around a pivot point.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	rotation_amount
///				Rotation amount in radians, positive direction is clockwise.
/// 
/// @param[in]	origin
///				The pivot point of which the mesh vertices are rotated around.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						Rotate(
	Mesh<VertexT>		&	mesh,
	float					rotation_amount,
	glm::vec2				origin
)
{
	auto rotation_matrix = CreateRotationMatrix2( rotation_amount );

	for( auto & i : mesh.vertices ) {
		i.vertex_coords -= origin;
		i.vertex_coords = rotation_matrix * i.vertex_coords;
		i.vertex_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
