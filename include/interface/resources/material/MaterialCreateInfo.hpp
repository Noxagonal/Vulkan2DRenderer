#pragma once

#include <core/Common.hpp>
#include "ShaderCreateInfo.hpp"



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct MaterialCreateInfo
{
	std::vector<ShaderCreateInfo>		shader_create_infos;

	uint32_t		PLACEHOLDER_vertex_draw_settings;
	uint32_t		PLACEHOLDER_instance_draw_settings;
	uint32_t		PLACEHOLDER_pipeline_settings;
	uint32_t		PLACEHOLDER_texture_settings;
	uint32_t		PLACEHOLDER_sampler_settings;
};



} // vk2d
