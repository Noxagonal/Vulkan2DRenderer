#pragma once

#include "../Core/Common.h"

#include <initializer_list>



namespace vk2d {



template<typename T>
struct ColorBase
{
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

using Colorf			= vk2d::ColorBase<float>;
using Colord			= vk2d::ColorBase<double>;
using Color8			= vk2d::ColorBase<uint8_t>;
using Color16			= vk2d::ColorBase<uint16_t>;
using Color32			= vk2d::ColorBase<uint32_t>;

} // vk2d