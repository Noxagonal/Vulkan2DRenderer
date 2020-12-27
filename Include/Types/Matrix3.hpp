#pragma once

#include "Core/Common.h"

#include "Types/Vector3.hpp"

#include <initializer_list>
#include <cmath>
#include <assert.h>



namespace vk2d {



/// @brief		Column based 3*3 matrix.
/// @tparam		T
///				Matrix precision.
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
	Matrix3Base( const std::initializer_list<T> & elements_in_row_major_order )
	{
		auto s = elements_in_row_major_order.size();
		assert( s <= 9 );
		auto e = elements_in_row_major_order.begin();
		column_1.x = ( s >= 1 ) ? *e++ : T{};
		column_2.x = ( s >= 2 ) ? *e++ : T{};
		column_3.x = ( s >= 3 ) ? *e++ : T{};
		column_1.y = ( s >= 4 ) ? *e++ : T{};
		column_2.y = ( s >= 5 ) ? *e++ : T{};
		column_3.y = ( s >= 6 ) ? *e++ : T{};
		column_1.z = ( s >= 7 ) ? *e++ : T{};
		column_2.z = ( s >= 8 ) ? *e++ : T{};
		column_3.z = ( s >= 9 ) ? *e++ : T{};
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
		// a = *this, b = other. number is column first, row second.
		auto & a11 = column_1.x; auto & a21 = column_2.x; auto & a31 = column_3.x;
		auto & a12 = column_1.y; auto & a22 = column_2.y; auto & a32 = column_3.y;
		auto & a13 = column_1.z; auto & a23 = column_2.z; auto & a33 = column_3.z;

		auto & b11 = other.column_1.x; auto & b21 = other.column_2.x; auto & b31 = other.column_3.x;
		auto & b12 = other.column_1.y; auto & b22 = other.column_2.y; auto & b32 = other.column_3.y;
		auto & b13 = other.column_1.z; auto & b23 = other.column_2.z; auto & b33 = other.column_3.z;

		vk2d::Matrix3Base<T> ret;
		// Row 1
		ret.column_1.x	= a11*b11 + a21*b12 + a31*b13;
		ret.column_2.x	= a11*b21 + a21*b22 + a31*b23;
		ret.column_3.x	= a11*b31 + a21*b32 + a31*b33;

		// Row 2
		ret.column_1.y	= a12*b11 + a22*b12 + a32*b13;
		ret.column_2.y	= a12*b21 + a22*b22 + a32*b23;
		ret.column_3.y	= a12*b31 + a22*b32 + a32*b33;

		// Row 3
		ret.column_1.z	= a13*b11 + a23*b12 + a33*b13;
		ret.column_2.z	= a13*b21 + a23*b22 + a33*b23;
		ret.column_3.z	= a13*b31 + a23*b32 + a33*b33;

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

	/// @brief		Get matrix as formatted multi-line text.
	/// @param[in]	field_lenght
	///				Maximum number of string characters each field should occupy.
	/// @return		Text representation of the matrix.
	std::string AsFormattedText( uint32_t field_lenght )
	{
		auto value_str = [field_lenght]( T value ) -> std::string
		{
			std::stringstream tss;
			tss << value;
			auto str = tss.str().substr( 0, field_lenght );
			if( str.back() == '.' ) str = str.substr( 0, field_lenght - 1 );
			return str;
		};

		std::stringstream ss;
		ss << "[";
		ss << std::setw( field_lenght + 1 ) << value_str( column_1.x ) << ",";
		ss << std::setw( field_lenght + 1 ) << value_str( column_2.x ) << ",";
		ss << std::setw( field_lenght + 2 ) << value_str( column_3.x ) << " ]\n";

		ss << "[";
		ss << std::setw( field_lenght + 1 ) << value_str( column_1.y ) << ",";
		ss << std::setw( field_lenght + 1 ) << value_str( column_2.y ) << ",";
		ss << std::setw( field_lenght + 2 ) << value_str( column_3.y ) << " ]\n";

		ss << "[";
		ss << std::setw( field_lenght + 1 ) << value_str( column_1.z ) << ",";
		ss << std::setw( field_lenght + 1 ) << value_str( column_2.z ) << ",";
		ss << std::setw( field_lenght + 2 ) << value_str( column_3.z ) << " ]\n";

		return ss.str();
	}
};

/// @brief		C++ std::ostream<< operator, Column per column order.
/// @tparam		T
///				Matrix precision.
/// @param[in,out]	os
///				ostream.
/// @param		m
///				Reference to a matrix.
/// @return		Reference to "os" parameter.
template<typename T>
std::ostream & operator<<( std::ostream & os, const Matrix3Base<T> & v )
{
	return os << "[" << column_1 << ", " << column_2 << ", " << column_3 << "]";
}

/// @brief		Single precision 3*3 matrix.
using Matrix3f			= vk2d::Matrix3Base<float>;

/// @brief		Double precision 3*3 matrix.
using Matrix3d			= vk2d::Matrix3Base<double>;



/// @brief		Create 3*3 rotation matrix.
/// @tparam		T
///				Matrix precision.
/// @param		rotation
///				Rotation in radians.
/// @return		Rotation matrix.
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
