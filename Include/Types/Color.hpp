#pragma once

#include "Core/Common.h"

#include <initializer_list>



namespace vk2d {



/// @brief		This is a collection of 4 color channels in order of RGBA.
/// @tparam		T
///				Data type per color channel.
template<typename T>
class ColorBase
{
public:

	T r			= {};
	T g			= {};
	T b			= {};
	T a			= {};

	ColorBase()										= default;
	ColorBase( const vk2d::ColorBase<T> & other )	= default;
	ColorBase( vk2d::ColorBase<T> && other )		= default;
	ColorBase( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) r = *e++;
		if( e ) g = *e++;
		if( e ) b = *e++;
		if( e ) a = *e++;
	}
	ColorBase( T r, T g, T b, T a ) :
		r( r ),
		g( g ),
		b( b ),
		a( a )
	{};

	vk2d::ColorBase<T> & operator=( const vk2d::ColorBase<T> & other )	= default;
	vk2d::ColorBase<T> & operator=( vk2d::ColorBase<T> && other )		= default;
};

/// @brief		float per color channel.
using Colorf			= vk2d::ColorBase<float>;

/// @brief		double per color channel.
using Colord			= vk2d::ColorBase<double>;

/// @brief		uint8_t per color channel.
using Color8			= vk2d::ColorBase<uint8_t>;

/// @brief		uint16_t per color channel.
using Color16			= vk2d::ColorBase<uint16_t>;

/// @brief		uint32_t per color channel.
using Color32			= vk2d::ColorBase<uint32_t>;

} // vk2d
