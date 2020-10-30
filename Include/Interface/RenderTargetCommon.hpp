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
