#pragma once

#include "core/Common.h"

#include "MeshBase.hpp"



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Collection of vertices and their relations.
///
///				A mesh represents a collection of vertices and their relations forming a shape that can be drawn onto a surface.
/// 
///				- A mesh can be created using any of the mesh generator functions, or constructing it manually.
///				- A mesh can be modified using any of the modifier functions or by manually modifying the vertices and indices.
///
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
template<typename ...ShaderParamsT>
class Mesh : public MeshBase
{
public:

};



} // vk2d
