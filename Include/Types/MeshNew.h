#pragma once

#include "Core/Common.h"

#include "Types/MeshPrimitives.hpp"
#include "Types/Rect2.hpp"
#include "Types/Color.hpp"

#include <vector>
#include <string>

namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
}

class FontResource;
class Sampler;
class Texture;



/// @brief		A mesh represents a collection of vertices and their relations
///				forming a shape that can be drawn onto a surface.
/// 
///				- A mesh can be created using any of the mesh generator functions,
///				or constructing it manually.
///				- A mesh can be modified using any of the modifier functions or
///				by manually modifying the vertices and indices.
class MeshNew
{
public:
	// Push back, push front, size...
	// Generate UV layer...

	// Might need to hide these under some functions as the vector sizes need to match.
	std::vector<glm::vec2>										coords							= {};
	std::vector<std::vector<glm::vec2>>							UVs								= {};
	std::vector<std::vector<std::vector<float>>>				texture_channel_weights			= {};	// Maybe put this in user data? It's not really that useful...
	std::vector<std::vector<uint8_t>>							user_data_per_vertex			= {};

	std::vector<uint32_t>										indices							= {};

	std::vector<vk2d::Sampler>									samplers						= {};
	std::vector<vk2d::Texture>									textures						= {};
	std::vector<uint8_t>										user_data_per_draw				= {};

	vk2d::Rect2f												aabb							= {};

private:
	bool														generated						= false;							///< Tells if this mesh was created by a mesh generator.
	vk2d::MeshType												generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;	///< Tells the original mesh type if it was created by a mesh generator.
	vk2d::MeshType												mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;	///< Current mesh type.

	uint32_t													user_data_per_vertex_size		= {};
	uint32_t													user_data_per_draw_size			= {};
};



} // vk2d
