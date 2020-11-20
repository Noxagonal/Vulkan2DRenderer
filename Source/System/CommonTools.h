#pragma once

#include "../Core/SourceCommon.h"

#include "../../Include/Types/Vector2.h"

namespace vk2d {

namespace _internal {



std::vector<VkExtent2D>				GenerateMipSizes(
	vk2d::Vector2u					full_size );



} // _internal

} // vk2d
