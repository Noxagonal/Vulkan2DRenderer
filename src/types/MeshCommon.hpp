#pragma once

#include <core/SourceCommon.h>

#include <types/MeshPrimitives.hpp>
#include <types/Rect2.hpp>

#include <vector>



namespace vk2d {
namespace vk2d_internal {



template<typename T>
Rect2Base<T> CalculateAABBFromPointList(
	const std::vector<glm::vec<2, T, glm::packed_highp>>		&	points
)
{
	Rect2Base<T> ret { points[ 0 ], points[ 0 ] };
	for( auto p : points ) {
		ret.top_left.x		= std::min( ret.top_left.x, p.x );
		ret.top_left.y		= std::min( ret.top_left.y, p.y );
		ret.bottom_right.x	= std::max( ret.bottom_right.x, p.x );
		ret.bottom_right.y	= std::max( ret.bottom_right.y, p.y );
	}
	return ret;
}

Rect2f CalculateAABBFromVertexList(
	const std::vector<Vertex>			&	vertices
);

Vertex CreateDefaultValueVertex();

void ClearVerticesToDefaultValues(
	std::vector<Vertex>				&	vertices
);



} // vk2d_internal
} // vk2d
