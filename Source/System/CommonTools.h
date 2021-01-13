#pragma once

#include "Core/SourceCommon.h"

namespace vk2d {

namespace _internal {



std::vector<VkExtent2D>				GenerateMipSizes(
	glm::uvec2						full_size );



} // _internal

} // vk2d
