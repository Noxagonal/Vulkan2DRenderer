#pragma once

#include <core/SourceCommon.hpp>

#include <vulkan/shaders/ShaderInterface.hpp>

#include <utility/Hasher.hpp>
#include <utility/EnumHelpers.hpp>



namespace vk2d {
namespace vulkan {



class ComputePipelineInfo
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 									ComputePipelineInfo() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 									ComputePipelineInfo(
		VkPipelineLayout					vulkan_pipeline_layout,
		VkShaderModule						vulkan_compute_shader
	) :
		vulkan_pipeline_layout( vulkan_pipeline_layout ),
		vulkan_compute_shader( vulkan_compute_shader )
	{
		hash = CalculateHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 									ComputePipelineInfo(
		const ComputePipelineInfo		&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline ComputePipelineInfo			&	operator=(
		const ComputePipelineInfo		&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool								operator<(
		const ComputePipelineInfo		&	other
	) const
	{
		return hash < other.GetHash();
	}

	inline VkPipelineLayout					GetVulkanPipelineLayout() const
	{
		return vulkan_pipeline_layout;
	}

	inline VkShaderModule					GetVulkanComputeShader() const
	{
		return vulkan_compute_shader;
	}

	inline size_t							GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline constexpr size_t					CalculateHash() const
	{
		Hasher hasher;
		hasher.Hash( reinterpret_cast<size_t>( vulkan_pipeline_layout ) );
		hasher.Hash( reinterpret_cast<size_t>( vulkan_compute_shader ) );
		return hasher.GetHash();
	}

	VkPipelineLayout						vulkan_pipeline_layout			= {};
	VkShaderModule							vulkan_compute_shader			= {};

	size_t									hash;
};



} // vulkan
} // vk2d
