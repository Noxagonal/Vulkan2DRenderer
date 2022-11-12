#pragma once

#include <core/Common.h>

#include <types/Color.hpp>

#include <array>
#include <vector>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Indicates mesh type.
///
///				A mesh type is used in multiple places to indicate what kind of data structure the mesh has and how to interpret
///				it.
enum class MeshType : uint32_t
{
	/// @brief		3 indices form a triangle from a list of vertices.
	TRIANGLE_FILLED,

	/// @brief		3 indices form a triangle from a list of vertices, only edges of the triangles are rendered.
	TRIANGLE_WIREFRAME,

	/// @brief		2 indices form a line from a list of vertices.
	LINE,

	/// @brief		a list of vertices is used to render individual points, no indices are used.
	POINT,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		A collection of data which defines a single corner of a triangle, end of a line or a point in space.
///
///				A vertex is a single point in space defining different parameters for that location, like color and size of this
///				point <br>
///				A vertex is just a point in space but it can be connected to other points via the index list, which would then
///				form a triangle or a line between different vertices.
struct Vertex
{
	/// @brief		Spacial coordinates of this vertex.
	[[deprecated]]
	alignas( 8 )	glm::vec2				vertex_coords			= {};

	/// @brief		UV coordinates of this vertex.
	///
	///				This determines where to read from a texture. UV coordinate space is always in range from 0.0 to 1.0 where
	///				{0.0, 0.0} is top left of texture and {1.0, 1.0} is bottom right of the texture.
	[[deprecated]]
	alignas( 8 )	glm::vec2				uv_coords				= {};

	/// @brief		Vertex color.
	///
	///				Texture color is multiplied by this, or if no texture is applied, determines the displayed color for this
	///				vertex.
	[[deprecated]]
	alignas( 16 )	Colorf					color					= {};

	/// @brief		Size of the vertex.
	///
	///				This parameter is only used when rendering points.
	[[deprecated]]
	alignas( 4 )	float					point_size				= {};

	/// @brief		Texture layer.
	///
	///				If texture has layers, then this parameter tells which layer of that texture is to be used with this
	///				vertex. If any texture layer weights are used, eg. Mesh::texture_layer_weights, then this
	///				parameter is ignored.
	[[deprecated]]
	alignas( 4 )	uint32_t				single_texture_layer	= {};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is a container enforcing using 2 indices when drawing lines.
struct VertexIndex_2
{
	std::array<uint32_t, 2>					indices					= {};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is a container enforcing using 3 indices when drawing polygons.
struct VertexIndex_3
{
	std::array<uint32_t, 3>					indices					= {};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is a container to hold image texel size and texel data.
struct ImageData
{
	/// @brief		Texel size of an image.
	glm::uvec2								size					= {};

	/// @brief		A list of texel color values forming an image.
	///
	///				Image data is left to right, top to bottom ordered.
	std::vector<Color8>						data;
};



} // vk2d
