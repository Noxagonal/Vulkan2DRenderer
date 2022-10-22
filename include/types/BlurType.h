#pragma once

#include "../Core/Common.h"

namespace vk2d {



enum class BlurType : uint32_t
{
	BOX,			///< Box blur is a type of blur where each pixel in a rectangle area contributes equally to the result.
	GAUSSIAN,		///< Gaussian blur is a type of blur where closer pixels contribute more and further pixels contribute less to the result, the amount of contribution follows the gaussian falloff.
};



} // vk2d
