
#include "../Core/SourceCommon.h"

#include "ShaderInterface.h"







bool vk2d::_internal::GraphicsShaderProgram::operator<(
	const vk2d::_internal::GraphicsShaderProgram		&	other
) const
{
	return
		std::tie(
			vertex,
			fragment
		) <
		std::tie(
			other.vertex,
			other.fragment
		);
}

bool vk2d::_internal::GraphicsShaderProgram::operator>(
	const vk2d::_internal::GraphicsShaderProgram		&	other
) const
{
	return
		std::tie(
			vertex,
			fragment
		) >
		std::tie(
			other.vertex,
			other.fragment
		);
}

bool vk2d::_internal::GraphicsShaderProgram::operator<=(
	const vk2d::_internal::GraphicsShaderProgram		&	other
	) const
{
	return
		std::tie(
			vertex,
			fragment
		) <=
		std::tie(
			other.vertex,
			other.fragment
		);
}

bool vk2d::_internal::GraphicsShaderProgram::operator>=(
	const vk2d::_internal::GraphicsShaderProgram		&	other
	) const
{
	return
		std::tie(
			vertex,
			fragment
		) >=
		std::tie(
			other.vertex,
			other.fragment
		);
}

bool vk2d::_internal::GraphicsShaderProgram::operator==(
	const vk2d::_internal::GraphicsShaderProgram		&	other
) const
{
	return
		std::tie(
			vertex,
			fragment
		) ==
		std::tie(
			other.vertex,
			other.fragment
		);
}

bool vk2d::_internal::GraphicsShaderProgram::operator!=(
	const vk2d::_internal::GraphicsShaderProgram		&	other
) const
{
	return
		std::tie(
			vertex,
			fragment
		) !=
		std::tie(
			other.vertex,
			other.fragment
		);
}







bool vk2d::_internal::GraphicsPipelineSettings::operator<(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	// do this everywhere and understand how strict - smaller - than works;

	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) <
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator>(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) >
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator<=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
	) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) <=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator>=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
	) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) >=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator==(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) ==
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator!=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
	) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples,
			enable_blending
		) !=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples,
			other.enable_blending
		);
}



bool vk2d::_internal::ComputePipelineSettings::operator<( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) <
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator>( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) >
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator<=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) <=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator>=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) >=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator==( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) ==
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator!=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			vk_pipeline_layout,
			vk_shader_program
		) !=
		std::tie(
			other.vk_pipeline_layout,
			other.vk_shader_program
		);
}
