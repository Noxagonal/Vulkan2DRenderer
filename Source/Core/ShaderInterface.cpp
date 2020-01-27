#pragma once

#include "../Header/Core/SourceCommon.h"

#include "../Header/Core/ShaderInterface.h"







bool vk2d::_internal::ShaderStages::operator<(
	const vk2d::_internal::ShaderStages		&	other
) const
{
	if( vertex < other.vertex ) {
		return true;
	}
	if( fragment < other.fragment ) {
		return true;
	}
	return false;
}

bool vk2d::_internal::ShaderStages::operator>(
	const vk2d::_internal::ShaderStages		&	other
) const
{
	if( vertex > other.vertex ) {
		return true;
	}
	if( fragment > other.fragment ) {
		return true;
	}
	return false;
}

bool vk2d::_internal::ShaderStages::operator==(
	const vk2d::_internal::ShaderStages		&	other
) const
{
	if( vertex != other.vertex ) {
		return false;
	}
	if( fragment != other.fragment ) {
		return false;
	}
	return true;
}

bool vk2d::_internal::ShaderStages::operator!=(
	const vk2d::_internal::ShaderStages		&	other
) const
{
	return !operator==( other );
}







bool vk2d::_internal::PipelineSettings::operator<(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	if( primitive_topology < other.primitive_topology ) {
		return true;
	}
	if( polygon_mode < other.polygon_mode ) {
		return true;
	}
	if( shader_stages < other.shader_stages ) {
		return true;
	}
	if( samples < other.samples ) {
		return true;
	}
	return false;
}

bool vk2d::_internal::PipelineSettings::operator>(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	if( primitive_topology > other.primitive_topology ) {
		return true;
	}
	if( polygon_mode > other.polygon_mode ) {
		return true;
	}
	if( shader_stages > other.shader_stages ) {
		return true;
	}
	if( samples > other.samples ) {
		return true;
	}
	return false;
}

bool vk2d::_internal::PipelineSettings::operator==(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	if( primitive_topology != other.primitive_topology ) {
		return false;
	}
	if( polygon_mode != other.polygon_mode ) {
		return false;
	}
	if( shader_stages != other.shader_stages ) {
		return false;
	}
	if( samples != other.samples ) {
		return false;
	}
	return true;
}

bool vk2d::_internal::PipelineSettings::operator!=(
	const vk2d::_internal::PipelineSettings		&	other
) const
{
	return !operator==( other );
}
