#pragma once

#include "../Core/Common.h"

#include "Vector3.hpp"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



template<typename T>
class Matrix3Base
{
public:

	vk2d::Vector3Base<T>	column_1	= {};
	vk2d::Vector3Base<T>	column_2	= {};
	vk2d::Vector3Base<T>	column_3	= {};

	Matrix3Base()											= default;
	Matrix3Base( const vk2d::Matrix3Base<T> & other )		= default;
	Matrix3Base( vk2d::Matrix3Base<T> && other )			= default;
	Matrix3Base( T identity )
	{
		column_1	= { identity, 0.0f, 0.0f };
		column_2	= { 0.0f, identity, 0.0f };
		column_3	= { 0.0f, 0.0f, identity };
	}
	Matrix3Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 9 );
		auto e = elements.begin();
		if( e ) column_1.x = *e++;
		if( e ) column_2.x = *e++;
		if( e ) column_3.x = *e++;
		if( e ) column_1.y = *e++;
		if( e ) column_2.y = *e++;
		if( e ) column_3.y = *e++;
		if( e ) column_1.z = *e++;
		if( e ) column_2.z = *e++;
		if( e ) column_3.z = *e++;
	}
	Matrix3Base(
		T c1_r1, T c2_r1, T c3_r1,
		T c1_r2, T c2_r2, T c3_r2,
		T c1_r3, T c2_r3, T c3_r3
	)
	{
		column_1	= { c1_r1, c1_r2, c1_r3 };
		column_2	= { c2_r1, c2_r2, c2_r3 };
		column_3	= { c3_r1, c3_r2, c3_r3 };
	}

	vk2d::Matrix3Base<T> & operator=( const vk2d::Matrix3Base<T> & other )		= default;
	vk2d::Matrix3Base<T> & operator=( vk2d::Matrix3Base<T> && other )			= default;

	vk2d::Matrix3Base<T> operator*( T multiplier )
	{
		vk2d::Matrix3Base<T> ret = {};
		ret.column_1		= column_1 * multiplier;
		ret.column_2		= column_2 * multiplier;
		ret.column_3		= column_3 * multiplier;
		return ret;
	}

	vk2d::Matrix3Base<T> operator*( const vk2d::Matrix3Base<T> & other )
	{
		vk2d::Matrix3Base<T> ret;

		// Cheat sheet...
		//    c1              c2              c3                   c1   c2   c3      c1   c2   c3
		//	[ aj + bm + cp ][ ak + bn + cq ][ al + bo + cr ]  	= [ a ][ b ][ c ] * [ j ][ k ][ l ]  x
		//	[ dj + em + fp ][ dk + en + fq ][ dl + eo + fr ]  	= [ d ][ e ][ f ] * [ m ][ n ][ o ]  y
		//	[ gj + hm + ip ][ gk + hn + iq ][ gl + ho + ir ]  	= [ g ][ h ][ i ] * [ p ][ q ][ r ]  z

		ret.column_1.x	= column_1.x * other.column_1.x + column_2.x * other.column_1.y + column_3.x * other.column_1.z;
		ret.column_2.x	= column_1.x * other.column_2.x + column_2.x * other.column_2.y + column_3.x * other.column_2.z;
		ret.column_3.x	= column_1.x * other.column_3.x + column_2.x * other.column_3.y + column_3.x * other.column_3.z;
		ret.column_1.y	= column_1.y * other.column_1.x + column_2.y * other.column_1.y + column_3.y * other.column_1.z;
		ret.column_2.y	= column_1.y * other.column_2.x + column_2.y * other.column_2.y + column_3.y * other.column_2.z;
		ret.column_3.y	= column_1.y * other.column_3.x + column_2.y * other.column_3.y + column_3.y * other.column_3.z;
		ret.column_1.z	= column_1.z * other.column_1.x + column_2.z * other.column_1.y + column_3.z * other.column_1.z;
		ret.column_2.z	= column_1.z * other.column_2.x + column_2.z * other.column_2.y + column_3.z * other.column_2.z;
		ret.column_3.z	= column_1.z * other.column_3.x + column_2.z * other.column_3.y + column_3.z * other.column_3.z;

		return ret;
	}

	vk2d::Vector3Base<T> operator*( const vk2d::Vector3Base<T> & other )
	{
		vk2d::Vector3Base<T> ret;
		ret.x			= column_1.x * other.x + column_2.x * other.y + column_3.x * other.z;
		ret.y			= column_1.y * other.x + column_2.y * other.y + column_3.y * other.z;
		ret.z			= column_1.z * other.x + column_2.z * other.y + column_3.z * other.z;
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
		return column_1 == other.column_1 && column_2 == other.column_2 && column_3 == other.column_3;
	}
	bool operator!=( vk2d::Matrix3Base<T> other )
	{
		return column_1 != other.column_1 || column_2 != other.column_2 || column_3 != other.column_3;
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
		+x,		-y,		T( 0 ),
		+y,		+x,		T( 0 ),
		T( 0 ),	T( 0 ),	T( 1 )
	);
}

} // vk2d
