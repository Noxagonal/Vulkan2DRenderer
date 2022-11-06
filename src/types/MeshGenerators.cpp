
#include <core/SourceCommon.h>

#include <types/MeshGenerators.hpp>
#include <types/Mesh.h>

#include <types/MeshCommon.hpp>

#include <interface/resources/FontResource.h>
#include <interface/resources/FontResourceImpl.h>



VK2D_API void vk2d::vk2d_internal::GeneratePointMeshFromListImpl(
	MeshBase							&	mesh,
	const std::vector<glm::vec2>		&	points
)
{
	auto aabb			= vk2d_internal::CalculateAABBFromPointList( points );
	auto aabb_origin	= aabb.top_left;
	auto aabb_size		= aabb.bottom_right - aabb_origin;

	mesh.vertices.resize( points.size() );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );
	for( size_t i = 0; i < points.size(); ++i ) {
		mesh.vertices[ i ].vertex_coords	= points[ i ];
		mesh.vertices[ i ].uv_coords		= ( points[ i ] - aabb_origin ) / aabb_size;
	}

	mesh.generated				= true;
	mesh.generated_mesh_type	= MeshType::POINT;
	mesh.aabb					= aabb;
	mesh.SetMeshType( mesh.generated_mesh_type );
}

VK2D_API void vk2d::vk2d_internal::GenerateLineMeshFromListImpl(
	MeshBase							&	mesh,
	const std::vector<glm::vec2>		&	points,
	const std::vector<VertexIndex_2>	&	indices
)
{
	GeneratePointMeshFromListImpl( mesh, points );
	mesh.indices.resize( indices.size() * 2 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 2 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
	}

	mesh.generated				= true;
	mesh.generated_mesh_type	= MeshType::LINE;
	mesh.SetMeshType( mesh.generated_mesh_type );
}

VK2D_API void vk2d::vk2d_internal::GenerateTriangleMeshFromListImpl(
	MeshBase							&	mesh,
	const std::vector<glm::vec2>		&	points,
	const std::vector<VertexIndex_3>	&	indices,
	bool									filled
)
{
	GeneratePointMeshFromListImpl( mesh, points );
	mesh.indices.resize( indices.size() * 3 );
	for( size_t i = 0, d = 0; i < indices.size(); ++i, d += 3 ) {
		mesh.indices[ d + 0 ] = indices[ i ].indices[ 0 ];
		mesh.indices[ d + 1 ] = indices[ i ].indices[ 1 ];
		mesh.indices[ d + 2 ] = indices[ i ].indices[ 2 ];
	}

	if( filled ) {
		mesh.generated_mesh_type	= MeshType::TRIANGLE_FILLED;
	} else {
		mesh.generated_mesh_type	= MeshType::TRIANGLE_WIREFRAME;
	}
	mesh.generated				= true;
	mesh.SetMeshType( mesh.generated_mesh_type );
}

