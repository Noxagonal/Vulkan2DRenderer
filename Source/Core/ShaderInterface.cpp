
#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/ShaderInterface.h"

#include <tuple>







bool vk2d::_internal::ShaderProgram::operator<(
	const vk2d::_internal::ShaderProgram		&	other
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

bool vk2d::_internal::ShaderProgram::operator>(
	const vk2d::_internal::ShaderProgram		&	other
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

bool vk2d::_internal::ShaderProgram::operator<=(
	const vk2d::_internal::ShaderProgram		&	other
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

bool vk2d::_internal::ShaderProgram::operator>=(
	const vk2d::_internal::ShaderProgram		&	other
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

bool vk2d::_internal::ShaderProgram::operator==(
	const vk2d::_internal::ShaderProgram		&	other
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

bool vk2d::_internal::ShaderProgram::operator!=(
	const vk2d::_internal::ShaderProgram		&	other
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







bool vk2d::_internal::PipelineSettings::operator<(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	// do this everywhere and understand how strict - smaller - than works;

	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) <
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::PipelineSettings::operator>(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) >
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::PipelineSettings::operator<=(
	const vk2d::_internal::PipelineSettings		&	other
	) const
{
	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) <=
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::PipelineSettings::operator>=(
	const vk2d::_internal::PipelineSettings		&	other
	) const
{
	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) >=
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::PipelineSettings::operator==(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) ==
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}

bool vk2d::_internal::PipelineSettings::operator!=(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	return
		std::tie(
			render_pass,
			primitive_topology,
			polygon_mode,
			shader_programs,
			samples
		) !=
		std::tie(
			other.render_pass,
			other.primitive_topology,
			other.polygon_mode,
			other.shader_programs,
			other.samples
		);
}
