#pragma once

#include "Core/Common.h"


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



} // vk2d