VK2D_API void vk2d::vk2d_internal::GenerateRectangleMeshImpl(
	MeshBase							&	mesh,
	Rect2f									area,
	bool									filled
)
{
	mesh.vertices.resize( 4 );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	// 0. Top left
	mesh.vertices[ 0 ].vertex_coords		= { area.top_left.x, area.top_left.y };
	mesh.vertices[ 0 ].uv_coords			= { 0.0f, 0.0f };

	// 1. Top right
	mesh.vertices[ 1 ].vertex_coords		= { area.bottom_right.x, area.top_left.y };
	mesh.vertices[ 1 ].uv_coords			= { 1.0f, 0.0f };

	// 2. Bottom left
	mesh.vertices[ 2 ].vertex_coords		= { area.top_left.x, area.bottom_right.y };
	mesh.vertices[ 2 ].uv_coords			= { 0.0f, 1.0f };

	// 3. Bottom right
	mesh.vertices[ 3 ].vertex_coords		= { area.bottom_right.x, area.bottom_right.y };
	mesh.vertices[ 3 ].uv_coords			= { 1.0f, 1.0f };

	if( filled ) {
		// Draw filled polygons
		mesh.indices.resize( 2 * 3 );
		mesh.indices[ 0 ]	= 0;
		mesh.indices[ 1 ]	= 2;
		mesh.indices[ 2 ]	= 1;
		mesh.indices[ 3 ]	= 1;
		mesh.indices[ 4 ]	= 2;
		mesh.indices[ 5 ]	= 3;
		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::TRIANGLE_FILLED;
		mesh.SetMeshType( mesh.generated_mesh_type );
	} else {
		// Draw lines
		mesh.indices.resize( 4 * 2 );
		mesh.indices[ 0 ]	= 0;
		mesh.indices[ 1 ]	= 2;
		mesh.indices[ 2 ]	= 2;
		mesh.indices[ 3 ]	= 3;
		mesh.indices[ 4 ]	= 3;
		mesh.indices[ 5 ]	= 1;
		mesh.indices[ 6 ]	= 1;
		mesh.indices[ 7 ]	= 0;
		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::LINE;
		mesh.SetMeshType( mesh.generated_mesh_type );
	}

	mesh.aabb = area;
}

