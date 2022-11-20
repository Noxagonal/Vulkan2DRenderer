#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Skew UV coordinates on all vertices around origin.
///
/// @note		Scewing UVs works similarly to scewing vertices except for UVs. This means everything appears the opposite of
///				the actual effect.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	skew_amount
///				Positive values scew UVs on bottom right of origin to be moved towards bottom right and vertices top left of
///				origin to be moved towards top left corner. <br>
///				Amount is a half of the other dimension. This way the total angle is 45 degrees when scewing single axis by 1.0.
/// 
/// @param[in]	origin
///				Origin point of which everything is scewed in relation to.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasUVCoords<VertexT> )
void						SkewUV(
	Mesh<VertexT>		&	mesh,
	glm::vec2				skew_amount,
	glm::vec2				origin
)
{
	for( auto & i : mesh.vertices ) {
		auto c = i.uv_coords - origin;
		i.uv_coords.x = c.y * skew_amount.x + c.x;
		i.uv_coords.y = c.x * skew_amount.y + c.y;
		i.uv_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
