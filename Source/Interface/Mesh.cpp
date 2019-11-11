
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Mesh.h"

#include <float.h>



vk2d::AABB2d CalculateAABBFromPointList(
	const std::vector<vk2d::Vector2d>		&	points
)
{
	vk2d::AABB2d ret { points[ 0 ], points[ 0 ] };
	for( auto p : points ) {
		ret.top_left.x		= std::min( ret.top_left.x, p.x );
		ret.top_left.y		= std::min( ret.top_left.y, p.y );
		ret.bottom_right.x	= std::max( ret.bottom_right.x, p.x );
		ret.bottom_right.y	= std::max( ret.bottom_right.y, p.y );
	}
	return ret;
}



VK2D_API vk2d::Mesh VK2D_APIENTRY vk2d::GeneratePointMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points
)
{
	auto aabb			= CalculateAABBFromPointList( points );
	auto aabb_origin	= aabb.top_left;
	auto aabb_size		= aabb.bottom_right - aabb_origin;

	Mesh mesh;
	mesh.vertices.resize( points.size() );
	for( size_t i = 0; i < points.size(); ++i ) {
		mesh.vertices[ i ].vertex_coords	= points[ i ];
		mesh.vertices[ i ].uv_coords		= ( points[ i ] - aabb_origin ) / aabb_size;
		mesh.vertices[ i ].color			= { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	mesh.mesh_type			= vk2d::MeshType::POINT;
	return mesh;
}

VK2D_API vk2d::Mesh VK2D_APIENTRY vk2d::GenerateLineMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points,
	const std::vector<vk2d::VertexIndex_2>	&	indices
)
{
	auto mesh = GeneratePointMeshFromList( points );
	mesh.indices.resize( indices.size() * 2 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 2 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
	}

	mesh.mesh_type			= vk2d::MeshType::LINE;
	return mesh;
}

VK2D_API vk2d::Mesh VK2D_APIENTRY vk2d::GenerateTriangleMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points,
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	bool										filled
)
{
	auto mesh = GeneratePointMeshFromList( points );
	mesh.indices.resize( indices.size() * 3 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 3 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
		mesh.indices[ d + 2 ] = indices[ i ].indices[ 2 ];
	}

	if( filled ) {
		mesh.mesh_type		= vk2d::MeshType::TRIANGLE_FILLED;
	} else {
		mesh.mesh_type		= vk2d::MeshType::TRIANGLE_WIREFRAME;
	}
	return mesh;
}