VK2D_API void vk2d::vk2d_internal::GenerateEllipseMeshImpl(
	MeshBase							&	mesh,
	Rect2f									area,
	bool									filled,
	float									edge_count
)
{
	if( edge_count < 3.0f ) edge_count = 3.0f;

	glm::vec2 center_point		= {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= area.bottom_right.x - center_point.x;
	float center_to_edge_y		= area.bottom_right.y - center_point.y;

	float rotation_step			= 0.0f;
	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( std::ceil( edge_count ) );

	mesh.vertices.resize( edge_count_integer );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	for( uint32_t i = 0; i < edge_count_integer; ++i ) {
		mesh.vertices[ i ].vertex_coords		= {
			std::cos( rotation_step ) * center_to_edge_x + center_point.x,
			std::sin( rotation_step ) * center_to_edge_y + center_point.y
		};
		mesh.vertices[ i ].uv_coords			= {
			std::cos( rotation_step ) * 0.5f + 0.5f,
			std::sin( rotation_step ) * 0.5f + 0.5f
		};
		rotation_step					+= rotation_step_size;
	}

	if( filled ) {
		// Draw filled polygons
		mesh.indices.resize( size_t( edge_count_integer - 2 ) * 3 );
		{
			for( uint32_t i = 2, a = 0; i < edge_count_integer; ++i, a += 3 ) {
				assert( i < edge_count_integer );

				mesh.indices[ size_t( a ) + 0 ]	= 0;
				mesh.indices[ size_t( a ) + 1 ]	= i - 1;
				mesh.indices[ size_t( a ) + 2 ]	= i;
			}
		}
		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::TRIANGLE_FILLED;
		mesh.SetMeshType( mesh.generated_mesh_type );
	} else {
		// Draw lines
		mesh.indices.resize( size_t( edge_count_integer ) * 2 );
		{
			for( uint32_t i = 1, a = 0; i < edge_count_integer; ++i, a += 2 ) {
				assert( i < edge_count_integer );

				mesh.indices[ size_t( a ) + 0 ]	= i - 1;
				mesh.indices[ size_t( a ) + 1 ]	= i;
			}
			mesh.indices[ size_t( edge_count_integer ) * 2LL - 2 ]	= edge_count_integer - 1;
			mesh.indices[ size_t( edge_count_integer ) * 2LL - 1 ]	= 0;
		}
		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::LINE;
		mesh.SetMeshType( mesh.generated_mesh_type );
	}
	mesh.RecalculateAABBFromVertices();
}

VK2D_API void vk2d::vk2d_internal::GenerateEllipsePieMeshImpl(
	MeshBase							&	mesh,
	Rect2f									area,
	float									begin_angle_radians,
	float									coverage,
	bool									filled,
	float									edge_count
)
{
	if( edge_count < 3.0f )			edge_count		= 3.0f;
	if( coverage > 1.0f )			coverage		= 1.0f;
	if( coverage <= 0.0f )			return;			// Nothing to draw

	glm::vec2 center_point		= {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= area.bottom_right.x - center_point.x;
	float center_to_edge_y		= area.bottom_right.y - center_point.y;

	float rotation_step_size	= float( RAD / edge_count );

	uint32_t edge_count_integer	= uint32_t( std::ceil( edge_count ) );

	{
		auto angle_clamp		= float( std::floor( begin_angle_radians / RAD ) );
		begin_angle_radians		= begin_angle_radians - float( angle_clamp * RAD );
		assert( begin_angle_radians <= RAD );
	}
	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

	mesh.vertices.reserve( size_t( edge_count_integer ) + 2 );
	// Center vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords		= center_point;
		v.uv_coords			= { 0.5f, 0.5f };
		mesh.vertices.push_back( v );
	}

	auto intermediate_point_begin	= uint32_t( begin_angle_radians / rotation_step_size );
	auto intermediate_point_end		= uint32_t( end_angle_radians / rotation_step_size );
	auto intermediate_point_count	= intermediate_point_end - intermediate_point_begin;

	// Start vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords		= {
			std::cos( begin_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( begin_angle_radians ) * center_to_edge_y + center_point.y
		};
		v.uv_coords			= {
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
			v.vertex_coords		= {
				std::cos( rotation_step_size * i ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step_size * i ) * center_to_edge_y + center_point.y
			};
			v.uv_coords			= {
				std::cos( rotation_step_size * i ) * 0.5f + 0.5f,
				std::sin( rotation_step_size * i ) * 0.5f + 0.5f
			};
			mesh.vertices.push_back( v );
		}

		// Second half, from 0 angle to end angle
		float rotation_step		= 0.0f;
		while( rotation_step	< end_angle_radians - RAD ) {

			auto v = vk2d_internal::CreateDefaultValueVertex();
			v.vertex_coords		= {
				std::cos( rotation_step ) * center_to_edge_x + center_point.x,
				std::sin( rotation_step ) * center_to_edge_y + center_point.y
			};
			v.uv_coords			= {
				std::cos( rotation_step ) * 0.5f + 0.5f,
				std::sin( rotation_step ) * 0.5f + 0.5f
			};
			mesh.vertices.push_back( v );

			rotation_step		+= rotation_step_size;
		}
	}

	// End vertex.
	{
		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords		= {
			std::cos( end_angle_radians ) * center_to_edge_x + center_point.x,
			std::sin( end_angle_radians ) * center_to_edge_y + center_point.y
		};
		v.uv_coords			= {
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
				mesh.indices[ a + 0 ]	= 0;
				mesh.indices[ a + 1 ]	= uint32_t( i ) - 1;
				mesh.indices[ a + 2 ]	= uint32_t( i );
			}
		}
		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::TRIANGLE_FILLED;
		mesh.SetMeshType( mesh.generated_mesh_type );
	} else {
		// Draw lines
		mesh.indices.resize( mesh.vertices.size() * 2 );
		for( size_t i = 1, a = 0; i < mesh.vertices.size(); ++i, a += 2 ) {
			mesh.indices[ a + 0 ]	= uint32_t( i ) - 1;
			mesh.indices[ a + 1 ]	= uint32_t( i );
		}
		mesh.indices[ mesh.indices.size() - 2 ]	= uint32_t( mesh.vertices.size() - 1 );
		mesh.indices[ mesh.indices.size() - 1 ]	= 0;

		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::LINE;
		mesh.SetMeshType( mesh.generated_mesh_type );
	}
	mesh.RecalculateAABBFromVertices();
}

