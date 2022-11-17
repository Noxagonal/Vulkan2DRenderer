#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Set all vertices to a specific color.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	new_color
///				Color to apply to all vertices.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasColor<VertexT> )
void						SetVerticesColor(
	Mesh<VertexT>		&	mesh,
	Colorf					new_color
)
{
	for( auto & v : mesh.vertices ) {
		v.color = new_color;
	}
}



} // mesh_modifiers
} // vk2d
