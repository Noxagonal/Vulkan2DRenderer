#pragma once

#include "../Core/Common.h"


namespace vk2d {



enum class Multisamples : uint32_t
{
	SAMPLE_COUNT_1		= 1,
	SAMPLE_COUNT_2		= 2,
	SAMPLE_COUNT_4		= 4,
	SAMPLE_COUNT_8		= 8,
	SAMPLE_COUNT_16		= 16,
	SAMPLE_COUNT_32		= 32,
	SAMPLE_COUNT_64		= 64
};
inline vk2d::Multisamples operator&( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) & uint32_t( m2 ) );
}
inline vk2d::Multisamples operator|( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) | uint32_t( m2 ) );
}



enum class RenderCoordinateSpace : uint32_t
{
	TEXEL_SPACE,					// Default, ( 0, 0 ) at top left corner of the screen, bottom right is window extent.
	TEXEL_SPACE_CENTERED,			// Same as TEXEL_SPACE but ( 0, 0 ) coordinates are at screen center.
	NORMALIZED_SPACE,				// Window always contains 1x1 coordinate space, larger side is extended to keep window contents from stretching.
	NORMALIZED_SPACE_CENTERED,		// Same as NORMALIZED SPACE but window always contains 2x2 coordinate space, ( 0, 0 ) is window center
	NORMALIZED_VULKAN,				// ( -1, -1 ) top left, ( 1, 1 ) bottom right.
};



} // vk2d
