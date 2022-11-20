#pragma once

#include <core/Common.h>

#include "VertexBase.hpp"

#include <containers/Color.hpp>



namespace vk2d {



class StandardVertex : public VertexBase<glm::vec2, glm::vec2, Colorf, float, uint32_t> {
public:
	using Base = VertexBase;

	/// @brief		Spatial coordinates of this vertex.
	glm::vec2			&	vertex_coords			= Get<0>();

	/// @brief		UV coordinates of this vertex.
	///
	///				This determines where to read from a texture. UV coordinate space is always in range from 0.0 to 1.0 where
	///				{0.0, 0.0} is top left of texture and {1.0, 1.0} is bottom right of the texture.
	glm::vec2			&	uv_coords				= Get<1>();

	/// @brief		Vertex color.
	///
	///				Texture color is multiplied by this, or if no texture is applied, determines the displayed color for this
	///				vertex.
	Colorf				&	color					= Get<2>();

	/// @brief		Size of the vertex.
	///
	///				This parameter is only used when rendering points.
	float				&	point_size				= Get<3>();

	/// @brief		Texture layer.
	///
	///				If texture has layers, then this parameter tells which layer of that texture is to be used with this
	///				vertex. If any texture layer weights are used, eg. Mesh::texture_layer_weights, then this
	///				parameter is ignored.
	uint32_t			&	single_texture_layer	= Get<4>();
};



namespace vk2d_internal {
namespace tests {

struct StandardVertexReference {
	glm::vec2	vertex_coords			= {};
	glm::vec2	uv_coords				= {};
	Colorf		color					= {};
	float		point_size				= {};
	uint32_t	single_texture_layer	= {};
};

static_assert( VertexHasVertexCoords<StandardVertex> );
static_assert( VertexHasUVCoords<StandardVertex> );
static_assert( VertexHasColor<StandardVertex> );
static_assert( VertexHasPointSize<StandardVertex> );
static_assert( VertexHasSingleTextureLayer<StandardVertex> );

static_assert( StandardVertex::GetMySize()				== sizeof( StandardVertexReference ) );
static_assert( StandardVertex::GetMyAlignment()			== alignof( StandardVertexReference ) );
static_assert( alignof( StandardVertex )				>= alignof( StandardVertexReference ) );

static_assert( StandardVertex::GetMemberCount()			== 5 );
static_assert( StandardVertex::GetMemberOffset<0>()		== offsetof( StandardVertexReference, vertex_coords ) );
static_assert( StandardVertex::GetMemberOffset<1>()		== offsetof( StandardVertexReference, uv_coords ) );
static_assert( StandardVertex::GetMemberOffset<2>()		== offsetof( StandardVertexReference, color ) );
static_assert( StandardVertex::GetMemberOffset<3>()		== offsetof( StandardVertexReference, point_size ) );
static_assert( StandardVertex::GetMemberOffset<4>()		== offsetof( StandardVertexReference, single_texture_layer ) );
static_assert( StandardVertex::GetMemberSize<0>()		== sizeof( StandardVertexReference::vertex_coords ) );
static_assert( StandardVertex::GetMemberSize<1>()		== sizeof( StandardVertexReference::uv_coords ) );
static_assert( StandardVertex::GetMemberSize<2>()		== sizeof( StandardVertexReference::color ) );
static_assert( StandardVertex::GetMemberSize<3>()		== sizeof( StandardVertexReference::point_size ) );
static_assert( StandardVertex::GetMemberSize<4>()		== sizeof( StandardVertexReference::single_texture_layer ) );

} // tests
} // vk2d_internal



} // vk2d
