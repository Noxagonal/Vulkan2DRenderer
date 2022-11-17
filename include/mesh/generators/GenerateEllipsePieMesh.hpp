#pragma once

#include <core/Common.h>
#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate an ellipse or a circle that has a "slice" cut out, similar to usual graphs.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
///				be used.
/// 
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// 
/// @param[in]	coverage
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full ellipse. Moving value
///				from 0 to 1 makes "whole" pie visible in clockwise direction.
/// 
/// @param[in]	filled
///				true to draw the inside of the pie, false to draw the outline only.
/// 
/// @param[in]	edge_count 
///				How many corners the complete ellipse should have, or quality if you prefer. This is a float value for
///				"smoother" transitions between amount of corners, in case this value is animated.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateEllipsePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled					= true,
	float											edge_count				= 64.0f
)
{
	auto mesh = Mesh<VertexT>();

	if( edge_count < 3.0f )			edge_count = 3.0f;
	if( coverage > 1.0f )			coverage = 1.0f;
	if( coverage <= 0.0f )			return;			// Nothing to draw

	glm::vec2 center_point = {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x = area.bottom_right.x - center_point.x;
	float center_to_edge_y = area.bottom_right.y - center_point.y;

	float rotation_step_size = float( RAD / edge_count );

	uint32_t edge_count_ceil = uint32_t( std::ceil( edge_count ) );

	{
		auto angle_clamp = float( std::floor( begin_angle_radians / RAD ) );
		begin_angle_radians = begin_angle_radians - float( angle_clamp * RAD );
		assert( begin_angle_radians <= RAD );
	}
	auto end_angle_radians = float( RAD ) * coverage;
	end_angle_radians += begin_angle_radians;
	auto area_angle = end_angle_radians - begin_angle_radians;

	mesh.vertices.reserve( size_t( edge_count_ceil ) + 2 );
	// Center vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords = center_point;
		v.uv_coords = { 0.5f, 0.5f };
		mesh.vertices.push_back( v );
	}

	auto intermediate_point_begin = uint32_t( begin_angle_radians / rotation_step_size );
	auto intermediate_point_end = uint32_t( end_angle_radians / rotation_step_size );
	auto intermediate_point_count = intermediate_point_end - intermediate_point_begin;

	// Start vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords = {
			std::cos( begin_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( begin_angle_radians ) * center_to_edge_y + center_point.y
		};
		v.uv_coords = {
			std::cos( begin_angle_radians ) * 0.5f + 0.5f,
			std::sin( begin_angle_radians ) * 0.5f + 0.5f
		};
		mesh.vertices.push_back( v );
	}

	// Intermediate vertices.
	{
		// First half, from begin angle to 0 angle
		for( uint32_t i = intermediate_point_begin + 1; i < intermediate_point_end + 1; ++i ) {
			if( double( rotation_step_size ) * i > RAD ) break;

			auto v = vk2d_internal::CreateDefaultValueVertex();
			v.vertex_coords = {
				std::cos( rotation_step_size * i ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step_size * i ) * center_to_edge_y + center_point.y
			};
			v.uv_coords = {
				std::cos( rotation_step_size * i ) * 0.5f + 0.5f,
				std::sin( rotation_step_size * i ) * 0.5f + 0.5f
			};
			mesh.vertices.push_back( v );
		}

		// Second half, from 0 angle to end angle
		float rotation_step = 0.0f;
		while( rotation_step < end_angle_radians - RAD ) {

			auto v = vk2d_internal::CreateDefaultValueVertex();
			v.vertex_coords = {
				std::cos( rotation_step ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step ) * center_to_edge_y + center_point.y
			};
			v.uv_coords = {
				std::cos( rotation_step ) * 0.5f + 0.5f,
				std::sin( rotation_step ) * 0.5f + 0.5f
			};
			mesh.vertices.push_back( v );

			rotation_step += rotation_step_size;
		}
	}

	// End vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords = {
			std::cos( end_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( end_angle_radians ) * center_to_edge_y + center_point.y
		};
		v.uv_coords = {
			std::cos( end_angle_radians ) * 0.5f + 0.5f,
			std::sin( end_angle_radians ) * 0.5f + 0.5f
		};
		mesh.vertices.push_back( v );
	}

	if( filled ) {
		// Draw filled polygons
		mesh.indices.resize( ( mesh.vertices.size() - 2 ) * 3 );
		{
			for( size_t i = 2, a = 0; i < mesh.vertices.size(); ++i, a += 3 ) {
				mesh.indices[ a + 0 ] = 0;
				mesh.indices[ a + 1 ] = uint32_t( i ) - 1;
				mesh.indices[ a + 2 ] = uint32_t( i );
			}
		}
		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else {
	 // Draw lines
		mesh.indices.resize( mesh.vertices.size() * 2 );
		for( size_t i = 1, a = 0; i < mesh.vertices.size(); ++i, a += 2 ) {
			mesh.indices[ a + 0 ] = uint32_t( i ) - 1;
			mesh.indices[ a + 1 ] = uint32_t( i );
		}
		mesh.indices[ mesh.indices.size() - 2 ] = uint32_t( mesh.vertices.size() - 1 );
		mesh.indices[ mesh.indices.size() - 1 ] = 0;

		mesh.generated_mesh_type = MeshType::LINE;
	}

	mesh.generated = true;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh.RecalculateAABBFromVertices();

	return mesh;
}



} // vk2d
