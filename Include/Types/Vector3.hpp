#pragma once

#include "../Core/Common.h"

#include <initializer_list>



namespace vk2d {



template<typename T>
class Vector3Base
{
public:

	T x		= {};
	T y		= {};
	T z		= {};

	Vector3Base()																= default;
	Vector3Base( T x, T y, T z ) : x( x ), y( y ), z( z ) {};
	Vector3Base( const vk2d::Vector3Base<T> & other )							= default;
	Vector3Base( vk2d::Vector3Base<T> && other )								= default;
	Vector3Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 3 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
		if( e ) z = *e++;
	}

	vk2d::Vector3Base<T> & operator=( const vk2d::Vector3Base<T> & other )		= default;
	vk2d::Vector3Base<T> & operator=( vk2d::Vector3Base<T> && other )			= default;

	vk2d::Vector3Base<T> operator+( vk2d::Vector3Base<T> other ) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	vk2d::Vector3Base<T> operator-( vk2d::Vector3Base<T> other ) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}
	vk2d::Vector3Base<T> operator*( vk2d::Vector3Base<T> other ) const
	{
		return { x * other.x, y * other.y, z * other.z };
	}
	vk2d::Vector3Base<T> operator/( vk2d::Vector3Base<T> other ) const
	{
		return { x / other.x, y / other.y, z / other.z };
	}
	vk2d::Vector3Base<T> operator*( T other ) const
	{
		return { x * other, y * other, z * other };
	}
	vk2d::Vector3Base<T> operator/( T other ) const
	{
		return { x / other, y / other, z / other };
	}
	vk2d::Vector3Base<T> & operator+=( vk2d::Vector3Base<T> other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	vk2d::Vector3Base<T> & operator-=( vk2d::Vector3Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	vk2d::Vector3Base<T> & operator*=( vk2d::Vector3Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	vk2d::Vector3Base<T> & operator/=( vk2d::Vector3Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}
	vk2d::Vector3Base<T> & operator*=( T other )
	{
		x *= other;
		y *= other;
		z *= other;
		return *this;
	}
	vk2d::Vector3Base<T> & operator/=( T other )
	{
		x /= other;
		y /= other;
		z /= other;
		return *this;
	}
	bool operator==( vk2d::Vector3Base<T> other )
	{
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=( vk2d::Vector3Base<T> other )
	{
		return x != other.x || y != other.y || z != other.z;
	}
};

using Vector3f			= vk2d::Vector3Base<float>;
using Vector3d			= vk2d::Vector3Base<double>;
using Vector3i			= vk2d::Vector3Base<int32_t>;
using Vector3u			= vk2d::Vector3Base<uint32_t>;

} // vk2d
