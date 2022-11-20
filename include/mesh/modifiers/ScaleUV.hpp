#pragma once

#include <core/Common.hpp>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Scale UV coordinates on all vertices around origin.
///
/// @note		Scaling UVs will work exactly the same as to scaling vertices except for UV coordinates, so scaling everything
///				to 0.0 will make texture to be sampled from a single point, making texture appear infinitely large.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	scaling_amount
///				Scale amount, 0.0 will scale all UVs into a single point, 1.0 does nothing, 2.0 doubles the UV scale.
/// 
/// @param[in]	origin
///				Origin point of which everything is scaled towards or away from.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasUVCoords<VertexT> )
void						ScaleUV(
	Mesh<VertexT>		&	mesh,
	glm::vec2				scaling_amount,
	glm::vec2				origin
)
{
	for( auto & i : mesh.vertices ) {
		i.uv_coords -= origin;
		i.uv_coords *= scaling_amount;
		i.uv_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
