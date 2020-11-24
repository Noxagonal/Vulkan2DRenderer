#pragma once

#include "../Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



template<typename T>
class Vector2Base
{
public:

	T x		= {};
	T y		= {};

	Vector2Base()																= default;
	Vector2Base( T x, T y ) : x( x ), y( y ) {};
	Vector2Base( const vk2d::Vector2Base<T> & other )							= default;
	Vector2Base( vk2d::Vector2Base<T> && other )								= default;
	Vector2Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
	}

	vk2d::Vector2Base<T> & operator=( const vk2d::Vector2Base<T> & other )		= default;
	vk2d::Vector2Base<T> & operator=( vk2d::Vector2Base<T> && other )			= default;

	vk2d::Vector2Base<T> operator+( vk2d::Vector2Base<T> other ) const
	{
		return { x + other.x, y + other.y };
	}
	vk2d::Vector2Base<T> operator-( vk2d::Vector2Base<T> other ) const
	{
		return { x - other.x, y - other.y };
	}
	vk2d::Vector2Base<T> operator*( vk2d::Vector2Base<T> other ) const
	{
		return { x * other.x, y * other.y };
	}
	vk2d::Vector2Base<T> operator/( vk2d::Vector2Base<T> other ) const
	{
		return { x / other.x, y / other.y };
	}
	vk2d::Vector2Base<T> operator*( T other ) const
	{
		return { x * other, y * other };
	}
	vk2d::Vector2Base<T> operator/( T other ) const
	{
		return { x / other, y / other };
	}
	vk2d::Vector2Base<T> & operator+=( vk2d::Vector2Base<T> other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator-=( vk2d::Vector2Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator*=( vk2d::Vector2Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator/=( vk2d::Vector2Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator*=( T other )
	{
		x *= other;
		y *= other;
		return *this;
	}
	vk2d::Vector2Base<T> & operator/=( T other )
	{
		x /= other;
		y /= other;
		return *this;
	}
	bool operator==( vk2d::Vector2Base<T> other )
	{
		return x == other.x && y == other.y;
	}
	bool operator!=( vk2d::Vector2Base<T> other )
	{
		return x != other.x || y != other.y;
	}



	T CalculateDistanceFrom( vk2d::Vector2Base<T> other )
	{
		vk2d::Vector2Base<T> temp = *this - other;
		return std::sqrt( temp.x * temp.x + temp.y * temp.y );
	}

	vk2d::Vector2Base<T> CalculateNormalized()
	{
		auto distance = CalculateDistanceFrom( {} );
		if( distance < T( 0.001 ) ) return vk2d::Vector2Base<T>( T( 1.0 ), T( 0.0 ) );
		return *this / distance;
	}
};

using Vector2f			= vk2d::Vector2Base<float>;
using Vector2d			= vk2d::Vector2Base<double>;
using Vector2i			= vk2d::Vector2Base<int32_t>;
using Vector2u			= vk2d::Vector2Base<uint32_t>;

} // vk2d
