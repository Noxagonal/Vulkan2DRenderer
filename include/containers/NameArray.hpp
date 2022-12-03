#pragma once

#include <core/Common.hpp>

#include <string_view>



namespace vk2d {

namespace vk2d_internal {



template<typename T>
concept NameArrayElementRequirement = requires( T o )
{
	requires std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>;
};



} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t Size>
requires( Size > 0 )
class NameArray
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	consteval										NameArray() = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<vk2d_internal::NameArrayElementRequirement ...ArgsT>
	requires( sizeof...( ArgsT ) != Size )
	consteval										NameArray(
		ArgsT										...names
	)
	{
		static_assert( sizeof...( ArgsT ) == Size, "Incorrect number of names." );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<vk2d_internal::NameArrayElementRequirement ...ArgsT>
	requires( sizeof...( ArgsT ) == Size )
	consteval										NameArray(
		ArgsT										...names
	)
	{
		SetNames<0, ArgsT...>( std::forward<ArgsT>( names )... );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr std::string_view						operator[](
		size_t										index
	)
	{
		return data[ index ];
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<
		size_t										Position,
		vk2d_internal::NameArrayElementRequirement	FirstT,
		vk2d_internal::NameArrayElementRequirement	...RestT
	>
	consteval void									SetNames(
		FirstT										name,
		RestT									&&	...rest
	)
	{
		data[ Position ] = name;

		if constexpr( sizeof...( RestT ) )
		{
			SetNames<Position + 1>( std::forward<RestT>( rest )... );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::array<std::string_view, Size>				data;
};



} // vk2d
