#pragma once

#include "core/Common.h"


namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Render target texture and window coordinate space scale and position.
///
///				Coordinate space tells how to map vertex coordinates to different parts of the rendered surface. See the values
///				for more info.
enum class RenderCoordinateSpace : uint32_t
{
	/// @brief		0x0 coordinates at top left corner, bottom right coordinate is the number of pixels.
	///
	///				Size of the texel space is the number of pixels, if you draw at coordinates 20x20, it'll be offset 20x20
	///				pixels from the top left corner.
	/// 
	/// @note		Does not stretch content.
	TEXEL_SPACE,

	/// @brief		0x0 coordinates at center, same scale and orientation as TEXEL_SPACE but offsetted to center.
	///
	///				Size of the texel space is the number of pixels, if you draw at coordinates -20x-20 it'll be offset -20x-20
	///				pixels from the center towards top left corner.
	/// 
	/// @note		Does not stretch content.
	TEXEL_SPACE_CENTERED,

	/// @brief		0x0 coordinates at top left corner, 1x1 coordinates at bottom right but longer side is extended.
	///
	///				This mode keeps the minimum size of 1.0 by 1.0 always, but if width is longer than height, width is extended
	///				so that the contents do not appear stretched. Likewise, if height is more than width, then height is
	///				extended.
	///
	///				For example, assuming window size of 600x600 pixels, if you draw at coordinates 0.75x0.5, it'll be offset to
	///				450x300 pixels from the top left corner. 
	/// 
	/// @note		Does not stretch content.
	NORMALIZED_SPACE,

	/// @brief		0x0 coordinates at center, same scale and orientation as NORMALIZED_SPACE but top left is -1x-1 and bottom
	///				right is 1x1.
	///
	///				This mode keeps the minimum size of 2.0 by 2.0 always, but if width is longer than height, width is extended
	///				so that the contents do not appear stretched. Likewise, if height is more than width, then height is
	///				extended.
	///
	///				For example, assuming window size of 600x600 pixels, if you draw at coordinates -0.5x0.0, it'll be offset to
	///				150x300 pixels from the top left corner.
	///
	/// @note		Does not stretch content.
	NORMALIZED_SPACE_CENTERED,

	/// @brief		Linear coordinates from -1x-1 to 1x1.
	///
	///				Allows manually setting your own scales. Top left coordinates are always -1x-1, center is always 0x0 and
	///				Bottom right coordinates are always 1x1 regardless of the shape of the window.
	///
	/// @note		Stretches content.
	LINEAR_SPACE,	///< Stretches content. Always ( -1.0, -1.0 ) top left, ( 1.0, 1.0 ) bottom right regardless of the texel size.
};



} // vk2d
