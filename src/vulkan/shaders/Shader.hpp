#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vulkan {



class ShaderManager;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Shader
{
	friend class ShaderManager;

	Shader(
		VkShaderModule				vulkan_shader_module
	);

public:

private:

	VkShaderModule					vulkan_shader_module;
};



} // vulkan
} // vk2d
