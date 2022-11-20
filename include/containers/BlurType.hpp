#pragma once

#include <core/Common.hpp>

namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class BlurType : uint32_t
{
	/// @brief		Use box blur.
	///
	///				This is a type of blur where each pixel in a rectangle area contributes equally to the result.
	BOX,

	/// @brief		Use gaussian blur.
	///
	///				This is what you would usually want for smoothed out blur.
	///				This is a type of blur where closer pixels contribute more and further pixels contribute less to the
	///				result, the amount of contribution follows the gaussian falloff.
	GAUSSIAN,
};



} // vk2d
