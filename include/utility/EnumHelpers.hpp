#pragma once

#include <core/Common.hpp>

#include <type_traits>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Converts an enum to it's underlying type.
///
/// @tparam		EnumT
///				Enum type.
///
/// @param[in]	e
///				Enumerator value.
///
/// @return		Underlying type of the enum.
template<typename EnumT>
constexpr std::underlying_type_t<EnumT> EnumToUnderlying( EnumT e )
{
	return static_cast<std::underlying_type_t<EnumT>>( e );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Converts enum underlying type back to enum.
///
/// @tparam		EnumT
///				Enum type.
///
/// @param		v
///				Enum underlying type to convert back to enum.
///
/// @return		Value as enumerator.
template<typename EnumT>
constexpr EnumT UnderlyingToEnum( std::underlying_type_t<EnumT> v )
{
	return static_cast<EnumT>( v );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Applies bitwise or to 2 enums of the same type.
///
/// @tparam		EnumT
///				Enum type.
///
/// @param[in]	first
///				First enumerator value.
///
/// @param[in]	second
///				Second enumerator value.
///
/// @return		Resulting enum value.
template<typename EnumT>
constexpr EnumT EnumBitwiseOr( EnumT first, EnumT second )
{
	return UnderlyingToEnum<EnumT>( EnumToUnderlying( first ) | EnumToUnderlying( second ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Applies bitwise and to 2 enums of the same type.
///
/// @tparam		EnumT
///				Enum type.
///
/// @param[in]	first
///				First enumerator value.
///
/// @param[in]	second
///				Second enumerator value.
///
/// @return		Resulting enum value.
template<typename EnumT>
constexpr EnumT EnumBitwiseAnd( EnumT first, EnumT second )
{
	return UnderlyingToEnum<EnumT>( EnumToUnderlying( first ) & EnumToUnderlying( second ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Checks that first enum has all the bits "on" of the second enum.
///
/// @tparam		EnumT
///				Enum type.
///
/// @param[in]	first
///				First enumerator value.
///
/// @param[in]	second
///				Second enumerator value.
///
/// @return		True if the first enum has all the bits "on" of the second enum, false otherwise.
template<typename EnumT>
constexpr bool EnumHasFields( EnumT first, EnumT second )
{
	return ( EnumToUnderlying( first ) & EnumToUnderlying( second ) ) == EnumToUnderlying( second );
}



} // vk2d
