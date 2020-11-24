#pragma once

#include "../Core/Common.h"

#include "Vector2.hpp"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



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
		vk2d::Matrix2Base<T> ret;

		// Cheat sheet...
		//   c1         c2               c1   c2      c1   c2
		//	[ ae + bg ][ af + bh ]	=	[ a ][ b ] * [ e ][ f ]  x
		//	[ ce + dg ][ cf + dh ]	=	[ c ][ d ] * [ g ][ h ]  y

		ret.column_1.x	= column_1.x * other.column_1.x + column_2.x * other.column_1.y;
		ret.column_2.x	= column_1.x * other.column_2.x + column_2.x * other.column_2.y;
		ret.column_1.y	= column_1.y * other.column_1.x + column_2.y * other.column_1.y;
		ret.column_2.y	= column_1.y * other.column_2.x + column_2.y * other.column_2.y;

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

using Matrix2f			= vk2d::Matrix2Base<float>;
using Matrix2d			= vk2d::Matrix2Base<double>;
using Matrix2i			= vk2d::Matrix2Base<int32_t>;
using Matrix2u			= vk2d::Matrix2Base<uint32_t>;



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
