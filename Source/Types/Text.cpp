
#include "Core/SourceCommon.h"

#include "Types/Text.h"



VK2D_API vk2d::Text::Text(
	const char			*	str
)
{
	auto str_size = std::strlen( str );
	if( str_size > 0 ) {
		Resize( str_size );
		for( size_t i = 0; i < str_size; i++ )
		{
			str_data[ i ] = str[ i ];
		}
		str_data[ str_size ] = '\0';
	}
}

VK2D_API vk2d::Text::Text(
	const wchar_t		*	str
)
{
	auto str_size = std::wcslen( str );
	if( str_size > 0 ) {
		Resize( str_size );
		for( size_t i = 0; i < str_size; i++ )
		{
			str_data[ i ] = str[ i ];
		}
		str_data[ str_size ] = '\0';
	}
}

VK2D_API vk2d::Text::Text(
	const vk2d::Text	&	other
)
{
	CopyOther( other );
}

VK2D_API vk2d::Text::Text(
	vk2d::Text			&&	other
)
{
	MoveOther( std::move( other ) );
}

VK2D_API vk2d::Text::~Text()
{
	Deallocate();
}

VK2D_API vk2d::Text & VK2D_APIENTRY vk2d::Text::operator=(
	const vk2d::Text	&	other
)
{
	CopyOther( other );
	return *this;
}

VK2D_API vk2d::Text & VK2D_APIENTRY vk2d::Text::operator=(
	vk2d::Text			&&	other
)
{
	MoveOther( std::move( other ) );
	return *this;
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

VK2D_API void VK2D_APIENTRY vk2d::Text::Reserve(
	size_t new_capacity
)
{
	auto alloc_size = new_capacity + 1;
	if( str_capacity < new_capacity ) {
		auto new_str = new int32_t[ alloc_size ];
		if( !new_str ) return;
		if( str_data ) {
			std::memcpy( new_str, str_data, str_size * sizeof( *new_str ) );
			new_str[ str_size ] = '\0';
			Deallocate();
		}
		str_data		= new_str;
		str_capacity	= new_capacity;
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Text::Resize(
	size_t new_size
)
{
	Reserve( new_size );
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

VK2D_API void VK2D_APIENTRY vk2d::Text::CopyOther( const vk2d::Text & other )
{
	Resize( other.str_size );
	for( size_t i = 0; i < other.str_size; ++i ) {
		str_data[ i ] = other.str_data[ i ];
	}
	str_size = other.str_size;
	str_data[ str_size ] = '\0';
}

VK2D_API void VK2D_APIENTRY vk2d::Text::MoveOther( vk2d::Text && other )
{
	std::swap( str_data,		other.str_data );
	std::swap( str_size,		other.str_size );
	std::swap( str_capacity,	other.str_capacity );
}

VK2D_API void VK2D_APIENTRY vk2d::Text::Deallocate()
{
	delete[] str_data;
	str_data		= nullptr;
	str_capacity	= 0;
}
