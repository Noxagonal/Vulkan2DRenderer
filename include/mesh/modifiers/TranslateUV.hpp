#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Moves vertices UV coordinates to some direction.
///
/// @note		This directly moves the UVs to some direction so texture movement will appear exact opposite of moving vertices.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	movement
///				Direction to move the UVs towards. Note that this is in UV coordinates which are always in range of 0.0 to 1.0,
///				in case a UV coordinate goes outside this range, a Sampler will determine what to do with it.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasUVCoords<VertexT> )
void						TranslateUV(
	Mesh<VertexT>		&	mesh,
	const glm::vec2			movement
)
{
	for( auto & i : mesh.vertices ) {
		i.uv_coords += movement;
	}
}



} // mesh_modifiers
} // vk2d
