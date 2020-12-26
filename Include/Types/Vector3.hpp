#pragma once

#include "Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



/// @brief		This is a 3D vector containing 3 values in {XYZ} order. A vector can
///				be a unit vector meaning it's lenght is always 1.0 or it can
///				represent a location in coordinate space.
/// @tparam		T
///				Type precision of this vector.
template<typename T>
class Vector3Base
{
public:

	T x		= {};
	T y		= {};
	T z		= {};

	Vector3Base()																= default;
	Vector3Base( T x, T y, T z ) : x( x ), y( y ), z( z ) {};
	Vector3Base( const vk2d::Vector3Base<T> & other )							= default;
	Vector3Base( vk2d::Vector3Base<T> && other )								= default;
	Vector3Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 3 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
		if( e ) z = *e++;
	}

	vk2d::Vector3Base<T> & operator=( const vk2d::Vector3Base<T> & other )		= default;
	vk2d::Vector3Base<T> & operator=( vk2d::Vector3Base<T> && other )			= default;

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator+( vk2d::Vector3Base<T> other ) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	/// @brief		Simple substract from this vector. Directly substracts each value
	///				of this with each value of other.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator-( vk2d::Vector3Base<T> other ) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator*( vk2d::Vector3Base<T> other ) const
	{
		return { x * other.x, y * other.y, z * other.z };
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator/( vk2d::Vector3Base<T> other ) const
	{
		return { x / other.x, y / other.y, z / other.z };
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator*( T scalar ) const
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A new vector.
	vk2d::Vector3Base<T> operator/( T scalar ) const
	{
		return { x / scalar, y / scalar, z / scalar };
	}

	/// @brief		Simple add vector to vector. Directly adds each value of this with
	///				each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to add to this.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator+=( vk2d::Vector3Base<T> other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	/// @brief		Simple divide from this vector. Directly substracts each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to substract from this.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator-=( vk2d::Vector3Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	/// @brief		Simple multiplication of this and other. Directly multiplies each
	///				value of this with each value of other and store the result back
	///				to itself.
	/// @param[in]	other
	///				Other vector to multiply this with.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator*=( vk2d::Vector3Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	/// @brief		Simple division of this and other. Directly divide each value of
	///				this with each value of other and store the result back to itself.
	/// @param[in]	other
	///				Other vector to devide with this.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator/=( vk2d::Vector3Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	/// @brief		Simple multiplication with a single value. Directly multiply each
	///				value of this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Multiply this with scalar.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator*=( T scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	/// @brief		Simple division with a single value. Directly divide each value of
	///				this with scalar and store the result back to itself.
	/// @param[in]	scalar
	///				Divide this vector with scalar.
	/// @return		A reference to this.
	vk2d::Vector3Base<T> & operator/=( T scalar )
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
	bool operator==( vk2d::Vector3Base<T> other )
	{
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=( vk2d::Vector3Base<T> other )
	{
		return x != other.x || y != other.y || z != other.z;
	}



	/// @brief		Calculate basic distance between this and another vector.
	///				If vector is integer type then distance is rounded as per math rules.
	/// @param		other
	///				Other vector to calculate distance to.
	/// @return		Distance between the vectors.
	T CalculateDistanceTo( vk2d::Vector3Base<T> other )
	{
		if constexpr( std::is_integral_v<T> ) {
			vk2d::Vector3Base<T> temp = *this - other;
			return T( std::round( std::sqrt(
				double( temp.x ) * double( temp.x ) +
				double( temp.y ) * double( temp.y ) +
				double( temp.z ) * double( temp.z )
			) ) );
		} else {
			vk2d::Vector3Base<T> temp = *this - other;
			return T( std::sqrt(
				temp.x * temp.x +
				temp.y * temp.y +
				temp.z * temp.z
			) );
		}
	}

	/// @brief		Calculates normalized/unit vector of this. Changes the coordinates
	///				so that the lenght of the vector is 1.0 without changing where the
	///				vector is pointing.
	/// @return		A new vector as unit vector.
	vk2d::Vector3Base<T> CalculateNormalized()
	{
		auto distance = CalculateDistanceTo( {} );
		if( distance <= T( vk2d::KINDA_SMALL_VALUE ) ) return vk2d::Vector3Base<T>{ T( 1.0 ), T( 0.0 ), T( 0.0 ) };
		return *this / distance;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector3Base<T>& v) {
	return os << v.x << "," << v.y << "," << v.z;
}

/// @brief		3D vector with float precision.
using Vector3f			= vk2d::Vector3Base<float>;

/// @brief		3D vector with double precision.
using Vector3d			= vk2d::Vector3Base<double>;

/// @brief		3D vector with int32_t precision.
using Vector3i			= vk2d::Vector3Base<int32_t>;

/// @brief		3D vector with uint32_t precision.
using Vector3u			= vk2d::Vector3Base<uint32_t>;

} // vk2d
