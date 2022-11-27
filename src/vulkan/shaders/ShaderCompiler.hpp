#pragma once

#include <core/SourceCommon.hpp>

#include <containers/ShaderInfo.hpp>



namespace vk2d {

namespace vk2d_internal {
class InstanceImpl;
} // vk2d_internal

namespace vulkan {



class Device;
class ShaderManager;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderCompiler
{
	friend class ShaderManager;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShaderCompiler(
		vk2d_internal::InstanceImpl		&	instance
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Compile and create Vulkan shader module.
	///
	/// @note		Multithreading: Not thread safe.
	///
	/// @param[in]	user_shader_info
	///				User provided shader text.
	///
	/// @return		Vulkan shader module.
	VkShaderModule							CreateShaderModule(
		const ShaderInfo				&	user_shader_info
	);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Compiles SpirV shader module from GLSL code.
	///
	/// @note		Multithreading: Not thread safe.
	///
	/// @param[in]	user_shader_info
	///				Shader info provided by the user.
	///
	/// @return		SpirV shader.
	std::vector<uint32_t>					CompileSpirV(
		const ShaderInfo				&	user_shader_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl			&	instance;
};



} // vulkan
} // vk2d
