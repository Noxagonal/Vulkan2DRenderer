#pragma once

#include "core/Common.h"


namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Multisample flags.
///
///				Single variable can contain multible multisample flags.
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Operator & for multisamples.
///
///				This can be used to test if a multisample variable contains a specific multisample flag. For example:
/// @code
///				vk2d::Multisamples ms1 = (outside source);
///				if( ms1 & vk2d::Multisamples::SAMPLE_COUNT_8 ) {
///					// ms1 sample count 8 flag is set.
///				}
/// @endcode
/// 
/// @param[in]	m1
///				First multisample value.
/// 
/// @param[in]	m2
///				Second multisample value.
/// 
/// @return		Multisample flags that were set in both m1 and m2.
inline vk2d::Multisamples operator&( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) & uint32_t( m2 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Operator | for multisamples.
///
///				This can be used to set a multisample flag on a variable without disturbing other multisample flags in the
///				variable. For example:
/// @code
///				auto combined = vk2d::Multisamples::SAMPLE_COUNT_1 | vk2d::Multisamples::SAMPLE_COUNT_4;
/// @endcode
/// 
/// @param[in]	m1
///				First multisample value.
/// 
/// @param[in]	m2
///				Second multisample value.
/// 
/// @return		Combined flags in single value.
inline vk2d::Multisamples operator|( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) | uint32_t( m2 ) );
}



} // vk2d
