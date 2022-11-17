#pragma once

#include <core/Common.h>

#include "MeshBase.hpp"
#include "vertex/StandardVertex.hpp"



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Collection of vertices and their relations.
///
///				A mesh represents a collection of vertices and their relations forming a shape that can be drawn onto a surface.
/// 
///				- A mesh can be created using any of the mesh generator functions, or constructing it manually.
///				- A mesh can be modified using any of the modifier functions or by manually modifying the vertices and indices.
///
/// @tparam		VertexT
///				Vertex type to be used with the mesh.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
template<typename VertexT = StandardVertex>
class Mesh : public MeshBase
{
	static_assert(
		vk2d_internal::VertexBaseType<VertexT> || vk2d_internal::VertexBaseDerivedType<VertexT>,
		"Vertex type must be VertexBase or derived from it"
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Vertices list.
	///
	/// @see		Vertex
	std::vector<VertexT>							vertices						= {};
};



} // vk2d
