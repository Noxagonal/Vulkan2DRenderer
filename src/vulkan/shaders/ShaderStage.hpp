#pragma once

#include <core/SourceCommon.hpp>



namespace vk2d {
namespace vulkan {



enum class ShaderStage
{
	NONE					= 0,
	VERTEX,
	TESSELLATION_CONTROL,
	TESSELLATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
};



} // vulkan
} // vk2d
