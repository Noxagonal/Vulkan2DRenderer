#pragma once

#include "SourceCommon.h"

#include "../../Include/Interface/RenderPrimitives.h"

#include <vector>

namespace vk2d {

namespace _internal {



std::vector<VkExtent2D>				GenerateMipSizes(
	vk2d::Vector2u					full_size );



} // _internal

} // vk2d
