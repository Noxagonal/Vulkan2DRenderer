#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vulkan {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderManagerShaderEntry
{
	VkShaderModule	vulkan_shader_module	= {};
	size_t			reference_count			= {};
	size_t			hash					= {};
};



} // vulkan
} // vk2d
