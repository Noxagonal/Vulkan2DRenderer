#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Set size of a single vertex when rendering point meshes.
///
///				If this mesh is rendered as a list of points we can define how large those points will be. This is a per vertex
///				feature so you can manually set the size per vertex, this just sets all vertices to be the same size. This uses
///				a GPU feature and is expected to perform well even when rendering lots of points, however it can be low quality.
///				For more control over the quality you should render rectangles or circles instead.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	point_size
///				Texel size of the point to be rendered.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasPointSize<VertexT> )
void						SetVerticesPointSize(
	Mesh<VertexT>		&	mesh,
	float					point_size
)
{
	for( auto & v : mesh.vertices ) {
		v.point_size = point_size;
	}
}



} // mesh_modifiers
} // vk2d
