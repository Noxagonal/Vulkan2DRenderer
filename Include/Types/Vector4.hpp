#pragma once

#include "Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <assert.h>
#include <ostream>



namespace vk2d {



/// @brief		This is a 4D vector containing 4 values in {XYZW} order. A vector can
///				be a unit vector meaning it's lenght is always 1.0 or it can
///				represent a location in coordinate space.
/// @tparam		T
///				Type precision of this vector.
template<typename T>
class Vector4Base
{
public:

	T x		= {};
	T y		= {};
	T z		= {};
	T w		= {};

	Vector4Base()																= default;
	Vector4Base( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {};
	Vector4Base( const vk2d::Vector4Base<T> & other )							= default;
	Vector4Base( vk2d::Vector4Base<T> && other )								= default;
	Vector4Base( const std::initializer_list<T> & elements )
	{
		auto s = elements.size();
		assert( s <= 4 );
		auto e = elements.begin();
		x = ( s >= 1 ) ? *e++ : T{};
		y = ( s >= 2 ) ? *e++ : T{};
		z = ( s >= 3 ) ? *e++ : T{};
		w = ( s >= 4 ) ? *e++ : T{};
	}

	vk2d::Vector4Base<T> & operator=( const vk2d::Vector4Base<T> & other )		= default;
	vk2d::Vector4Base<T> & operator=( vk2d::Vector4Base<T> && other )			= default;

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator+( vk2d::Vector4Base<T> other ) const
	{
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}

	/// @brief		Simple substract from this vector. Directly substracts each value
	///				of this with each value of other.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator-( vk2d::Vector4Base<T> other ) const
	{
		return { x - other.x, y - other.y, z - other.z, w - other.w };
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator*( vk2d::Vector4Base<T> other ) const
	{
		return { x * other.x, y * other.y, z * other.z, w * other.w };
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator/( vk2d::Vector4Base<T> other ) const
	{
		return { x / other.x, y / other.y, z / other.z, w / other.w };
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator*( T scalar ) const
	{
		return { x * scalar, y * scalar, z * scalar, w * scalar };
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A new vector.
	vk2d::Vector4Base<T> operator/( T scalar ) const
	{
		return { x / scalar, y / scalar, z / scalar, w / scalar };
	}

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator+=( vk2d::Vector4Base<T> other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	/// @brief		Simple divide from this vector. Directly substracts each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator-=( vk2d::Vector4Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other and store the result back
	///				to itself.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator*=( vk2d::Vector4Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator/=( vk2d::Vector4Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator*=( T scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A reference to this.
	vk2d::Vector4Base<T> & operator/=( T scalar )
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	}
	bool operator==( vk2d::Vector4Base<T> other )
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
	bool operator!=( vk2d::Vector4Base<T> other )
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}



	/// @brief		Calculate basic distance between this and another vector.
	///				If vector is integer type then distance is rounded as per math rules.
	/// @param		other
	///				Other vector to calculate distance to.
	/// @return		Distance between the vectors.
	T CalculateDistanceTo( vk2d::Vector4Base<T> other )
	{
		if constexpr( std::is_integral_v<T> ) {
			vk2d::Vector4Base<T> temp = *this - other;
			return T( std::round( std::sqrt(
				double( temp.x ) * double( temp.x ) +
				double( temp.y ) * double( temp.y ) +
				double( temp.z ) * double( temp.z ) +
				double( temp.w ) * double( temp.w )
			) ) );
		} else {
			vk2d::Vector4Base<T> temp = *this - other;
			return T( std::sqrt(
				temp.x * temp.x +
				temp.y * temp.y +
				temp.z * temp.z +
				temp.w * temp.w
			) );
		}
	}

	/// @brief		Calculates normalized/unit vector of this. Changes the coordinates
	///				so that the lenght of the vector is 1.0 without changing where the
	///				vector is pointing.
	/// @return		A new vector as unit vector.
	vk2d::Vector4Base<T> CalculateNormalized()
	{
		auto distance = CalculateDistanceTo( {} );
		if( distance <= T( vk2d::KINDA_SMALL_VALUE ) ) return vk2d::Vector4Base<T>{ T( 1.0 ), T( 0.0 ), T( 0.0 ), T( 0.0 ) };
		return *this / distance;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector4Base<T>& v) {
	return os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
}

/// @brief		4D vector with float precision.
using Vector4f			= vk2d::Vector4Base<float>;

/// @brief		4D vector with double precision.
using Vector4d			= vk2d::Vector4Base<double>;

/// @brief		4D vector with int32_t precision.
using Vector4i			= vk2d::Vector4Base<int32_t>;

/// @brief		4D vector with uint32_t precision.
using Vector4u			= vk2d::Vector4Base<uint32_t>;

} // vk2d