VK2D_API void vk2d::vk2d_internal::GenerateRectanglePieMeshImpl(
	MeshBase							&	mesh,
	Rect2f									area,
	float									begin_angle_radians,
	float									coverage,
	bool									filled
)
{
	if( coverage >= 1.0f ) {
		GenerateRectangleMeshImpl( mesh, area, filled );
		return;
	}
	if( coverage <= 0.0f ) {
		return;		// Nothing to draw
	}

	glm::vec2 center_point		= {
		( area.top_left.x + area.bottom_right.x ) / 2.0f,
		( area.top_left.y + area.bottom_right.y ) / 2.0f
	};

	float center_to_edge_x		= area.bottom_right.x - center_point.x;
	float center_to_edge_y		= area.bottom_right.y - center_point.y;

	float width					= std::abs( area.bottom_right.x - area.top_left.x );
	float height				= std::abs( area.bottom_right.y - area.top_left.y );

	auto end_angle_radians		= float( RAD ) * coverage;
	end_angle_radians			+= begin_angle_radians;
	auto area_angle				= end_angle_radians - begin_angle_radians;

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
		// Specialized version of Ray to AABB intersecion test that
		// always intersects and returns the exit coordinates only.
		auto RayExitIntersection	=[](
			glm::vec2 box_coords_1,
			glm::vec2 box_coords_2,
			glm::vec2 box_center,		// ray origin, and box center are both at the same coordinates
			glm::vec2 ray_end
			) -> glm::vec2
		{
			glm::vec2	ray_begin			= box_center;
			float		clipped_exit		= 1.0;

			// Clipping on x and y axis
			float dim_low_x		= ( box_coords_1.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_low_y		= ( box_coords_1.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );
			float dim_high_x	= ( box_coords_2.x - ray_begin.x ) / ( ray_end.x - ray_begin.x );
			float dim_high_y	= ( box_coords_2.y - ray_begin.y ) / ( ray_end.y - ray_begin.y );

			dim_high_x			= std::max( dim_low_x, dim_high_x );
			dim_high_y			= std::max( dim_low_y, dim_high_y );
			clipped_exit		= std::min( dim_high_y, dim_high_x );

			glm::vec2 collider_localized			= ray_end - ray_begin;
			glm::vec2 localized_intersection_point	= collider_localized * clipped_exit;
			glm::vec2 globalized_intersection_point	= localized_intersection_point + box_center;
			return globalized_intersection_point;
		};

		float ray_lenght			= center_to_edge_x + center_to_edge_y;
		glm::vec2 ray_angle_1_end	= { std::cos( begin_angle_radians ) * ray_lenght, std::sin( begin_angle_radians ) * ray_lenght };
		glm::vec2 ray_angle_2_end	= { std::cos( end_angle_radians ) * ray_lenght, std::sin( end_angle_radians ) * ray_lenght };
		ray_angle_1_end				+= center_point;
		ray_angle_2_end				+= center_point;

		unordered_outer_points[ PIE_BEGIN ]	= RayExitIntersection(
			area.top_left,
			area.bottom_right,
			center_point,
			ray_angle_1_end
		);
		unordered_outer_points[ PIE_END ]		= RayExitIntersection(
			area.top_left,
			area.bottom_right,
			center_point,
			ray_angle_2_end
		);
	}

	auto IsOnSameXAxis	= [](
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

	auto IsOnSameYAxis	= [](
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
			uint32_t		original_linear_point_index		= {};
			float			linear_coords					= {};
			glm::vec2		actual_coords					= {};
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
			glm::vec2 actual_coords	= unordered_outer_points[ index ];

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
	{
		mesh.vertices.reserve( 7 );

		auto v = vk2d_internal::CreateDefaultValueVertex();
		v.vertex_coords		= center_point;
		v.uv_coords			= { 0.5f, 0.5f };
		mesh.vertices.push_back( v );

		for( auto opl : outer_point_list ) {
			v.vertex_coords		= opl;
			v.uv_coords			= ( opl - unordered_outer_points[ 0 ] ) / glm::vec2( width, height );
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

		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::TRIANGLE_FILLED;
		mesh.SetMeshType( mesh.generated_mesh_type );

	} else {
		mesh.indices.reserve( mesh.vertices.size() * 2 + 2 );
		for( size_t i = 1; i < mesh.vertices.size(); ++i ) {
			mesh.indices.push_back( uint32_t( i ) - 1 );
			mesh.indices.push_back( uint32_t( i ) );
		}
		mesh.indices.push_back( uint32_t( mesh.vertices.size() - 1 ) );
		mesh.indices.push_back( 0 );

		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::LINE;
		mesh.SetMeshType( mesh.generated_mesh_type );
	}
	mesh.RecalculateAABBFromVertices();
}

VK2D_API void vk2d::vk2d_internal::GenerateLatticeMeshImpl(
	MeshBase							&	mesh,
	Rect2f								area,
	glm::vec2							subdivisions,
	bool								filled
)
{
	uint32_t vertex_count_x			= uint32_t( std::ceil( subdivisions.x ) ) + 2;
	uint32_t vertex_count_y			= uint32_t( std::ceil( subdivisions.y ) ) + 2;
	uint32_t total_vertex_count		= vertex_count_x * vertex_count_y;

	glm::vec2 mesh_size		= area.bottom_right - area.top_left;
	glm::vec2 vertex_spacing	= mesh_size / ( subdivisions + glm::vec2( 1.0f, 1.0f ) );
	glm::vec2 uv_spacing		= glm::vec2( 1.0f, 1.0f ) / ( subdivisions + glm::vec2( 1.0f, 1.0f ) );

	mesh.vertices.resize( total_vertex_count );
	vk2d_internal::ClearVerticesToDefaultValues( mesh.vertices );

	for( size_t y = 0; y < vertex_count_y - 1; ++y ) {
		for( size_t x = 0; x < vertex_count_x - 1; ++x ) {
			auto & v		= mesh.vertices[ y * vertex_count_x + x ];
			v.vertex_coords	= { vertex_spacing.x * x + area.top_left.x, vertex_spacing.y * y + area.top_left.y };
			v.uv_coords		= { uv_spacing.x * x, uv_spacing.y * y };
		}
		auto & v			= mesh.vertices[ y * vertex_count_x + vertex_count_x - 1 ];
		v.vertex_coords		= { area.bottom_right.x, vertex_spacing.y * y + area.top_left.y };
		v.uv_coords			= { 1.0f, uv_spacing.y * y };
	}
	for( size_t x = 0; x < vertex_count_x - 1; ++x ) {
		auto & v		= mesh.vertices[ size_t( vertex_count_y - 1 ) * vertex_count_x + x ];
		v.vertex_coords	= { vertex_spacing.x * x + area.top_left.x, area.bottom_right.y };
		v.uv_coords		= { uv_spacing.x * x, 1.0f };
	}
	auto & v			= mesh.vertices[ size_t( vertex_count_y - 1 ) * vertex_count_x + vertex_count_x - 1 ];
	v.vertex_coords		= { area.bottom_right.x, area.bottom_right.y };
	v.uv_coords			= { 1.0f, 1.0f };

	if( filled ) {
		// Draw solid mesh with polygons.

		auto reserved_size = ( vertex_count_x - 1 ) * ( vertex_count_y - 1 ) * 2 * 3;
		mesh.indices.reserve( reserved_size );
		for( size_t y = 1; y < vertex_count_y; ++y ) {
			for( size_t x = 1; x < vertex_count_x; ++x ) {
				auto tl		= uint32_t( ( y - 1 ) * vertex_count_x + x - 1 );	// top left vertex.
				auto tr		= uint32_t( ( y - 1 ) * vertex_count_x + x );		// top right vertex.
				auto bl		= uint32_t( ( y - 0 ) * vertex_count_x + x - 1 );	// bottom left vertex.
				auto br		= uint32_t( ( y - 0 ) * vertex_count_x + x );		// bottom right vertex.
				mesh.indices.push_back( tl );	// First triangle.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( tr );
				mesh.indices.push_back( tr );	// Second triangle.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( br );
			}
		}

		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::TRIANGLE_FILLED;
		mesh.SetMeshType( mesh.generated_mesh_type );

	} else {
		// Draw lattice lines.

		auto reserved_size = ( vertex_count_x - 1 ) * ( vertex_count_y - 1 ) * 4 * 2;
		mesh.indices.reserve( reserved_size );
		for( size_t y = 1; y < vertex_count_y; ++y ) {
			for( size_t x = 1; x < vertex_count_x; ++x ) {
				auto tl		= uint32_t( ( y - 1 ) * vertex_count_x + x - 1 );	// top left vertex.
				auto tr		= uint32_t( ( y - 1 ) * vertex_count_x + x );		// top right vertex.
				auto bl		= uint32_t( ( y - 0 ) * vertex_count_x + x - 1 );	// bottom left vertex.
				auto br		= uint32_t( ( y - 0 ) * vertex_count_x + x );		// bottom right vertex.
				mesh.indices.push_back( tl );	// First line.
				mesh.indices.push_back( bl );
				mesh.indices.push_back( bl );	// Second line.
				mesh.indices.push_back( br );
				mesh.indices.push_back( br );	// Third line.
				mesh.indices.push_back( tr );
				mesh.indices.push_back( tr );	// Fourth line.
				mesh.indices.push_back( tl );
			}
		}

		mesh.generated				= true;
		mesh.generated_mesh_type		= MeshType::LINE;
		mesh.SetMeshType( mesh.generated_mesh_type );
	}
	mesh.RecalculateAABBFromVertices();
}

VK2D_API void vk2d::vk2d_internal::GenerateTextMeshImpl(
	MeshBase							&	mesh,
	FontResource						*	font,
	glm::vec2								origin,
	std::string								text,
	float									kerning,
	glm::vec2								scale,
	bool									vertical,
	uint32_t								font_face,
	bool									wait_for_resource_load
)
{
	if( std::size( text ) <= 0 ) return;
	if( !font ) return;
	if( !font->impl.get() ) return;
	auto fi = font->impl.get();
	if( wait_for_resource_load ) {
		fi->WaitUntilLoaded( std::chrono::nanoseconds::max() );
	} else {
		if( fi->GetStatus() == ResourceStatus::UNDETERMINED ) return;
	}
	if( !fi->FaceExists( font_face ) ) return;

	mesh.vertices.reserve( text.size() * 4 );
	mesh.indices.reserve( text.size() * 6 );

	auto AppendBox =[ &mesh, scale ](
		const glm::vec2		&	location,
		const Rect2f		&	coords,
		const Rect2f		&	uv_coords,
		uint32_t				texture_channel
		)
	{
		auto vertex_offset		= mesh.vertices.size();
		auto index_offset		= mesh.indices.size();

		auto tcoords			= coords;
		tcoords.top_left		*= scale;
		tcoords.bottom_right	*= scale;
		tcoords					+= location;

		mesh.aabb.top_left.x		= std::min( mesh.aabb.top_left.x, tcoords.top_left.x );
		mesh.aabb.top_left.y		= std::min( mesh.aabb.top_left.y, tcoords.top_left.y );
		mesh.aabb.bottom_right.x	= std::max( mesh.aabb.bottom_right.x, tcoords.bottom_right.x );
		mesh.aabb.bottom_right.y	= std::max( mesh.aabb.bottom_right.y, tcoords.bottom_right.y );

		mesh.vertices.resize( vertex_offset + 4 );
		mesh.vertices[ vertex_offset + 0 ].vertex_coords			= glm::vec2( tcoords.top_left.x, tcoords.top_left.y );
		mesh.vertices[ vertex_offset + 0 ].uv_coords				= glm::vec2( uv_coords.top_left.x, uv_coords.top_left.y );
		mesh.vertices[ vertex_offset + 0 ].color					= Colorf( 1.0f, 1.0f, 1.0f, 1.0f );
		mesh.vertices[ vertex_offset + 0 ].point_size			= 1;
		mesh.vertices[ vertex_offset + 0 ].single_texture_layer	= texture_channel;

		mesh.vertices[ vertex_offset + 1 ].vertex_coords			= glm::vec2( tcoords.bottom_right.x, tcoords.top_left.y );
		mesh.vertices[ vertex_offset + 1 ].uv_coords				= glm::vec2( uv_coords.bottom_right.x, uv_coords.top_left.y );
		mesh.vertices[ vertex_offset + 1 ].color					= Colorf( 1.0f, 1.0f, 1.0f, 1.0f );
		mesh.vertices[ vertex_offset + 1 ].point_size			= 1;
		mesh.vertices[ vertex_offset + 1 ].single_texture_layer	= texture_channel;

		mesh.vertices[ vertex_offset + 2 ].vertex_coords			= glm::vec2( tcoords.top_left.x, tcoords.bottom_right.y );
		mesh.vertices[ vertex_offset + 2 ].uv_coords				= glm::vec2( uv_coords.top_left.x, uv_coords.bottom_right.y );
		mesh.vertices[ vertex_offset + 2 ].color					= Colorf( 1.0f, 1.0f, 1.0f, 1.0f );
		mesh.vertices[ vertex_offset + 2 ].point_size			= 1;
		mesh.vertices[ vertex_offset + 2 ].single_texture_layer	= texture_channel;

		mesh.vertices[ vertex_offset + 3 ].vertex_coords			= glm::vec2( tcoords.bottom_right.x, tcoords.bottom_right.y );
		mesh.vertices[ vertex_offset + 3 ].uv_coords				= glm::vec2( uv_coords.bottom_right.x, uv_coords.bottom_right.y );
		mesh.vertices[ vertex_offset + 3 ].color					= Colorf( 1.0f, 1.0f, 1.0f, 1.0f );
		mesh.vertices[ vertex_offset + 3 ].point_size			= 1;
		mesh.vertices[ vertex_offset + 3 ].single_texture_layer	= texture_channel;

		mesh.indices.resize( index_offset + 6 );
		mesh.indices[ index_offset + 0 ]	= uint32_t( vertex_offset + 0 );
		mesh.indices[ index_offset + 1 ]	= uint32_t( vertex_offset + 2 );
		mesh.indices[ index_offset + 2 ]	= uint32_t( vertex_offset + 1 );
		mesh.indices[ index_offset + 3 ]	= uint32_t( vertex_offset + 1 );
		mesh.indices[ index_offset + 4 ]	= uint32_t( vertex_offset + 2 );
		mesh.indices[ index_offset + 5 ]	= uint32_t( vertex_offset + 3 );
	};

	auto location	= origin;
	if( vertical ) {
		// Writing vertical text
		{
			auto gi = fi->GetGlyphInfo( font_face, text[ 0 ] );
			mesh.aabb.top_left		= gi->vertical_coords.top_left * scale + location;
			mesh.aabb.bottom_right	= gi->vertical_coords.bottom_right * scale + location;
		}
		for( auto c : text ) {
			auto gi = fi->GetGlyphInfo( font_face, c );
			AppendBox( location, gi->vertical_coords, gi->uv_coords, gi->atlas_index );
			location.y	+= ( gi->vertical_advance + kerning ) * scale.y;
		}
	} else {
		// Writing horisontal text
		{
			auto gi = fi->GetGlyphInfo( font_face, text[ 0 ] );
			mesh.aabb.top_left		= gi->horisontal_coords.top_left * scale + location;
			mesh.aabb.bottom_right	= gi->horisontal_coords.bottom_right * scale + location;
		}
		for( auto c : text ) {
			auto gi = fi->GetGlyphInfo( font_face, c );
			AppendBox( location, gi->horisontal_coords, gi->uv_coords, gi->atlas_index );
			location.x	+= ( gi->horisontal_advance + kerning ) * scale.x;
		}
	}

	mesh.SetTexture( fi->GetTextureResource() );
}
