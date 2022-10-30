
#include "core/SourceCommon.h"

#include "MeshCommon.hpp"



vk2d::Rect2f vk2d::vk2d_internal::CalculateAABBFromVertexList(
	const std::vector<Vertex>			&	vertices
)
{
	Rect2f ret { vertices[ 0 ].vertex_coords, vertices[ 0 ].vertex_coords };
	for( auto p : vertices ) {
		ret.top_left.x		= std::min( ret.top_left.x, p.vertex_coords.x );
		ret.top_left.y		= std::min( ret.top_left.y, p.vertex_coords.y );
		ret.bottom_right.x	= std::max( ret.bottom_right.x, p.vertex_coords.x );
		ret.bottom_right.y	= std::max( ret.bottom_right.y, p.vertex_coords.y );
	}
	return ret;
}

vk2d::Vertex vk2d::vk2d_internal::CreateDefaultValueVertex()
{
	Vertex v;
	v.vertex_coords				= {};
	v.uv_coords					= {};
	v.color						= { 1.0f, 1.0f, 1.0f, 1.0f };
	v.point_size				= 1.0f;
	v.single_texture_layer	= 0;
	return v;
}

void vk2d::vk2d_internal::ClearVerticesToDefaultValues(
	std::vector<Vertex>				&	vertices
)
{
	for( auto & v : vertices ) {
		v = CreateDefaultValueVertex();
	}
}
