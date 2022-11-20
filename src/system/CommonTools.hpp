#pragma once

#include <core/SourceCommon.hpp>

namespace vk2d {

namespace vk2d_internal {



std::vector<VkExtent2D>				GenerateMipSizes(
	glm::uvec2						full_size );



} // vk2d_internal

} // vk2d
