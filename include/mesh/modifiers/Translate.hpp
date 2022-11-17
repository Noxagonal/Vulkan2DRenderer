#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Moves all the vertices in some direction.
///
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	movement
///				A math vector that will be added to each vertex position.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						Translate(
	Mesh<VertexT>		&	mesh,
	const glm::vec2			movement
)
{
	for( auto & i : mesh.vertices ) {
		i.vertex_coords += movement;
	}
}



} // mesh_modifiers
} // vk2d
