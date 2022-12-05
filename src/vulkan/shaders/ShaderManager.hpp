#pragma once

#include <core/SourceCommon.hpp>
#include "ShaderCompiler.hpp"
#include "Shader.hpp"
#include <interface/resources/material/ShaderCreateInfo.hpp>
#include "ShaderHandle.hpp"



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
	friend class ShaderHandle;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ShaderEntry
	{
		VkShaderModule								vulkan_shader_module			= {};
		size_t										reference_count					= {};
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShaderManager(
		vk2d_internal::InstanceImpl				&	instance,
		Device									&	vulkan_device
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShaderManager(
		const ShaderManager						&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ShaderManager(
		ShaderManager							&&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~ShaderManager();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tries to find a shader and return a handle to it if it exists.
	///
	/// @param		shader_create_info
	///				Shader create info structure describing the shader we want to find.
	///
	/// @return		Handle to existing shader or empty handle if shader was not found.
	ShaderHandle									FindShader(
		const ShaderCreateInfo					&	shader_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tries to find a shader and return a handle to it if it exists.
	///
	/// @param		shader_hash
	///				Hash of the shader we wish to find.
	///
	/// @return		Handle to existing shader or empty handle if shader was not found.
	ShaderHandle									FindShader(
		size_t										shader_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets a shader if it exists or creates a new shader if it does not exist.
	///
	/// @param[in]	shader_create_info
	///				Shader create info structure describing the shader we want to find or create.
	///
	/// @return		Handle to shader.
	ShaderHandle									GetShader(
		const ShaderCreateInfo					&	shader_create_info
	);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Creates a new shader.
	///
	/// @param[in]	shader_create_info
	///				Shader create info structure describing the shader we want to create.
	///
	/// @return		Handle to shader.
	ShaderHandle									CreateShader(
		const ShaderCreateInfo					&	shader_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											IncrementReferenceCount(
		size_t										shader_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DecrementReferenceCount(
		size_t										shader_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyShader(
		size_t										shader_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyShader(
		std::map<size_t, ShaderEntry>::iterator		shader_list_iterator
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyAllShaders();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl					&	instance;
	Device										&	vulkan_device;

	ShaderCompiler									shader_compiler;

	// TODO: Find better ways to implement a hash map. eg. std::flat_map. Mind pointer invalidation though.
	std::map<size_t, ShaderEntry>					shader_list;
};



} // vulkan
} // vk2d
