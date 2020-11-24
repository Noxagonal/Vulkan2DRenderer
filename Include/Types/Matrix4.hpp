#pragma once

#include "../Core/Common.h"

#include "Vector4.hpp"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



template<typename T>
class Matrix4Base
{
public:

	vk2d::Vector4Base<T>	column_1	= {};
	vk2d::Vector4Base<T>	column_2	= {};
	vk2d::Vector4Base<T>	column_3	= {};
	vk2d::Vector4Base<T>	column_4	= {};

	Matrix4Base()											= default;
	Matrix4Base( const vk2d::Matrix4Base<T> & other )		= default;
	Matrix4Base( vk2d::Matrix4Base<T> && other )			= default;
	Matrix4Base( T identity )
	{
		column_1	= { identity, 0.0f, 0.0f, 0.0f };
		column_2	= { 0.0f, identity, 0.0f, 0.0f };
		column_3	= { 0.0f, 0.0f, identity, 0.0f };
		column_4	= { 0.0f, 0.0f, 0.0f, identity };
	}
	Matrix4Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 16 );
		auto e = elements.begin();
		if( e ) column_1.x = *e++;
		if( e ) column_2.x = *e++;
		if( e ) column_3.x = *e++;
		if( e ) column_4.x = *e++;
		if( e ) column_1.y = *e++;
		if( e ) column_2.y = *e++;
		if( e ) column_3.y = *e++;
		if( e ) column_4.y = *e++;
		if( e ) column_1.z = *e++;
		if( e ) column_2.z = *e++;
		if( e ) column_3.z = *e++;
		if( e ) column_4.z = *e++;
		if( e ) column_1.w = *e++;
		if( e ) column_2.w = *e++;
		if( e ) column_3.w = *e++;
		if( e ) column_4.w = *e++;
	}
	Matrix4Base(
		T c1_r1, T c2_r1, T c3_r1, T c4_r1,
		T c1_r2, T c2_r2, T c3_r2, T c4_r2,
		T c1_r3, T c2_r3, T c3_r3, T c4_r3,
		T c1_r4, T c2_r4, T c3_r4, T c4_r4
	)
	{
		column_1	= { c1_r1, c1_r2, c1_r3, c1_r4 };
		column_2	= { c2_r1, c2_r2, c2_r3, c2_r4 };
		column_3	= { c3_r1, c3_r2, c3_r3, c3_r4 };
		column_4	= { c4_r1, c4_r2, c4_r3, c4_r4 };
	}

	vk2d::Matrix4Base<T> & operator=( const vk2d::Matrix4Base<T> & other )		= default;
	vk2d::Matrix4Base<T> & operator=( vk2d::Matrix4Base<T> && other )			= default;

	vk2d::Matrix4Base<T> operator*( T multiplier )
	{
		vk2d::Matrix4Base<T> ret = {};
		ret.column_1		= column_1 * multiplier;
		ret.column_2		= column_2 * multiplier;
		ret.column_3		= column_3 * multiplier;
		ret.column_4		= column_4 * multiplier;
		return ret;
	}

	vk2d::Matrix4Base<T> operator*( const vk2d::Matrix4Base<T> & other )
	{
		// a = *this, b = other. number is column first, row second.
		auto & a11 = column_1.x; auto & a21 = column_2.x; auto & a31 = column_3.x; auto & a41 = column_4.x;
		auto & a12 = column_1.y; auto & a22 = column_2.y; auto & a32 = column_3.y; auto & a42 = column_4.y;
		auto & a13 = column_1.z; auto & a23 = column_2.z; auto & a33 = column_3.z; auto & a43 = column_4.z;
		auto & a14 = column_1.w; auto & a24 = column_2.w; auto & a34 = column_3.w; auto & a44 = column_4.w;

		auto & b11 = other.column_1.x; auto & b21 = other.column_2.x; auto & b31 = other.column_3.x; auto & b41 = other.column_4.x;
		auto & b12 = other.column_1.y; auto & b22 = other.column_2.y; auto & b32 = other.column_3.y; auto & b42 = other.column_4.y;
		auto & b13 = other.column_1.z; auto & b23 = other.column_2.z; auto & b33 = other.column_3.z; auto & b43 = other.column_4.z;
		auto & b14 = other.column_1.w; auto & b24 = other.column_2.w; auto & b34 = other.column_3.w; auto & b44 = other.column_4.w;

		vk2d::Matrix4Base<T> ret;
		// Row 1
		ret.column_1.x	= a11*b11 + a21*b12 + a31*b13 + a41*b14;
		ret.column_2.x	= a11*b21 + a21*b22 + a31*b23 + a41*b24;
		ret.column_3.x	= a11*b31 + a21*b32 + a31*b33 + a41*b34;
		ret.column_4.x	= a11*b41 + a21*b42 + a31*b43 + a41*b44;

		// Row 2
		ret.column_1.y	= a12*b11 + a22*b12 + a32*b13 + a42*b14;
		ret.column_2.y	= a12*b21 + a22*b22 + a32*b23 + a42*b24;
		ret.column_3.y	= a12*b31 + a22*b32 + a32*b33 + a42*b34;
		ret.column_4.y	= a12*b41 + a22*b42 + a32*b43 + a42*b44;

		// Row 3
		ret.column_1.z	= a13*b11 + a23*b12 + a33*b13 + a43*b14;
		ret.column_2.z	= a13*b21 + a23*b22 + a33*b23 + a43*b24;
		ret.column_3.z	= a13*b31 + a23*b32 + a33*b33 + a43*b34;
		ret.column_4.z	= a13*b41 + a23*b42 + a33*b43 + a43*b44;

		// Row 4
		ret.column_1.w	= a14*b11 + a24*b12 + a34*b13 + a44*b14;
		ret.column_2.w	= a14*b21 + a24*b22 + a34*b23 + a44*b24;
		ret.column_3.w	= a14*b31 + a24*b32 + a34*b33 + a44*b34;
		ret.column_4.w	= a14*b41 + a24*b42 + a34*b43 + a44*b44;

		return ret;
	}

	vk2d::Vector4Base<T> operator*( const vk2d::Vector4Base<T> & other )
	{
		vk2d::Vector4Base<T> ret;
		ret.x			= column_1.x * other.x + column_2.x * other.y + column_3.x * other.z + column_4.x * other.w;
		ret.y			= column_1.y * other.x + column_2.y * other.y + column_3.y * other.z + column_4.y * other.w;
		ret.z			= column_1.z * other.x + column_2.z * other.y + column_3.z * other.z + column_4.z * other.w;
		ret.w			= column_1.w * other.x + column_2.w * other.y + column_3.w * other.z + column_4.w * other.w;
		return ret;
	}

	vk2d::Matrix4Base<T> & operator*=( T multiplier )
	{
		*this	= *this * multiplier;
		return *this;
	}
	vk2d::Matrix4Base<T> & operator*=( const vk2d::Matrix4Base<T> & other )
	{
		*this	= *this * other;
		return *this;
	}
	bool operator==( vk2d::Matrix4Base<T> other )
	{
		return column_1 == other.column_1 && column_2 == other.column_2 && column_3 == other.column_3 && column_4 == other.column_4;
	}
	bool operator!=( vk2d::Matrix4Base<T> other )
	{
		return column_1 != other.column_1 || column_2 != other.column_2 || column_3 != other.column_3 || column_4 != other.column_4;
	}
};

using Matrix4f			= vk2d::Matrix4Base<float>;
using Matrix4d			= vk2d::Matrix4Base<double>;
using Matrix4i			= vk2d::Matrix4Base<int32_t>;
using Matrix4u			= vk2d::Matrix4Base<uint32_t>;



template<typename T>
vk2d::Matrix4Base<T> CreateRotationMatrix4(
	T rotation )
{
	auto x = T( std::cos( rotation ) );
	auto y = T( std::sin( rotation ) );
	return vk2d::Matrix4Base<T>(
		+x,		-y,		T( 0 ), T( 0 ),
		+y,		+x,		T( 0 ), T( 0 ),
		T( 0 ), T( 0 ), T( 1 ), T( 0 ),
		T( 0 ), T( 0 ), T( 0 ), T( 1 )
	);
}

} // vk2d
