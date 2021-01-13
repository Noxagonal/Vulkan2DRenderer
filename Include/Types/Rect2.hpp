#pragma once

#include "Core/Common.h"

#include <initializer_list>
#include <ostream>



namespace vk2d {



/// @brief		This represent axis aligned rectangle area, or AABB depending on the situation.
/// @tparam		T
///				Type of the contained data, depends on the situation.
template<typename T>
class Rect2Base
{
public:

	/// @brief		Top left coordinate.
	glm::vec<2, T, glm::packed_highp>		top_left			= {};

	/// @brief		Bottom right coordinates. This is not size but a coordinate on the same coordinate
	///				space as vk2d::Rect2Base::top_left so this value can be right of or above
	///				vk2d::Rect2Base::top_left, depending on the situation this may be okay, in situations
	///				where top left and bottom right order matters you can use vk2d::Rect2Base::GetOrganized().
	glm::vec<2, T, glm::packed_highp>		bottom_right		= {};

	Rect2Base()										= default;
	Rect2Base( const vk2d::Rect2Base<T> & other )	= default;
	Rect2Base( vk2d::Rect2Base<T> && other )		= default;

	/// @param[in]	x1
	///				Top left: x
	/// @param[in]	y1
	///				Top left: y
	/// @param[in]	x2
	///				Bottom right: x
	/// @param[in]	y2
	///				Bottom right: y
	Rect2Base( T x1, T y1, T x2, T y2
	) :
		top_left( { x1, y1 } ), bottom_right( { x2, y2 } )
	{}
	Rect2Base( glm::vec<2, T, glm::packed_highp> top_left, glm::vec<2, T, glm::packed_highp> bottom_right
	) :
		top_left( top_left ),
		bottom_right( bottom_right )
	{}

	/// @param[in]	elements
	///				Initializer list where elements are ordered
	///				{Top left, Bottom right}
	Rect2Base( const std::initializer_list<glm::vec<2, T, glm::packed_highp>> & elements )
	{
		auto s = elements.size();
		assert( s <= 2 );
		auto e = elements.begin();
		top_left		= ( s >= 1 ) ? *e++ : glm::vec<2, T, glm::packed_highp>{};
		bottom_right	= ( s >= 2 ) ? *e++ : glm::vec<2, T, glm::packed_highp>{};
	}

	/// @param[in]	elements
	///				Initializer list where elements are ordered
	///				{Top left x, Top left y, Bottom right x, Bottom right y}
	Rect2Base( const std::initializer_list<T> & elements )
	{
		auto s = elements.size();
		assert( s <= 4 );
		auto e = elements.begin();
		top_left.x		= ( s >= 1 ) ? *e++ : T{};
		top_left.y		= ( s >= 2 ) ? *e++ : T{};
		bottom_right.x	= ( s >= 3 ) ? *e++ : T{};
		bottom_right.y	= ( s >= 4 ) ? *e++ : T{};
	}

	vk2d::Rect2Base<T> & operator=( const vk2d::Rect2Base<T> & other )	= default;
	vk2d::Rect2Base<T> & operator=( vk2d::Rect2Base<T> && other )		= default;

	/// @brief		Add a 2D vector directly to both top left and bottom right.
	///				Adding this way effectively moves the rectangle in the coordinate space
	///				to a new location without changing it's size.
	/// @param[in]	other
	///				Vector telling where the resulting rectangle should be translated.
	/// @return		A new rectangle.
	vk2d::Rect2Base<T> operator+( glm::vec<2, T, glm::packed_highp> other ) const
	{
		return { top_left + other, bottom_right + other };
	}

	/// @brief		This works exactly the same way as vk2d::Rect2Base::operator+().
	///				Adding this way effectively moves the rectangle in the coordinate space
	///				to a new location without changing it's size, except the vector values
	///				are substracted instead of added.
	/// @param[in]	other
	///				Vector telling where the resulting rectangle should be translated away from.
	/// @return		A new rectangle.
	vk2d::Rect2Base<T> operator-( glm::vec<2, T, glm::packed_highp> other ) const
	{
		return { top_left - other, bottom_right - other };
	}

	/// @brief		Add a 2D vector directly to both top left and bottom right.
	///				Adding this way effectively moves the rectangle in the coordinate space
	///				to a new location without changing it's size.
	/// @param[in]	other
	///				Vector telling where this rectangle should be translated.
	/// @return		Reference to this.
	vk2d::Rect2Base<T> & operator+=( glm::vec<2, T, glm::packed_highp> other )
	{
		top_left += other;
		bottom_right += other;
		return *this;
	}

	/// @brief		This works exactly the same way as vk2d::Rect2Base::operator+=().
	///				Adding this way effectively moves the rectangle in the coordinate space
	///				to a new location without changing it's size, except the vector values
	///				are substracted instead of added.
	/// @param[in]	other
	///				Vector telling where this rectangle should be translated away from.
	/// @return		Reference to this.
	vk2d::Rect2Base<T> & operator-=( glm::vec<2, T, glm::packed_highp> other )
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



	/// @brief		Get the size of the rectangle area. This value is the absolute size so
	///				it's never negative.
	/// @return		Size of the rectangle in 2 dimensions.
	glm::vec<2, T, glm::packed_highp> GetAreaSize()
	{
		return { std::abs( bottom_right.x - top_left.x ), std::abs( bottom_right.y - top_left.y ) };
	}

	/// @brief		Checks if a coordinate is inside this rectangle.
	/// @tparam		PointT
	///				Point can have different type than this rectangle.
	/// @param		point
	///				Point coordinate to check against.
	/// @return		true if point is inside this rectangle, false if point is outside.
	template<typename PointT>
	bool IsPointInside( glm::vec<2, PointT, glm::packed_highp> point )
	{
		if( T( point.x ) > top_left.x && T( point.x ) < bottom_right.x &&
			T( point.y ) > top_left.y && T( point.y ) < bottom_right.y ) {
			return true;
		}
		return false;
	}

	/// @brief		Get organized rectangle where vk2d::Rect2Base::top_left coordinates
	///				are always smaller than vk2d::Rect2Base::bottom_right.
	/// @return		New organized rectangle.
	vk2d::Rect2Base<T> GetOrganized()
	{
		vk2d::Rect2Base<T> ret = *this;
		if( ret.bottom_right.x < ret.top_left.x ) std::swap( ret.bottom_right.x, ret.top_left.x );
		if( ret.bottom_right.y < ret.top_left.y ) std::swap( ret.bottom_right.y, ret.top_left.y );
	}
};

/// @brief		2D rectangle with float precision.
using Rect2f			= Rect2Base<float>;

/// @brief		2D rectangle with double precision.
using Rect2d			= Rect2Base<double>;

/// @brief		2D rectangle with int32_t precision.
using Rect2i			= Rect2Base<int32_t>;

/// @brief		2D rectangle with uint32_t precision.
using Rect2u			= Rect2Base<uint32_t>;

} // vk2d
