#pragma once

#include <core/Common.hpp>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Moves all the vertices closer or father from the origin point.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	scaling_amount
///				0.0 scales everything into a single point, 1.0 does nothing, 2.0 doubles the current size of the mesh.
/// 
/// @param[in]	origin
///				Origin point of which everything is scaled towards or away from.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						Scale(
	Mesh<VertexT>		&	mesh,
	glm::vec2				scaling_amount,
	glm::vec2				origin
)
{
	for( auto & i : mesh.vertices ) {
		i.vertex_coords -= origin;
		i.vertex_coords *= scaling_amount;
		i.vertex_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
