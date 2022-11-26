#pragma once

#include <core/SourceCommon.hpp>
#include "ShaderCompiler.hpp"
#include "Shader.hpp"



namespace vk2d {
namespace vulkan {



class Device;
class Shader;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderManager
{
	friend class ShaderCompiler;

public:

	ShaderManager(
		vk2d_internal::InstanceImpl		&	instance
	);

	Shader								*	FindShader();

	Shader								*	CreateShader();
	void									DestroyShader(
		Shader							*	shader
	);

private:
	vk2d_internal::InstanceImpl			&	instance;

	ShaderCompiler							shader_compiler;
};



} // vulkan
} // vk2d
