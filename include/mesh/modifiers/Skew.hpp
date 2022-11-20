#pragma once

#include <core/Common.hpp>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Scews the mesh in vertical and horisontal directions based on origin point.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
///
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	scew_amount
///				Positive values scew vertices on bottom right of origin to be moved towards bottom right and vertices top left
///				of origin to be moved towards top left corner. <br>
///				Amount is a half of the other dimension. This way if you have a cube for example, setting amount to {0.0, 1.0}
///				will result the left side of the cube to be offsetted 0.5 of the total height upwards and the right side of the
///				cube to be offsetted 0.5 of the total height downwards. This way the total angle is 45 degrees when scewing
///				single axis by 1.0.
/// 
/// @param[in]	origin
///				Origin point of which everything is scewed in relation to.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						Skew(
	Mesh<VertexT>		&	mesh,
	glm::vec2				scew_amount,
	glm::vec2				origin
)
{
	for( auto & i : mesh.vertices ) {
		auto c = i.vertex_coords - origin;
		i.vertex_coords.x = c.y * scew_amount.x + c.x;
		i.vertex_coords.y = c.x * scew_amount.y + c.y;
		i.vertex_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
