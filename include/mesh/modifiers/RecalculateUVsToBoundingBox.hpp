#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Recalculates new UV locations for each vertex to be within the bounding box.
///
///				This makes the uv coordinates for each vertex to be inside of the UV map.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
void						RecalculateUVsToBoundingBox(
	Mesh<VertexT>		&	mesh
)
{
	auto size = aabb.bottom_right - aabb.top_left;
	for( auto & v : vertices ) {
		auto vp = v.vertex_coords - aabb.top_left;
		v.uv_coords = vp / size;
	}
}



} // mesh_modifiers
} // vk2d