/*
void WindowImpl::Draw_Line(
	Vector2d						point_1,
	Vector2d						point_2,
	Color							color )
{
	std::vector<Vertex>				vertices( 2 );
	std::vector<VertexIndex_2>		indices( 1 );

	vertices[ 0 ].vertex_coords		= point_1;
	vertices[ 0 ].uv_coords			= {};
	vertices[ 0 ].color				= color;

	vertices[ 1 ].vertex_coords		= point_2;
	vertices[ 1 ].uv_coords			= {};
	vertices[ 1 ].color				= color;

	indices[ 0 ].indices			= { 0, 1 };

	Draw_LineList(
		vertices,
		indices
	);
}

void WindowImpl::Draw_Box(
	bool							filled,
	Vector2d						top_left,
	Vector2d						bottom_right,
	Color							color )
{
	std::vector<Vertex>				vertices( 4 );

	// 0. Top left
	vertices[ 0 ].vertex_coords		= { top_left.x,			top_left.y };
	vertices[ 0 ].uv_coords			= { 0.0f, 0.0f };
	vertices[ 0 ].color				= color;

	// 1. Top right
	vertices[ 1 ].vertex_coords		= { bottom_right.x,		top_left.y };
	vertices[ 1 ].uv_coords			= { 1.0f, 0.0f };
	vertices[ 1 ].color				= color;

	// 2. Bottom left
	vertices[ 2 ].vertex_coords		= { top_left.x,			bottom_right.y };
	vertices[ 2 ].uv_coords			= { 0.0f, 1.0f };
	vertices[ 2 ].color				= color;

	// 3. Bottom right
	vertices[ 3 ].vertex_coords		= { bottom_right.x,		bottom_right.y };
	vertices[ 3 ].uv_coords			= { 1.0f, 1.0f };
	vertices[ 3 ].color				= color;

	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3>	indices( 2 );
		indices[ 0 ].indices		= { 0, 2, 1 };
		indices[ 1 ].indices		= { 1, 2, 3 };
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2>	indices( 4 );
		indices[ 0 ].indices		= { 0, 2 };
		indices[ 1 ].indices		= { 2, 3 };
		indices[ 2 ].indices		= { 3, 1 };
		indices[ 3 ].indices		= { 1, 0 };
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_Circle(
	bool							filled,
	Vector2d						top_left,
	Vector2d						bottom_right,
	float							edge_count,
	Color							color
)
{
	if( edge_count < 3.0f ) edge_count = 3.0f;

	Vector2d center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float rotation_step			= 0.0f;
	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( edge_count );
	if( edge_count - uint32_t( edge_count ) ) {
		++edge_count_integer;
	}

	std::vector<Vertex> vertices( edge_count_integer );
	for( uint32_t i = 0; i < edge_count_integer; ++i ) {
		vertices[ i ].vertex_coords		= {
			std::cos( rotation_step ) * center_to_edge_x + center_point.x,
			std::sin( rotation_step ) * center_to_edge_y + center_point.y
		};
		vertices[ i ].uv_coords			= {
			std::cos( rotation_step ),
			std::sin( rotation_step )
		};
		vertices[ i ].color				= color;
		rotation_step					+= rotation_step_size;
	}

	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3> indices( size_t( edge_count_integer ) + 1 );
		{
			uint32_t i = 0;
			for( i = 2; i < edge_count_integer; ++i ) {
				assert( i < edge_count_integer );
				indices[ i ] = { 0, i - 1, i };
			}
		}
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2> indices( size_t( edge_count_integer ) + 1 );
		{
			uint32_t i = 0;
			for( i = 1; i < edge_count_integer; ++i ) {
				assert( i < edge_count_integer );
				indices[ i ] = { i - 1, i };
			}
			indices[ edge_count_integer ] = { edge_count_integer - 1, 0 };
		}
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_Pie(
	bool							filled,
	Vector2d						top_left,
	Vector2d						bottom_right,
	float							begin_angle_radians,
	float							coverage,
	float							edge_count,
	Color							color
)
{
	if( edge_count < 3.0f )			edge_count		= 3.0f;
	if( coverage > 1.0f )			coverage		= 1.0f;
	if( coverage <= 0.0f )			return;			// Nothing to draw

	Vector2d center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float rotation_step			= 0.0f;
	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( edge_count );
	if( edge_count - uint32_t( edge_count ) ) {
		++edge_count_integer;
	}

	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

	std::vector<Vertex> vertices; vertices.reserve( size_t( edge_count_integer ) + 2 );
	vertices.push_back( {
		center_point,
		{ 0.5f, 0.5f },
		color }
	);
	while( true ) {
		if( rotation_step >= area_angle ) break;
		vertices.push_back( { {
				std::cos( rotation_step + begin_angle_radians ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step + begin_angle_radians ) * center_to_edge_y + center_point.y
			}, {
				std::cos( rotation_step ),
				std::sin( rotation_step )
			},
			color }
		);
		rotation_step					+= rotation_step_size;
	}
	vertices.push_back( { {
			std::cos( end_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( end_angle_radians ) * center_to_edge_y + center_point.y
		}, {
			std::cos( end_angle_radians ),
			std::sin( end_angle_radians )
		},
		color }
	);


	if( filled ) {
		// Draw filled polygons
		std::vector<VertexIndex_3> indices( vertices.size() + 1 );
		{
			uint32_t i = 0;
			for( i = 2; i < indices.size() - 1; ++i ) {
				//assert( i < edge_count_integer );
				indices[ i ] = { 0, i - 1, i };
			}
		}
		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		// Draw lines
		std::vector<VertexIndex_2> indices; indices.reserve( vertices.size() + 2 );
		{
			if( coverage < 1.0f )		indices.push_back( { 0, 1 } );
			uint32_t i = 0;
			for( i = 2; i < uint32_t( vertices.size() ); ++i ) {
				indices.push_back( { i - 1, i } );
			}
			if( coverage < 1.0f )		indices.push_back( { uint32_t( vertices.size() - 1 ), 0 } );
		}
		Draw_LineList(
			vertices,
			indices
		);
	}
}

void WindowImpl::Draw_PieBox(
	bool							filled,
	Vector2d						top_left,
	Vector2d						bottom_right,
	float							begin_angle_radians,
	float							coverage,
	Color							color
)
{
	if( coverage >= 1.0f ) {
		Draw_Box( filled, top_left, bottom_right, color );
		return;		// Done
	}
	if( coverage <= 0.0f ) {
		return;		// Nothing to draw
	}

	Vector2d center_point					= {
		( top_left.x + bottom_right.x ) / 2.0f,
		( top_left.y + bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= bottom_right.x - center_point.x;
	float center_to_edge_y		= bottom_right.y - center_point.y;

	float width					= std::abs( bottom_right.x - top_left.x );
	float height				= std::abs( bottom_right.y - top_left.y );

	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

	enum AllPointsIndex : uint32_t {
		BOX_CORNER_0,
		BOX_CORNER_1,
		BOX_CORNER_2,
		BOX_CORNER_3,
		PIE_BEGIN,
		PIE_END,
	};
	std::array<Vector2d, 6>	unordered_outer_points { {
		{ top_left.x,		top_left.y		},
		{ bottom_right.x,	top_left.y		},
		{ bottom_right.x,	bottom_right.y	},
		{ top_left.x,		bottom_right.y	}
	} };

	// Get pie begin and end coordinates
	{
		// Specialized version of Ray to AABB intersecion test that
		// always intersects and returns the exit coordinates only.
		auto RayExitIntersection	=[](
			Vector2d box_coords_1,
			Vector2d box_coords_2,
			Vector2d box_center,		// ray origin, and box center are both at the same coordinates
			Vector2d ray_end
			) -> Vector2d
		{
			Vector2d		ray_begin			= box_center;
			float		clipped_exit		= 1.0;

			// Clipping on x and y axis
			float dim_low_x		= ( box_coords_1.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_low_y		= ( box_coords_1.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );
			float dim_high_x	= ( box_coords_2.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_high_y	= ( box_coords_2.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );

			dim_high_x			= std::max( dim_low_x, dim_high_x );
			dim_high_y			= std::max( dim_low_y, dim_high_y );
			clipped_exit		= std::min( dim_high_y, dim_high_x );

			Vector2d collider_localized				= ray_end - ray_begin;
			Vector2d localized_intersection_point		= collider_localized * clipped_exit;
			Vector2d globalized_intersection_point	= localized_intersection_point + box_center;
			return globalized_intersection_point;
		};

		float ray_lenght		= center_to_edge_x + center_to_edge_y;
		Vector2d ray_angle_1_end	= { std::cos( begin_angle_radians ) * ray_lenght, std::sin( begin_angle_radians ) * ray_lenght };
		Vector2d ray_angle_2_end	= { std::cos( end_angle_radians ) * ray_lenght, std::sin( end_angle_radians ) * ray_lenght };
		ray_angle_1_end			+= center_point;
		ray_angle_2_end			+= center_point;

		unordered_outer_points[ PIE_BEGIN ]	= RayExitIntersection(
			top_left,
			bottom_right,
			center_point,
			ray_angle_1_end
		);
		unordered_outer_points[ PIE_END ]		= RayExitIntersection(
			top_left,
			bottom_right,
			center_point,
			ray_angle_2_end
		);
	}

	auto IsOnSameXAxis	= [](
		Vector2d			point_1,
		Vector2d			point_2
		) -> bool
	{
		if( point_1.y < point_2.y + 0.0001f &&
			point_1.y > point_2.y - 0.0001f ) {
			return true;
		}
		return false;
	};

	auto IsOnSameYAxis	= [](
		Vector2d			point_1,
		Vector2d			point_2
		) -> bool
	{
		if( point_1.x < point_2.x + 0.0001f &&
			point_1.x > point_2.x - 0.0001f ) {
			return true;
		}
		return false;
	};

	// Generate an ordered point list
	std::vector<Vector2d> outer_point_list;
	{
		// We'll linearize / unwrap the box so that a single number represents x/y coordinates
		//
		// 0---1
		// |   |	->	0---1---2---3---0
		// 3---2

		struct LinearPoint {
			uint32_t	original_linear_point_index;
			float		linear_coords;
			Vector2d		actual_coords;
		};
		float distance_counter = 0.0f;
		std::array<LinearPoint, 6>	linear_points { {
			{ BOX_CORNER_0, 0.0f,							unordered_outer_points[ BOX_CORNER_0 ] },
			{ BOX_CORNER_1, distance_counter += width,		unordered_outer_points[ BOX_CORNER_1 ] },
			{ BOX_CORNER_2, distance_counter += height,		unordered_outer_points[ BOX_CORNER_2 ] },
			{ BOX_CORNER_3, distance_counter += width,		unordered_outer_points[ BOX_CORNER_3 ] }
		} };

		auto ResolveLinearPoint	=[
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
			Vector2d actual_coords	= unordered_outer_points[ index ];

			if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
				// Top
				float linear		= ( actual_coords.x - linear_points[ 0 ].actual_coords.x ) + linear_points[ 0 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 1 ] ) ) {
				// Right
				float linear		= ( actual_coords.y - linear_points[ 1 ].actual_coords.y ) + linear_points[ 1 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameXAxis( actual_coords, unordered_outer_points[ 3 ] ) ) {
				// Bottom
				float linear		= ( linear_points[ 2 ].actual_coords.x - actual_coords.x ) + linear_points[ 2 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else if( IsOnSameYAxis( actual_coords, unordered_outer_points[ 0 ] ) ) {
				// Left
				float linear		= ( linear_points[ 3 ].actual_coords.y - actual_coords.y ) + linear_points[ 3 ].linear_coords;
				linear_points[ index ]	= { index, linear, actual_coords };
			} else {
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
			uint32_t	linear_resolved_pie_begin_point		= UINT32_MAX;
			uint32_t	linear_resolved_pie_end_point		= UINT32_MAX;
			for( size_t i = 0; i < linear_points.size(); ++i ) {
				if( linear_points[ i ].original_linear_point_index == PIE_BEGIN )	linear_resolved_pie_begin_point	= uint32_t( i );
				if( linear_points[ i ].original_linear_point_index == PIE_END )		linear_resolved_pie_end_point	= uint32_t( i );
			}
			assert( linear_resolved_pie_begin_point != UINT32_MAX );
			assert( linear_resolved_pie_end_point != UINT32_MAX );

			// Sort() organized everything for us, now we just need to
			// collect all in-between coordinates from the indices
			outer_point_list.reserve( 6 );
			uint32_t current_linear_point_index		= linear_resolved_pie_begin_point;
			while( current_linear_point_index != linear_resolved_pie_end_point ) {
				outer_point_list.push_back( linear_points[ current_linear_point_index ].actual_coords );
				if( ( ++current_linear_point_index ) == 6 ) current_linear_point_index = 0;
			}
			outer_point_list.push_back( linear_points[ linear_resolved_pie_end_point ].actual_coords );
	}

	// Create vertices
	std::vector<Vertex> vertices;
	{
		vertices.reserve( 7 );

		vertices.push_back( {
			center_point,
			{ 0.5f, 0.5f },
			color
			} );
		for( auto opl : outer_point_list ) {
			vertices.push_back( {
				opl,
				( opl - unordered_outer_points[ 0 ] ) / Vector2d( width, height ),
				color
				} );
		}
	}

	if( filled ) {
		std::vector<VertexIndex_3> indices;		indices.reserve( vertices.size() );
		for( uint32_t i = 2; i < uint32_t( vertices.size() ); ++i ) {
			indices.push_back( { 0, i - 1, i } );
		}

		Draw_TriangleList(
			true,
			vertices,
			indices
		);
	} else {
		std::vector<VertexIndex_2> indices;		indices.reserve( vertices.size() );
		for( uint32_t i = 1; i < uint32_t( vertices.size() ); ++i ) {
			indices.push_back( { i - 1, i } );
		}
		indices.push_back( { uint32_t( vertices.size() - 1 ), 0 } );
		Draw_LineList(
			vertices,
			indices
		);
	}
}

*/
