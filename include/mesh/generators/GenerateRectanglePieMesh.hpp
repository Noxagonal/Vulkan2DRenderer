#pragma once

#include <core/Common.h>

#include "MeshGeneratorDeclarations.hpp"

#include <mesh/Mesh.hpp>
#include <mesh/vertex/StandardVertex.hpp>
#include <mesh/vertex/VertexTools.hpp>
#include "GenerateRectangleMesh.hpp"



namespace vk2d {
namespace mesh_generators {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a rectangular pie, similar to drawing a rectangle but which has a pie slice cut out.
/// 
/// @tparam		VertexT
///				Vertex type that describes vertex properties.
/// @warning	Custom shader interface must have exactly the same vertex parameters as the vertex.
/// 
/// @param[in]	area
///				Area of the rectangle. See RenderCoordinateSpace for more info about what scale is used.
///
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// 
/// @param[in]	coverage 
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full rectangle. Moving value
///				from 0 to 1 makes "whole" pie visible in clockwise direction.
/// 
/// @param[in]	filled
///				true to draw the inside of the pie rectangle, false to draw the outline only.
/// 
/// @return		A new mesh object.
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateRectanglePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled
)
{
	if( coverage >= 1.0f ) {
		return GenerateRectangleMesh( area, filled );
	}
	if( coverage <= 0.0f ) {
		return {};		// Nothing to draw
	}

	auto mesh = Mesh<VertexT>();

	glm::vec2 center_point = {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x = area.bottom_right.x - center_point.x;
	float center_to_edge_y = area.bottom_right.y - center_point.y;

	float width = std::abs( area.bottom_right.x - area.top_left.x );
	float height = std::abs( area.bottom_right.y - area.top_left.y );

	auto end_angle_radians = float( RAD ) * coverage;
	end_angle_radians += begin_angle_radians;
	auto area_angle = end_angle_radians - begin_angle_radians;

	enum AllPointsIndex : uint32_t
	{
		BOX_CORNER_0,
		BOX_CORNER_1,
		BOX_CORNER_2,
		BOX_CORNER_3,
		PIE_BEGIN,
		PIE_END,
	};
	std::array<glm::vec2, 6>	unordered_outer_points { {
		{ area.top_left.x,		area.top_left.y		},
		{ area.bottom_right.x,	area.top_left.y		},
		{ area.bottom_right.x,	area.bottom_right.y	},
		{ area.top_left.x,		area.bottom_right.y	}
	} };

	// Get pie begin and end coordinates
	{
		// Specialized version of Ray to AABB intersection test that
		// always intersects and returns the exit coordinates only.
		auto RayExitIntersection = [](
			glm::vec2 box_coords_1,
			glm::vec2 box_coords_2,
			glm::vec2 box_center,		// ray origin, and box center are both at the same coordinates
			glm::vec2 ray_end
			) -> glm::vec2
		{
			glm::vec2	ray_begin = box_center;
			float		clipped_exit = 1.0;

			// Clipping on x and y axis
			float dim_low_x = ( box_coords_1.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_low_y = ( box_coords_1.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );
			float dim_high_x = ( box_coords_2.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_high_y = ( box_coords_2.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );

			dim_high_x = std::max( dim_low_x, dim_high_x );
			dim_high_y = std::max( dim_low_y, dim_high_y );
			clipped_exit = std::min( dim_high_y, dim_high_x );

			glm::vec2 collider_localized = ray_end - ray_begin;
			glm::vec2 localized_intersection_point = collider_localized * clipped_exit;
			glm::vec2 globalized_intersection_point = localized_intersection_point + box_center;
			return globalized_intersection_point;
		};

		float ray_lenght = center_to_edge_x + center_to_edge_y;
		glm::vec2 ray_angle_1_end = { std::cos( begin_angle_radians ) * ray_lenght, std::sin( begin_angle_radians ) * ray_lenght };
		glm::vec2 ray_angle_2_end = { std::cos( end_angle_radians ) * ray_lenght, std::sin( end_angle_radians ) * ray_lenght };
		ray_angle_1_end += center_point;
		ray_angle_2_end += center_point;

		unordered_outer_points[ PIE_BEGIN ] = RayExitIntersection(
			area.top_left,
			area.bottom_right,
			center_point,
			ray_angle_1_end
		);
		unordered_outer_points[ PIE_END ] = RayExitIntersection(
			area.top_left,
			area.bottom_right,
			center_point,
			ray_angle_2_end
		);
	}

	auto IsOnSameXAxis = [](
		glm::vec2			point_1,
		glm::vec2			point_2
		) -> bool
	{
		if( point_1.y < point_2.y + KINDA_SMALL_VALUE &&
			point_1.y > point_2.y - KINDA_SMALL_VALUE ) {
			return true;
		}
		return false;
	};

	auto IsOnSameYAxis = [](
		glm::vec2			point_1,
		glm::vec2			point_2
	) -> bool
	{
		if( point_1.x < point_2.x + KINDA_SMALL_VALUE &&
			point_1.x > point_2.x - KINDA_SMALL_VALUE ) {
			return true;
		}
		return false;
	};

	// Generate an ordered point list
	std::vector<glm::vec2> outer_point_list;
	{
		// We'll linearize / unwrap the box so that a single number represents both x and y coordinates
		//
		// 0---1
		// |   |	->	0---1---2---3---0
		// 3---2

		struct LinearPoint
		{
			uint32_t		original_linear_point_index = {};
			float			linear_coords = {};
			glm::vec2		actual_coords = {};
		};
		float distance_counter = 0.0f;
		std::array<LinearPoint, 6>	linear_points { {
			{ BOX_CORNER_0, 0.0f,							unordered_outer_points[ BOX_CORNER_0 ] },
			{ BOX_CORNER_1, distance_counter += width,		unordered_outer_points[ BOX_CORNER_1 ] },
			{ BOX_CORNER_2, distance_counter += height,		unordered_outer_points[ BOX_CORNER_2 ] },
			{ BOX_CORNER_3, distance_counter += width,		unordered_outer_points[ BOX_CORNER_3 ] }
		} };

		auto ResolveLinearPoint = [
			width,
			height,
			IsOnSameXAxis,
			IsOnSameYAxis,
			&unordered_outer_points,
			&linear_points
		](
			uint32_t		index
		)
		{
			// Find which side of the box the point is located at
			glm::vec2 actual_coords = unordered_outer_points[ index ];

			if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
				// Top
				float linear = ( actual_coords.x - linear_points[ 0 ].actual_coords.x ) + linear_points[ 0 ].linear_coords;
				linear_points[ index ] = { index, linear, actual_coords };
			}
			else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 1 ] ) ) {
			 // Right
				float linear = ( actual_coords.y - linear_points[ 1 ].actual_coords.y ) + linear_points[ 1 ].linear_coords;
				linear_points[ index ] = { index, linear, actual_coords };
			}
			else if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 3 ] ) ) {
			 // Bottom
				float linear = ( linear_points[ 2 ].actual_coords.x - actual_coords.x ) + linear_points[ 2 ].linear_coords;
				linear_points[ index ] = { index, linear, actual_coords };
			}
			else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
			 // Left
				float linear = ( linear_points[ 3 ].actual_coords.y - actual_coords.y ) + linear_points[ 3 ].linear_coords;
				linear_points[ index ] = { index, linear, actual_coords };
			}
			else {
				assert( 0 );
			}
		};

		ResolveLinearPoint( PIE_BEGIN );
		ResolveLinearPoint( PIE_END );

		std::sort(
			linear_points.begin(),
			linear_points.end(),
			[]( LinearPoint & p1, LinearPoint & p2 )
			{
				return p1.linear_coords < p2.linear_coords;
			} );

		// We have a list of points in a winding order.
		// Now we need to figure out which points are
		// within and collect a list from those.

		// Find current index for pie begin and end in linear_points array
		uint32_t	linear_resolved_pie_begin_point = UINT32_MAX;
		uint32_t	linear_resolved_pie_end_point = UINT32_MAX;
		for( size_t i = 0; i < linear_points.size(); ++i ) {
			if( linear_points[ i ].original_linear_point_index == PIE_BEGIN )	linear_resolved_pie_begin_point = uint32_t( i );
			if( linear_points[ i ].original_linear_point_index == PIE_END )		linear_resolved_pie_end_point = uint32_t( i );
		}
		assert( linear_resolved_pie_begin_point != UINT32_MAX );
		assert( linear_resolved_pie_end_point != UINT32_MAX );

		// Sort() organized everything for us, now we just need to
		// collect all in-between coordinates from the indices
		outer_point_list.reserve( 6 );
		uint32_t current_linear_point_index = linear_resolved_pie_begin_point;
		while( current_linear_point_index != linear_resolved_pie_end_point ) {
			outer_point_list.push_back( linear_points[ current_linear_point_index ].actual_coords );
			if( ( ++current_linear_point_index ) == 6 ) current_linear_point_index = 0;
		}
		outer_point_list.push_back( linear_points[ linear_resolved_pie_end_point ].actual_coords );
	}

	// Create vertices
	{
		mesh.vertices.reserve( 7 );

		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords = center_point;
		v.uv_coords = { 0.5f, 0.5f };
		mesh.vertices.push_back( v );

		for( auto opl : outer_point_list ) {
			v.vertex_coords = opl;
			v.uv_coords = ( opl - unordered_outer_points[ 0 ] ) / glm::vec2( width, height );
			mesh.vertices.push_back( v );
		}
	}

	if( filled ) {
		mesh.indices.reserve( mesh.vertices.size() * 3 );
		for( size_t i = 2; i < mesh.vertices.size(); ++i ) {
			mesh.indices.push_back( 0 );
			mesh.indices.push_back( uint32_t( i ) - 1 );
			mesh.indices.push_back( uint32_t( i ) );
		}

		mesh.generated_mesh_type = MeshType::TRIANGLE_FILLED;
	}
	else {
		mesh.indices.reserve( mesh.vertices.size() * 2 + 2 );
		for( size_t i = 1; i < mesh.vertices.size(); ++i ) {
			mesh.indices.push_back( uint32_t( i ) - 1 );
			mesh.indices.push_back( uint32_t( i ) );
		}
		mesh.indices.push_back( uint32_t( mesh.vertices.size() - 1 ) );
		mesh.indices.push_back( 0 );

		mesh.generated_mesh_type = MeshType::LINE;
	}

	mesh.generated = true;
	mesh.mesh_type = mesh.generated_mesh_type;
	mesh_modifiers::RecalculateAABBFromVertices( mesh );

	return mesh;
}



} // mesh_generators
} // vk2d
