#pragma once

#include "../Core/Common.h"

#include "Vector2.h"
#include "Color.h"

#include <array>
#include <vector>



namespace vk2d {



// Mesh type is used in multiple places to indicate what kind of data structure the mesh has.
enum class MeshType : uint32_t
{
	TRIANGLE_FILLED,	// 3 indices form a triangle from a list of vertices.
	TRIANGLE_WIREFRAME,	// 3 indices form a triangle from a list of vertices, only edges are rendered.
	LINE,				// 2 indices form a line from a list of vertices.
	POINT,				// a list of vertices is used to render individual points, no indices are used.
};



// Vertex is a single point of which polygons or lines can be formed between.
// Or you could just render the points on their own.
struct Vertex
{
	alignas( 8 )	vk2d::Vector2f			vertex_coords			= {};	// Coordinates.
	alignas( 8 )	vk2d::Vector2f			uv_coords				= {};	// UV coordinates to texture.
	alignas( 16 )	vk2d::Colorf			color					= {};	// Texture color is multiplied by this.
	alignas( 4 )	float					point_size				= {};	// If rendering points, this is the width of the point.
	alignas( 4 )	uint32_t				single_texture_channel	= {};	// If not rendering multitextured, this is the texture layer to use with this vertex.
};

struct VertexIndex_2
{
	std::array<uint32_t, 2>		indices		= {};
};

struct VertexIndex_3
{
	std::array<uint32_t, 3>		indices		= {};
};

struct ImageData
{
	vk2d::Vector2u				size		= {};
	std::vector<vk2d::Color8>	data;
};


} // vk2d
