#pragma once

#include "Core/Common.h"

#include <string>
#include <string.h>
#include <wchar.h>

namespace vk2d {



class Text
{
public:
	VK2D_API																		Text()						=	default;

	template<typename T>
																					Text(
		const std::basic_string<T>				&	str
	)
	{
		auto str_size = std::size( str );
		if( str_size > 0 ) {
			resize( str_size );
			for( size_t i = 0; i < str_size; i++ )
			{
				str_data[ i ] = str[ i ];
			}
			str_data[ str_size ] = '\0';
		}
	}

	template<typename T>
																					Text(
		std::basic_string_view<T>					str
	)
	{
		auto str_size = std::size( str );
		if( str_size > 0 ) {
			resize( str_size );
			for( size_t i = 0; i < str_size; i++ )
			{
				str_data[ i ] = str[ i ];
			}
			str_data[ str_size ] = '\0';
		}
	}

	VK2D_API																		Text(
		const char								*	str );

	VK2D_API																		Text(
		const wchar_t							*	str );

	VK2D_API																		~Text();

	VK2D_API void									VK2D_APIENTRY					FromUTF8(
		std::string									str );

	VK2D_API std::string							VK2D_APIENTRY					ToUTF8();

	VK2D_API void									VK2D_APIENTRY					reserve(
		size_t										new_size );
	VK2D_API void									VK2D_APIENTRY					resize(
		size_t										new_size );
	VK2D_API size_t									VK2D_APIENTRY					size();
	VK2D_API int32_t							*	VK2D_APIENTRY					data();
	VK2D_API int32_t							*	VK2D_APIENTRY					begin();
	VK2D_API int32_t							*	VK2D_APIENTRY					end();

private:
	int32_t										*	str_data						= {};
	size_t											str_size						= {};
	size_t											str_capacity					= {};
};



} // vk2d
