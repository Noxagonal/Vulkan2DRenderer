#pragma once

#include "Core/Common.h"

#include "Types/Vector2.hpp"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



/// @brief		Column based 2*2 matrix.
/// @tparam		T
///				Matrix precision.
template<typename T>
class Matrix2Base
{
public:

	vk2d::Vector2Base<T>	column_1	= {};
	vk2d::Vector2Base<T>	column_2	= {};

	Matrix2Base()											= default;
	Matrix2Base( const vk2d::Matrix2Base<T> & other )		= default;
	Matrix2Base( vk2d::Matrix2Base<T> && other )			= default;
	Matrix2Base( T identity )
	{
		column_1	= { identity, 0.0f };
		column_2	= { 0.0f, identity };
	}
	Matrix2Base( const std::initializer_list<T> & elements_in_row_major_order )
	{
		assert( elements_in_row_major_order.size() <= 4 );
		auto e = elements_in_row_major_order.begin();
		if( e ) column_1.x = *e++;
		if( e ) column_2.x = *e++;
		if( e ) column_1.y = *e++;
		if( e ) column_2.y = *e++;
	}
	Matrix2Base(
		T c1_r1, T c2_r1,
		T c1_r2, T c2_r2
	)
	{
		column_1	= { c1_r1, c1_r2 };
		column_2	= { c2_r1, c2_r2 };
	}

	vk2d::Matrix2Base<T> & operator=( const vk2d::Matrix2Base<T> & other )		= default;
	vk2d::Matrix2Base<T> & operator=( vk2d::Matrix2Base<T> && other )			= default;

	vk2d::Matrix2Base<T> operator*( T multiplier )
	{
		vk2d::Matrix2Base<T> ret = {};
		ret.column_1	= column_1 * multiplier;
		ret.column_2	= column_2 * multiplier;
		return ret;
	}

	vk2d::Matrix2Base<T> operator*( const vk2d::Matrix2Base<T> & other )
	{
		// a = *this, b = other. number is column first, row second.
		auto & a11 = column_1.x; auto & a21 = column_2.x;
		auto & a12 = column_1.y; auto & a22 = column_2.y;

		auto & b11 = other.column_1.x; auto & b21 = other.column_2.x;
		auto & b12 = other.column_1.y; auto & b22 = other.column_2.y;

		vk2d::Matrix2Base<T> ret;
		// Row 1
		ret.column_1.x	= a11*b11 + a21*b12;
		ret.column_2.x	= a11*b21 + a21*b22;

		// Row 2
		ret.column_1.y	= a12*b11 + a22*b12;
		ret.column_2.y	= a12*b21 + a22*b22;

		return ret;
	}

	vk2d::Vector2Base<T> operator*( const vk2d::Vector2Base<T> & other )
	{
		vk2d::Vector2Base<T> ret;
		ret.x			= column_1.x * other.x + column_2.x * other.y;
		ret.y			= column_1.y * other.x + column_2.y * other.y;
		return ret;
	}

	vk2d::Matrix2Base<T> & operator*=( T multiplier )
	{
		*this	= *this * multiplier;
		return *this;
	}
	vk2d::Matrix2Base<T> & operator*=( const vk2d::Matrix2Base<T> & other )
	{
		*this	= *this * other;
		return *this;
	}
	bool operator==( vk2d::Matrix2Base<T> other )
	{
		return column_1 == other.column_1 && column_2 == other.column_2;
	}
	bool operator!=( vk2d::Matrix2Base<T> other )
	{
		return column_1 != other.column_1 || column_2 != other.column_2;
	}
};

/// @brief		Single precision 2*2 matrix.
using Matrix2f			= vk2d::Matrix2Base<float>;

/// @brief		Double precision 2*2 matrix.
using Matrix2d			= vk2d::Matrix2Base<double>;



/// @brief		Create 2*2 rotation matrix.
/// @tparam		T
///				Matrix precision.
/// @param		rotation
///				Rotation in radians.
/// @return		Rotation matrix.
template<typename T>
vk2d::Matrix2Base<T> CreateRotationMatrix2(
	T rotation )
{
	auto x = T( std::cos( rotation ) );
	auto y = T( std::sin( rotation ) );
	return vk2d::Matrix2Base<T>(
		+x, -y,
		+y, +x
	);
}

} // vk2d
