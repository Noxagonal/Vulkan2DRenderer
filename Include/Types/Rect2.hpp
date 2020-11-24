#pragma once

#include "../Core/Common.h"

#include "Vector2.hpp"

#include <initializer_list>



namespace vk2d {



template<typename T>
class Rect2Base
{
public:

	vk2d::Vector2Base<T>		top_left			= {};
	vk2d::Vector2Base<T>		bottom_right		= {};

	Rect2Base()										= default;
	Rect2Base( const vk2d::Rect2Base<T> & other )	= default;
	Rect2Base( vk2d::Rect2Base<T> && other )		= default;
	Rect2Base( T x1, T y1, T x2, T y2
	) :
		top_left( { x1, y1 } ), bottom_right( { x2, y2 } )
	{}
	Rect2Base( Vector2Base<T> top_left, Vector2Base<T> bottom_right
	) :
		top_left( top_left ),
		bottom_right( bottom_right )
	{}
	Rect2Base( const std::initializer_list<vk2d::Vector2Base<T>> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) top_left		= *e++;
		if( e ) bottom_right	= *e++;
	}
	Rect2Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) top_left.x		= *e++;
		if( e ) top_left.y		= *e++;
		if( e ) bottom_right.x	= *e++;
		if( e ) bottom_right.y	= *e++;
	}

	vk2d::Rect2Base<T> & operator=( const vk2d::Rect2Base<T> & other )	= default;
	vk2d::Rect2Base<T> & operator=( vk2d::Rect2Base<T> && other )		= default;

	vk2d::Rect2Base<T> operator+( vk2d::Vector2Base<T> other ) const
	{
		return { top_left + other, bottom_right + other };
	}
	vk2d::Rect2Base<T> operator-( vk2d::Vector2Base<T> other ) const
	{
		return { top_left - other, bottom_right - other };
	}
	vk2d::Rect2Base<T> & operator+=( vk2d::Vector2Base<T> other )
	{
		top_left += other;
		bottom_right += other;
		return *this;
	}
	vk2d::Rect2Base<T> & operator-=( vk2d::Vector2Base<T> other )
	{
		top_left -= other;
		bottom_right -= other;
		return *this;
	}
	bool operator==( vk2d::Rect2Base<T> other )
	{
		return top_left == other.top_left && bottom_right == other.bottom_right;
	}
	bool operator!=( vk2d::Rect2Base<T> other )
	{
		return top_left != other.top_left || bottom_right != other.bottom_right;
	}



	template<typename PointT>
	bool IsPointInside( vk2d::Vector2Base<PointT> point )
	{
		if( T( point.x ) > top_left.x && T( point.x ) < bottom_right.x &&
			T( point.y ) > top_left.y && T( point.y ) < bottom_right.y ) {
			return true;
		}
		return false;
	}
};

using Rect2f			= Rect2Base<float>;
using Rect2d			= Rect2Base<double>;
using Rect2i			= Rect2Base<int32_t>;
using Rect2u			= Rect2Base<uint32_t>;

} // vk2d
