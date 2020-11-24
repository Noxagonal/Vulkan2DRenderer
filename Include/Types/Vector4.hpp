#pragma once

#include "../Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



template<typename T>
class Vector4Base
{
public:

	T x		= {};
	T y		= {};
	T z		= {};
	T w		= {};

	Vector4Base()																= default;
	Vector4Base( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {};
	Vector4Base( const vk2d::Vector4Base<T> & other )							= default;
	Vector4Base( vk2d::Vector4Base<T> && other )								= default;
	Vector4Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
		if( e ) z = *e++;
		if( e ) w = *e++;
	}

	vk2d::Vector4Base<T> & operator=( const vk2d::Vector4Base<T> & other )		= default;
	vk2d::Vector4Base<T> & operator=( vk2d::Vector4Base<T> && other )			= default;

	vk2d::Vector4Base<T> operator+( vk2d::Vector4Base<T> other ) const
	{
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}
	vk2d::Vector4Base<T> operator-( vk2d::Vector4Base<T> other ) const
	{
		return { x - other.x, y - other.y, z - other.z, w - other.w };
	}
	vk2d::Vector4Base<T> operator*( vk2d::Vector4Base<T> other ) const
	{
		return { x * other.x, y * other.y, z * other.z, w * other.w };
	}
	vk2d::Vector4Base<T> operator/( vk2d::Vector4Base<T> other ) const
	{
		return { x / other.x, y / other.y, z / other.z, w / other.w };
	}
	vk2d::Vector4Base<T> operator*( T other ) const
	{
		return { x * other, y * other, z * other, w * other };
	}
	vk2d::Vector4Base<T> operator/( T other ) const
	{
		return { x / other, y / other, z / other, w / other };
	}
	vk2d::Vector4Base<T> & operator+=( vk2d::Vector4Base<T> other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}
	vk2d::Vector4Base<T> & operator-=( vk2d::Vector4Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}
	vk2d::Vector4Base<T> & operator*=( vk2d::Vector4Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}
	vk2d::Vector4Base<T> & operator/=( vk2d::Vector4Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}
	vk2d::Vector4Base<T> & operator*=( T other )
	{
		x *= other;
		y *= other;
		z *= other;
		w *= other;
		return *this;
	}
	vk2d::Vector4Base<T> & operator/=( T other )
	{
		x /= other;
		y /= other;
		z /= other;
		w /= other;
		return *this;
	}
	bool operator==( vk2d::Vector4Base<T> other )
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
	bool operator!=( vk2d::Vector4Base<T> other )
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}



	T CalculateDistanceFrom( vk2d::Vector4Base<T> other )
	{
		vk2d::Vector4Base<T> temp = *this - other;
		return std::sqrt( temp.x * temp.x + temp.y * temp.y + temp.z * temp.z + temp.w * temp.w );
	}

	vk2d::Vector4Base<T> CalculateNormalized()
	{
		auto distance = CalculateDistanceFrom( {} );
		if( distance < T( 0.001 ) ) return vk2d::Vector4Base<T>( T( 1.0 ), T( 0.0 ), T( 0.0 ), T( 0.0 ) );
		return *this / distance;
	}
};

using Vector4f			= vk2d::Vector4Base<float>;
using Vector4d			= vk2d::Vector4Base<double>;
using Vector4i			= vk2d::Vector4Base<int32_t>;
using Vector4u			= vk2d::Vector4Base<uint32_t>;

} // vk2d
