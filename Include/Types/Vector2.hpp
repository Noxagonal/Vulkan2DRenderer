#pragma once

#include "Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



/// @brief		This is a 2D vector containing 2 values in {XY} order. A vector can
///				be a unit vector meaning it's lenght is always 1.0 or it can
///				represent a location in coordinate space.
/// @tparam		T
///				Type precision of this vector.
template<typename T>
class Vector2Base
{
public:

	T x		= {};
	T y		= {};

	Vector2Base()																= default;
	Vector2Base( T x, T y ) : x( x ), y( y ) {};
	Vector2Base( const vk2d::Vector2Base<T> & other )							= default;
	Vector2Base( vk2d::Vector2Base<T> && other )								= default;
	Vector2Base( const std::initializer_list<T> & elements )
	{
		auto s = elements.size();
		assert( s <= 2 );
		auto e = elements.begin();
		x = ( s >= 1 ) ? *e++ : T{};
		y = ( s >= 2 ) ? *e++ : T{};
	}

	vk2d::Vector2Base<T> & operator=( const vk2d::Vector2Base<T> & other )		= default;
	vk2d::Vector2Base<T> & operator=( vk2d::Vector2Base<T> && other )			= default;

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator+( vk2d::Vector2Base<T> other ) const
	{
		return { x + other.x, y + other.y };
	}

	/// @brief		Simple substract from this vector. Directly substracts each value
	///				of this with each value of other.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator-( vk2d::Vector2Base<T> other ) const
	{
		return { x - other.x, y - other.y };
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator*( vk2d::Vector2Base<T> other ) const
	{
		return { x * other.x, y * other.y };
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator/( vk2d::Vector2Base<T> other ) const
	{
		return { x / other.x, y / other.y };
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator*( T scalar ) const
	{
		return { x * scalar, y * scalar };
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A new vector.
	vk2d::Vector2Base<T> operator/( T scalar ) const
	{
		return { x / scalar, y / scalar };
	}

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator+=( vk2d::Vector2Base<T> other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	/// @brief		Simple divide from this vector. Directly substracts each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator-=( vk2d::Vector2Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other and store the result back
	///				to itself.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator*=( vk2d::Vector2Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator/=( vk2d::Vector2Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator*=( T scalar )
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A reference to this.
	vk2d::Vector2Base<T> & operator/=( T scalar )
	{
		x /= scalar;
		y /= scalar;
		return *this;
	}
	bool operator==( vk2d::Vector2Base<T> other )
	{
		return x == other.x && y == other.y;
	}
	bool operator!=( vk2d::Vector2Base<T> other )
	{
		return x != other.x || y != other.y;
	}



	/// @brief		Calculate basic distance between this and another vector.
	///				If vector is integer type then distance is rounded as per math rules.
	/// @param		other
	///				Other vector to calculate distance to.
	/// @return		Distance between the vectors.
	T CalculateDistanceTo( vk2d::Vector2Base<T> other )
	{
		if constexpr( std::is_integral_v<T> ) {
			vk2d::Vector2Base<T> temp = *this - other;
			return T( std::round( std::sqrt(
				double( temp.x ) * double( temp.x ) +
				double( temp.y ) * double( temp.y )
			) ) );
		} else {
			vk2d::Vector2Base<T> temp = *this - other;
			return T( std::sqrt(
				temp.x * temp.x +
				temp.y * temp.y
			) );
		}
	}

	/// @brief		Calculates normalized/unit vector of this. Changes the coordinates
	///				so that the lenght of the vector is 1.0 without changing where the
	///				vector is pointing.
	/// @return		A new vector as unit vector.
	vk2d::Vector2Base<T> CalculateNormalized()
	{
		auto distance = CalculateDistanceTo( {} );
		if( distance <= T( vk2d::KINDA_SMALL_VALUE ) ) return vk2d::Vector2Base<T>{ T( 1.0 ), T( 0.0 ) };
		return *this / distance;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector2Base<T>& v) {
	return os << "[" << v.x << ", " << v.y << "]";
}

/// @brief		2D vector with float precision.
using Vector2f			= vk2d::Vector2Base<float>;

/// @brief		2D vector with double precision.
using Vector2d			= vk2d::Vector2Base<double>;

/// @brief		2D vector with int32_t precision.
using Vector2i			= vk2d::Vector2Base<int32_t>;

/// @brief		2D vector with uint32_t precision.
using Vector2u			= vk2d::Vector2Base<uint32_t>;

} // vk2d
