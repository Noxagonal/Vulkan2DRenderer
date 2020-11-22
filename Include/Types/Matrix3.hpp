#pragma once

#include "../Core/Common.h"

#include "Vector3.hpp"

#include <initializer_list>
#include <cmath>



namespace vk2d {



template<typename T>
class Matrix3Base
{
public:

	vk2d::Vector3Base<T>	row_1	= {};
	vk2d::Vector3Base<T>	row_2	= {};
	vk2d::Vector3Base<T>	row_3	= {};

	Matrix3Base()											= default;
	Matrix3Base( const vk2d::Matrix3Base<T> & other )		= default;
	Matrix3Base( vk2d::Matrix3Base<T> && other )			= default;
	Matrix3Base( float identity )
	{
		row_1	= { identity, 0.0f, 0.0f };
		row_2	= { 0.0f, identity, 0.0f };
		row_3	= { 0.0f, 0.0f, identity };
	}
	Matrix3Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 9 );
		auto e = elements.begin();
		if( e ) row_1.x = *e++;
		if( e ) row_1.y = *e++;
		if( e ) row_1.z = *e++;
		if( e ) row_2.x = *e++;
		if( e ) row_2.y = *e++;
		if( e ) row_2.z = *e++;
		if( e ) row_3.x = *e++;
		if( e ) row_3.y = *e++;
		if( e ) row_3.z = *e++;
	}
	Matrix3Base(
		float r1_c1, float r1_c2, float r1_c3,
		float r2_c1, float r2_c2, float r2_c3,
		float r3_c1, float r3_c2, float r3_c3
	)
	{
		row_1	= { r1_c1, r1_c2, r1_c3 };
		row_2	= { r2_c1, r2_c2, r2_c3 };
		row_3	= { r3_c1, r3_c2, r3_c3 };
	}

	vk2d::Matrix3Base<T> & operator=( const vk2d::Matrix3Base<T> & other )		= default;
	vk2d::Matrix3Base<T> & operator=( vk2d::Matrix3Base<T> && other )			= default;

	vk2d::Matrix3Base<T> operator*( T multiplier )
	{
		vk2d::Matrix3Base<T> ret = {};
		ret.row_1		= row_1 * multiplier;
		ret.row_2		= row_2 * multiplier;
		ret.row_3		= row_3 * multiplier;
		return ret;
	}

	vk2d::Matrix3Base<T> operator*( const vk2d::Matrix3Base<T> & other )
	{
		vk2d::Matrix3Base<T> ret = *this;
		ret.row_1.x		= row_1.x * other.row_1.x + row_1.y * other.row_2.x + row_1.z * other.row_3.x;
		ret.row_1.y		= row_1.x * other.row_1.y + row_1.y * other.row_2.y + row_1.z * other.row_3.y;
		ret.row_1.z		= row_1.x * other.row_1.z + row_1.y * other.row_2.z + row_1.z * other.row_3.z;
		ret.row_2.x		= row_2.x * other.row_1.x + row_2.y * other.row_2.x + row_2.z * other.row_3.x;
		ret.row_2.y		= row_2.x * other.row_1.y + row_2.y * other.row_2.y + row_2.z * other.row_3.y;
		ret.row_2.z		= row_2.x * other.row_1.z + row_2.y * other.row_2.z + row_2.z * other.row_3.z;
		ret.row_3.x		= row_3.x * other.row_1.x + row_3.y * other.row_2.x + row_3.z * other.row_3.x;
		ret.row_3.y		= row_3.x * other.row_1.y + row_3.y * other.row_2.y + row_3.z * other.row_3.y;
		ret.row_3.z		= row_3.x * other.row_1.z + row_3.y * other.row_2.z + row_3.z * other.row_3.z;
		return ret;
	}

	vk2d::Vector3Base<T> operator*( const vk2d::Vector3Base<T> & other )
	{
		vk2d::Vector3Base<T> ret = {};
		ret.x			= row_1.x * other.x + row_1.y * other.y + row_1.z * other.z;
		ret.y			= row_2.x * other.x + row_2.y * other.y + row_2.z * other.z;
		ret.z			= row_3.x * other.x + row_3.y * other.y + row_3.z * other.z;
		return ret;
	}

	vk2d::Matrix3Base<T> & operator*=( T multiplier )
	{
		*this	= *this * multiplier;
		return *this;
	}
	vk2d::Matrix3Base<T> & operator*=( const vk2d::Matrix3Base<T> & other )
	{
		*this	= *this * other;
		return *this;
	}
	bool operator==( vk2d::Matrix3Base<T> other )
	{
		return row_1 == other.row_1 && row_2 == other.row_2 && row_3 == other.row_3;
	}
	bool operator!=( vk2d::Matrix3Base<T> other )
	{
		return row_1 != other.row_1 || row_2 != other.row_2 || row_3 != other.row_3;
	}
};

using Matrix3f			= vk2d::Matrix3Base<float>;
using Matrix3d			= vk2d::Matrix3Base<double>;
using Matrix3i			= vk2d::Matrix3Base<int32_t>;
using Matrix3u			= vk2d::Matrix3Base<uint32_t>;



template<typename T>
vk2d::Matrix3Base<T> CreateRotationMatrix3(
	T rotation )
{
	auto x = T( std::cos( rotation ) );
	auto y = T( std::sin( rotation ) );
	return vk2d::Matrix3Base<T>(
		+x, -y, 0,
		+y, +x, 0,
		0,  0,  1
	);
}

} // vk2d
