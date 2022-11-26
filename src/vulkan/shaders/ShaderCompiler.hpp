#pragma once

#include <core/SourceCommon.hpp>

#include <containers/ShaderText.hpp>
#include <vulkan/shaders/ShaderStage.hpp>



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
	/// @note		Multithreading: Single thread at a time.
	///
	/// @param[in]	user_shader_text
	///				User provided shader text.
	///
	/// @return		Vulkan shader module.
	VkShaderModule							CreateShaderModule(
		ShaderStage							stage,
		const ShaderText				&	user_shader_text
	);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Compiles SpirV shader module from GLSL code.
	///
	/// @note		Multithreading: Single thread at a time.
	///
	/// @param[in]	stage
	///				Shader stage we're compiling the shader for.
	///
	/// @param[in]	user_shader_text
	///				Shader text provided by the user.
	///
	/// @return		SpirV shader.
	std::vector<uint32_t>					CompileSpirV(
		ShaderStage							stage,
		const ShaderText				&	user_shader_text
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl			&	instance;

	bool									is_good					= {};
};



} // vulkan
} // vk2d
