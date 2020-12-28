
#include "Core/SourceCommon.h"

#include "Types/Text.h"



VK2D_API vk2d::Text::Text(
	const char			*	str
)
{
	auto str_size = std::strlen( str );
	if( str_size > 0 ) {
		resize( str_size );
		for( size_t i = 0; i < str_size; i++ )
		{
			str_data[ i ] = str[ i ];
		}
		str_data[ str_size ] = '\0';
	}
};

VK2D_API vk2d::Text::Text(
	const wchar_t		*	str
)
{
	auto str_size = std::wcslen( str );
	if( str_size > 0 ) {
		resize( str_size );
		for( size_t i = 0; i < str_size; i++ )
		{
			str_data[ i ] = str[ i ];
		}
		str_data[ str_size ] = '\0';
	}
};

VK2D_API vk2d::Text::~Text()
{
	delete [] str_data;
}


VK2D_API void VK2D_APIENTRY vk2d::Text::FromUTF8(
	std::string		str )
{
	// TODO...
}

VK2D_API std::string VK2D_APIENTRY vk2d::Text::ToUTF8()
{
	// TODO...
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Text::reserve(
	size_t new_size
)
{
	auto alloc_size = new_size + 1;
	if( str_capacity < new_size ) {
		auto new_str = new int32_t[ alloc_size ];
		if( !new_str ) return;
		if( str_data ) {
			std::memcpy( new_str, str_data, str_size * sizeof( *new_str ) );
			new_str[ str_size ] = '\0';
		}
		str_data		= new_str;
		str_capacity	= new_size;
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Text::resize(
	size_t new_size
)
{
	reserve( new_size );
	if( str_data ) {
		str_size = new_size;
	} else {
		str_size = 0;
	}
}

VK2D_API size_t VK2D_APIENTRY vk2d::Text::size()
{
	return str_size;
}

VK2D_API int32_t * VK2D_APIENTRY vk2d::Text::data()
{
	return str_data;
}

VK2D_API int32_t * VK2D_APIENTRY vk2d::Text::begin()
{
	return &str_data[ 0 ];
}

VK2D_API int32_t * VK2D_APIENTRY vk2d::Text::end()
{
	return &str_data[ str_size ];
}
