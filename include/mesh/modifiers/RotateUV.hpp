#pragma once

#include <core/Common.hpp>

#include <mesh/Mesh.hpp>

#include <containers/Transform.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Rotate UV coordinates on all vertices around origin.
///
/// @note		This directly scales the UVs so texture rotation will appear inverted.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	rotation_amount
///				Rotation amount in radians, positive values will turn the UVs counter clockwise and the texture will appear to
///				rotate counter-clockwise.
/// 
/// @param[in]	origin
///				The pivot point of which the mesh UVs are rotated around. Please note that UV coordinates are always in range of
///				0.0 to 1.0 so origin outside this range will appear to both translate and rotate.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasUVCoords<VertexT> )
void						RotateUV(
	Mesh<VertexT>		&	mesh,
	float					rotation_amount,
	glm::vec2				origin
)
{
	auto rotation_matrix = CreateRotationMatrix2( rotation_amount );

	for( auto & i : mesh.vertices ) {
		i.uv_coords -= origin;
		i.uv_coords = rotation_matrix * i.uv_coords;
		i.uv_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
