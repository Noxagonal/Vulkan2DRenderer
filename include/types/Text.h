#pragma once

#include <core/Common.h>

#include <string>
#include <string.h>
#include <wchar.h>

namespace vk2d {



// This is a stub of a class I'm thinking of maybe implementing.
// Or I may throw it out.
// Thoughts:
// - If I implement this then I should also implement Array to replace std::vector (I kinda like the idea).
// - I'd have to reinvent the wheel that stl already does a really good job at.
// - Performance will likely be worse than stl libraries unless I dedicate serious efforts to this.
// - Optional memory pool as parameter? if so, maybe use stl polymorphic memory resources instead of implementing our own pool?
// - If memory pooled, multithreading will be a concern although this would happen anyways.
// - There might be more types I'd have to implement in the future (List, Map, Deque, Stack...).
// - Allows more control, more operators without fear of name conflicts, eg. std::cout << vk2d_array_object;
// - C++ expects some lower-case names like size() instead of Size(), or begin() instead of Begin(). This conflicts with my naming scheme.
// - Users would have to learn custom, likely less competent, containers, could be mitigated by allowing stl containers as input.
// - Would likely result more data copying between the library and the host application.
// - Stl is standard and most people know it well, should at least stick to the naming convention used by stl.
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

	VK2D_API										Text(
		const char								*	str );

	VK2D_API										Text(
		const wchar_t							*	str );

	VK2D_API										Text(
		const Text								&	other );

	VK2D_API										Text(
		Text									&&	other );

	VK2D_API										~Text();

	VK2D_API Text								&	operator=(
		const Text								&	other );

	VK2D_API Text								&	operator=(
		Text									&&	other );

	VK2D_API void									FromUTF8(
		std::string									str );

	VK2D_API std::string							ToUTF8();

	VK2D_API void									Reserve(
		size_t										new_capacity );
	VK2D_API void									Resize(
		size_t										new_size );
	VK2D_API size_t									size();
	VK2D_API int32_t							*	data();
	VK2D_API int32_t							*	begin();
	VK2D_API int32_t							*	end();

private:
	VK2D_API void									CopyOther(
		const Text								&	other );
	VK2D_API void									MoveOther(
		Text									&&	other );
	VK2D_API void									Deallocate();
	int32_t										*	str_data						= {};
	size_t											str_size						= {};
	size_t											str_capacity					= {};
};



} // vk2d
