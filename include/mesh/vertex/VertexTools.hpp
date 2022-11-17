#pragma once

#include <core/Common.h>

#include "StandardVertex.hpp"

#include <containers/Rect2.hpp>

#include <vector>



namespace vk2d {
namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
Rect2Base<T> CalculateAABBFromPointList(
	const std::vector<glm::vec<2, T, glm::packed_highp>>		&	points
)
{
	Rect2Base<T> ret { points[ 0 ], points[ 0 ] };
	for( auto p : points )
	{
		ret.top_left.x		= std::min( ret.top_left.x, p.x );
		ret.top_left.y		= std::min( ret.top_left.y, p.y );
		ret.bottom_right.x	= std::max( ret.bottom_right.x, p.x );
		ret.bottom_right.y	= std::max( ret.bottom_right.y, p.y );
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<VertexBaseDerivedType VertexT = StandardVertex>
requires( VertexHasVertexCoords<VertexT> )
Rect2f CalculateAABBFromVertexList(
	const std::vector<VertexT> & vertices
)
{
	Rect2f ret { vertices[ 0 ].vertex_coords, vertices[ 0 ].vertex_coords };
	for( auto p : vertices )
	{
		ret.top_left.x		= std::min( ret.top_left.x, p.vertex_coords.x );
		ret.top_left.y		= std::min( ret.top_left.y, p.vertex_coords.y );
		ret.bottom_right.x	= std::max( ret.bottom_right.x, p.vertex_coords.x );
		ret.bottom_right.y	= std::max( ret.bottom_right.y, p.vertex_coords.y );
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename VertexT = StandardVertex>
requires( VertexBaseDerivedType<VertexT> || VertexBaseType<VertexT> )
VertexT CreateDefaultValueVertex()
{
	auto v = VertexT();
	if constexpr( VertexHasColor<VertexT> )
	{
		v.color			= { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	if constexpr( VertexHasPointSize<VertexT> )
	{
		v.point_size	= 1.0f;
	}
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename VertexT = StandardVertex>
requires( VertexBaseDerivedType<VertexT> || VertexBaseType<VertexT> )
void ClearVerticesToDefaultValues(
	std::vector<VertexT> & vertices
)
{
	for( auto & v : vertices )
	{
		v = CreateDefaultValueVertex();
	}
}



} // vk2d_internal
} // vk2d
