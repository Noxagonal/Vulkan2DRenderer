#pragma once

#include <core/SourceCommon.hpp>
#include "ShaderCompiler.hpp"
#include "Shader.hpp"
#include <containers/ShaderInfo.hpp>

#include <map>



namespace vk2d {
namespace vk2d_internal {
class InstanceImpl;
} // vk2d_internal
namespace vulkan {



class Device;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderManager
{
	friend class ShaderCompiler;

public:

	ShaderManager(
		vk2d_internal::InstanceImpl		&	instance
	);

	VkShaderModule							FindShader(
		const ShaderInfo				&	shader_info
	);
	
	VkShaderModule							FindShader(
		size_t								shader_hash
	);

	VkShaderModule							CreateShader(
		const ShaderInfo				&	user_shader_info
	);

	void									DestroyShader(
		VkShaderModule						shader_module
	);

private:
	vk2d_internal::InstanceImpl			&	instance;

	ShaderCompiler							shader_compiler;

	std::map<size_t, VkShaderModule>		shader_list;
};



} // vulkan
} // vk2d
