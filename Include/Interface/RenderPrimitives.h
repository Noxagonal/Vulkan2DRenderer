#pragma once

#include "../Core/Common.h"

#include <initializer_list>
#include <cmath>
#include <array>
#include <assert.h>



namespace vk2d {



template<typename T>
struct Vector2Base {
	T x		= {};
	T y		= {};

	Vector2Base()																= default;
	Vector2Base( T x, T y ) : x( x ), y( y ) {};
	Vector2Base( const vk2d::Vector2Base<T> & other )							= default;
	Vector2Base( vk2d::Vector2Base<T> && other )								= default;
	Vector2Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
	}

	vk2d::Vector2Base<T> & operator=( const vk2d::Vector2Base<T> & other )	= default;
	vk2d::Vector2Base<T> & operator=( vk2d::Vector2Base<T> && other )			= default;

	vk2d::Vector2Base<T> operator+( vk2d::Vector2Base<T> other ) const
	{
		return { x + other.x, y + other.y };
	}
	vk2d::Vector2Base<T> operator-( vk2d::Vector2Base<T> other ) const
	{
		return { x - other.x, y - other.y };
	}
	vk2d::Vector2Base<T> operator*( vk2d::Vector2Base<T> other ) const
	{
		return { x * other.x, y * other.y };
	}
	vk2d::Vector2Base<T> operator/( vk2d::Vector2Base<T> other ) const
	{
		return { x / other.x, y / other.y };
	}
	vk2d::Vector2Base<T> operator*( T other ) const
	{
		return { x * other, y * other };
	}
	vk2d::Vector2Base<T> operator/( T other ) const
	{
		return { x / other, y / other };
	}
	vk2d::Vector2Base<T> & operator+=( vk2d::Vector2Base<T> other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator-=( vk2d::Vector2Base<T> other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator*=( vk2d::Vector2Base<T> other )
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator/=( vk2d::Vector2Base<T> other )
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	vk2d::Vector2Base<T> & operator*=( T other )
	{
		x *= other;
		y *= other;
		return *this;
	}
	vk2d::Vector2Base<T> & operator/=( T other )
	{
		x /= other;
		y /= other;
		return *this;
	}
};

using Vector2f			= vk2d::Vector2Base<float>;
using Vector2d			= vk2d::Vector2Base<double>;
using Vector2i			= vk2d::Vector2Base<int32_t>;
using Vector2u			= vk2d::Vector2Base<uint32_t>;



template<typename T>
struct Matrix2Base {
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
		vk2d::Vector2f ret = {};
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
};

using Matrix2f			= vk2d::Matrix2Base<float>;
using Matrix2d			= vk2d::Matrix2Base<double>;
using Matrix2i			= vk2d::Matrix2Base<int32_t>;
using Matrix2u			= vk2d::Matrix2Base<uint32_t>;



inline vk2d::Matrix2f CreateRotationMatrix(
	float rotation
)
{
	auto x = std::cos( rotation );
	auto y = std::sin( rotation );
	return vk2d::Matrix2f(
		+x, -y,
		+y, +x
	);
}



template<typename T>
struct AABB2Base {
	vk2d::Vector2Base<T>		top_left			= {};
	vk2d::Vector2Base<T>		bottom_right		= {};

	AABB2Base()									= default;
	AABB2Base( T x1, T y1, T x2, T y2
	) :
		top_left( { x1, y1 } ), bottom_right( { x2, y2 } )
	{};
	AABB2Base( const vk2d::AABB2Base<T> & other )	= default;
	AABB2Base( vk2d::AABB2Base<T> && other )		= default;
	AABB2Base( const std::initializer_list<vk2d::Vector2Base<T>> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) top_left		= *e++;
		if( e ) bottom_right	= *e++;
	}
	AABB2Base( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) top_left.x		= *e++;
		if( e ) top_left.y		= *e++;
		if( e ) bottom_right.x	= *e++;
		if( e ) bottom_right.y	= *e++;
	}

	vk2d::AABB2Base<T> & operator=( const vk2d::AABB2Base<T> & other )	= default;
	vk2d::AABB2Base<T> & operator=( vk2d::AABB2Base<T> && other )			= default;

	vk2d::AABB2Base<T> operator+( vk2d::Vector2Base<T> other ) const
	{
		return { top_left + other, bottom_right + other };
	}
	vk2d::AABB2Base<T> operator-( vk2d::Vector2Base<T> other ) const
	{
		return { top_left - other, bottom_right - other };
	}
	vk2d::AABB2Base<T> & operator+=( vk2d::Vector2Base<T> other )
	{
		top_left += other;
		bottom_right += other;
		return *this;
	}
	vk2d::AABB2Base<T> & operator-=( vk2d::Vector2Base<T> other )
	{
		top_left -= other;
		bottom_right -= other;
		return *this;
	}
};

using AABB2f			= AABB2Base<float>;
using AABB2d			= AABB2Base<double>;
using AABB2i			= AABB2Base<int32_t>;
using AABB2u			= AABB2Base<uint32_t>;



template<typename T>
struct ColorBase {
	T r			= {};
	T g			= {};
	T b			= {};
	T a			= {};

	ColorBase()										= default;
	ColorBase( const vk2d::ColorBase<T> & other )	= default;
	ColorBase( vk2d::ColorBase<T> && other )		= default;
	ColorBase( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) r = *e++;
		if( e ) g = *e++;
		if( e ) b = *e++;
		if( e ) a = *e++;
	}
	ColorBase( T r, T g, T b, T a ) :
		r( r ),
		g( g ),
		b( b ),
		a( a )
	{};

	vk2d::ColorBase<T> & operator=( const vk2d::ColorBase<T> & other )	= default;
	vk2d::ColorBase<T> & operator=( vk2d::ColorBase<T> && other )		= default;
};

using Colorf			= vk2d::ColorBase<float>;
using Colord			= vk2d::ColorBase<double>;
using Color8			= vk2d::ColorBase<uint8_t>;
using Color16			= vk2d::ColorBase<uint16_t>;
using Color32			= vk2d::ColorBase<uint32_t>;



struct Vertex {
	vk2d::Vector2f				vertex_coords;			// 2 x 32 bits
	vk2d::Vector2f				uv_coords;				// 2 x 32 bits
	vk2d::Colorf				color;					// 4 x 32 bits
	float						point_size;				// 1 x 32 bits
private:
	float						padding[ 3 ];			// Reserved, padding Vertex struct to 128 bit boundaries
};

struct VertexIndex_2 {
	std::array<uint32_t, 2>		indices;
};

struct VertexIndex_3 {
	std::array<uint32_t, 3>		indices;
};


} // vk2d
