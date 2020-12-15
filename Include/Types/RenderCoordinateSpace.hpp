#pragma once

#include "Core/Common.h"


namespace vk2d {



/// @brief		Coordinate space tells how to map vertex coordinates to different parts of the rendered surface.
///				See the values for more info.
enum class RenderCoordinateSpace : uint32_t
{
	TEXEL_SPACE,					///< No content stretching. Default, ( 0.0, 0.0 ) at top left corner, bottom right is texel size.
	TEXEL_SPACE_CENTERED,			///< No content stretching. Same scale as TEXEL_SPACE but ( 0.0, 0.0 ) coordinates are at the center, top/bottom, left/right coordinates are negative/positive half of texel size respectively.
	NORMALIZED_SPACE,				///< No content stretching. ( 0.0, 0.0 ) is top left corner, shorter dimension minimum range is always 0.0 - 1.0 regardless of the texel size, longer dimension range is extended to keep window contents from stretching.
	NORMALIZED_SPACE_CENTERED,		///< No content stretching. Same as NORMALIZED_SPACE but ( 0.0, 0.0 ) is center, ( -1.0, -1.0 ) is towards top left, ( 1.0, 1.0 ) is towards bottom right.
	NORMALIZED_VULKAN,				///< Stretches content. Always ( -1.0, -1.0 ) top left, ( 1.0, 1.0 ) bottom right regardless of the texel size.
};



} // vk2d
