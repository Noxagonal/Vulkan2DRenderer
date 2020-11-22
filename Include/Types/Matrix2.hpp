#pragma once

#include "../Core/Common.h"

#include "Vector2.hpp"

#include <initializer_list>
#include <cmath>



namespace vk2d {



template<typename T>
class Matrix2Base
{
public:

	vk2d::Vector2Base<T>	row_1	= {};
	vk2d::Vector2Base<T>	row_2	= {};

	Matrix2Base()											= default;
	Matrix2Base( const vk2d::Matrix2Base<T> & other )		= default;
	Matrix2Base( vk2d::Matrix2Base<T> && other )			= default;
	Matrix2Base( float identity )
	{
		row_1	= { identity, 0.0f };
		row_2	= { 0.0f, identity };
	}
	Matrix2Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) row_1.x = *e++;
		if( e ) row_1.y = *e++;
		if( e ) row_2.x = *e++;
		if( e ) row_2.y = *e++;
	}
	Matrix2Base(
		float r1_c1, float r1_c2,
		float r2_c1, float r2_c2
	)
	{
		row_1	= { r1_c1, r1_c2 };
		row_2	= { r2_c1, r2_c2 };
	}

	vk2d::Matrix2Base<T> & operator=( const vk2d::Matrix2Base<T> & other )		= default;
	vk2d::Matrix2Base<T> & operator=( vk2d::Matrix2Base<T> && other )			= default;

	vk2d::Matrix2Base<T> operator*( T multiplier )
	{
		vk2d::Matrix2Base<T> ret = {};
		ret.row_1		= row_1 * multiplier;
		ret.row_2		= row_2 * multiplier;
		return ret;
	}

	vk2d::Matrix2Base<T> operator*( const vk2d::Matrix2Base<T> & other )
	{
		vk2d::Matrix2Base<T> ret = *this;
		ret.row_1.x		= row_1.x * other.row_1.x + row_1.y * other.row_2.x;
		ret.row_1.y		= row_1.x * other.row_1.y + row_1.y * other.row_2.y;
		ret.row_2.x		= row_2.x * other.row_1.x + row_2.y * other.row_2.x;
		ret.row_2.y		= row_2.x * other.row_1.y + row_2.y * other.row_2.y;
		return ret;
	}

	vk2d::Vector2Base<T> operator*( const vk2d::Vector2Base<T> & other )
	{
		vk2d::Vector2Base<T> ret = {};
		ret.x			= row_1.x * other.x + row_1.y * other.y;
		ret.y			= row_2.x * other.x + row_2.y * other.y;
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
		return row_1 == other.row_1 && row_2 == other.row_2;
	}
	bool operator!=( vk2d::Matrix2Base<T> other )
	{
		return row_1 != other.row_1 || row_2 != other.row_2;
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
