#pragma once

#include <core/Common.hpp>

#include <containers/Color.hpp>

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



// TODO: Move this on it's own file.
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
