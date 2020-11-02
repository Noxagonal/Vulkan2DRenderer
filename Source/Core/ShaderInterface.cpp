
#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/ShaderInterface.h"







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
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) <
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator>(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	return
		std::tie(
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) >
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator<=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
	) const
{
	return
		std::tie(
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) <=
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator>=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
	) const
{
	return
		std::tie(
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) >=
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator==(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	return
		std::tie(
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) ==
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::GraphicsPipelineSettings::operator!=(
	const vk2d::_internal::GraphicsPipelineSettings		&	other
) const
{
	return
		std::tie(
			vk_render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) !=
		std::tie(
			other.vk_render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator<( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) <
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator>( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) >
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator<=( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) <=
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator>=( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) >=
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator==( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) ==
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputeShaderProgram::operator!=( const vk2d::_internal::ComputeShaderProgram & other ) const
{
	return
		std::tie(
			compute
		) !=
		std::tie(
			other.compute
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator<( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) <
		std::tie(
			other.shader_programs
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator>( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) >
		std::tie(
			other.shader_programs
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator<=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) <=
		std::tie(
			other.shader_programs
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator>=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) >=
		std::tie(
			other.shader_programs
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator==( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) ==
		std::tie(
			other.shader_programs
		);
}

bool vk2d::_internal::ComputePipelineSettings::operator!=( const vk2d::_internal::ComputePipelineSettings & other ) const
{
	return
		std::tie(
			shader_programs
		) !=
		std::tie(
			other.shader_programs
		);
}
