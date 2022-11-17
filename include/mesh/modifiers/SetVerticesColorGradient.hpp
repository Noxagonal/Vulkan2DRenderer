#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Set color to all vertices following linear gradient.
///
///				Gradient is generated between two points.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	color_1
///				First color.
/// 
/// @param[in]	color_2
///				Second color.
/// 
/// @param[in]	coord_1
///				Location of the first color.
/// 
/// @param[in]	coord_2
///				Location of the second color.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasColor<VertexT> )
void						SetVerticesColorGradient(
	Mesh<VertexT>		&	mesh,
	Colorf					color_1,
	Colorf					color_2,
	glm::vec2				coord_1,
	glm::vec2				coord_2
)
{
	glm::vec2		coord_vector = coord_2 - coord_1;
	glm::vec2		coord_dir = {};
	float			coord_lenght = std::sqrt( coord_vector.x * coord_vector.x + coord_vector.y * coord_vector.y );
	if( coord_lenght > 0.0f ) {
		coord_dir = coord_vector / coord_lenght;
	}
	else {
		coord_lenght = KINDA_SMALL_VALUE;
		coord_dir = { 1.0f, 0.0f };
	}

	// Matrix input has flipped column and row order.
	auto forward_rotation_matrix = glm::mat2(
		+coord_dir.x, +coord_dir.y,
		-coord_dir.y, +coord_dir.x
	);
	auto backward_rotation_matrix = glm::mat2(
		+coord_dir.x, -coord_dir.y,
		+coord_dir.y, +coord_dir.x
	);
	auto coord_linearilized = backward_rotation_matrix * ( coord_dir * coord_lenght );

	for( auto & v : mesh.vertices ) {
		auto c = v.vertex_coords - coord_1;

		c = backward_rotation_matrix * c;
		auto cx = c.x / coord_linearilized.x;

		if( cx < 0.0f )			v.color = color_1;
		else if( cx > 1.0f )	v.color = color_2;
		else {
			auto g = Colorf(
				color_1.r * ( 1.0f - cx ) + color_2.r * cx,
				color_1.g * ( 1.0f - cx ) + color_2.g * cx,
				color_1.b * ( 1.0f - cx ) + color_2.b * cx,
				color_1.a * ( 1.0f - cx ) + color_2.a * cx
			);
			v.color = g;
			//v.color = { 0, 1, 0, 1 };
		}
		c = forward_rotation_matrix * c;

		v.vertex_coords = c + coord_1;
	}
}



} // mesh_modifiers
} // vk2d
