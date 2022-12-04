#pragma once

#include <core/SourceCommon.hpp>
#include "ShaderCompiler.hpp"
#include "Shader.hpp"
#include <interface/resources/material/ShaderCreateInfo.hpp>



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
		vk2d_internal::InstanceImpl		&	instance,
		Device							&	vulkan_device
	);

	~ShaderManager();

	VkShaderModule							FindShader(
		const ShaderCreateInfo				&	shader_info
	);
	
	VkShaderModule							FindShader(
		size_t								shader_hash
	);

	VkShaderModule							CreateShader(
		const ShaderCreateInfo			&	shader_create_info
	);

	void									DestroyShader(
		VkShaderModule						shader_module
	);

private:

	void									DestroyShaders();

	vk2d_internal::InstanceImpl			&	instance;
	Device								&	vulkan_device;

	ShaderCompiler							shader_compiler;

	std::map<size_t, VkShaderModule>		shader_list;
};



} // vulkan
} // vk2d
