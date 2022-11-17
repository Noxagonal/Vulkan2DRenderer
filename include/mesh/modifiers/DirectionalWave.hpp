#pragma once

#include <core/Common.h>

#include <mesh/Mesh.hpp>



namespace vk2d {
namespace mesh_modifiers {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Makes a simple wave pattern that can be animated.
/// 
/// @tparam		VertexT
///				Vertex type. Must be derived from vk2d::VertexBase.
/// 
/// @param[in,out]	mesh
///				Reference to mesh which is modified.
/// 
/// @param[in]	direction_radians
///				Direction where the waves are moving towards. Angle is in radians.
/// 
/// @param[in]	frequency
///				Frequency is the number of waves inside the mesh bounding box average size.
/// 
/// @param[in]	animation
///				Animation is a float value that determines where the peaks of the waves are, slowly increasing this value will
///				make the waves move. This value rolls over at 0.0 and 1.0. Meaning that 0.0 and 1.0 will produce the exact same
///				pattern.
/// 
/// @param[in]	intensity
///				This determines the wave peak intensity per axis. This is applied from the perspective of the direction the wave
///				and not the final result.
/// 
/// @param[in]	origin
///				Origin point where waves are generated from, since this is a directional wave this parameter only really matters
///				if you're animating the rotation of the direction of the waves.
template<vk2d_internal::VertexBaseDerivedType VertexT>
requires( vk2d_internal::VertexHasVertexCoords<VertexT> )
void						DirectionalWave(
	Mesh<VertexT>		&	mesh,
	float					direction_radians,
	float					frequency,
	float					animation,
	glm::vec2				intensity,
	glm::vec2				origin
)
{
	auto size = aabb.bottom_right - aabb.top_left;

	glm::vec2	dir {
		std::cos( direction_radians ),
		std::sin( direction_radians )
	};
	// Matrix input has flipped column and row order.
	auto forward_rotation_matrix = glm::mat2(
		+dir.x, +dir.y,
		-dir.y, +dir.x
	);
	auto backward_rotation_matrix = glm::mat2(
		+dir.x, -dir.y,
		+dir.y, +dir.x
	);

	for( auto & i : mesh.vertices ) {
		auto c = i.vertex_coords - origin;
		c = backward_rotation_matrix * c;

		auto d = float( animation / RAD ) + float( frequency * ( ( c.x / size.x + c.y / size.y ) / 2.0f ) * RAD );
		i.vertex_coords = {
			std::cos( d ) * intensity.x + c.x,
			std::sin( d ) * intensity.y + c.y };

		i.vertex_coords = forward_rotation_matrix * i.vertex_coords;
		i.vertex_coords += origin;
	}
}



} // mesh_modifiers
} // vk2d
