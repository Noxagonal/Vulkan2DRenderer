#pragma once

#include "../Core/Common.h"

#include <array>
#include <assert.h>



namespace vk2d {


struct Vector2d {
	float x		= 0.0f;
	float y		= 0.0f;

	Vector2d()								= default;
	Vector2d( float x, float y ) : x( x ), y( y ) {};
	Vector2d( const Vector2d & other )		= default;
	Vector2d( Vector2d && other )			= default;
	Vector2d( const std::initializer_list<float> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
	}

	Vector2d & operator=( const Vector2d & other )	= default;
	Vector2d & operator=( Vector2d && other )		= default;

	Vector2d operator+( Vector2d other ) const
	{
		return { x + other.x, y + other.y };
	}
	Vector2d operator-( Vector2d other ) const
	{
		return { x - other.x, y - other.y };
	}
	Vector2d operator*( Vector2d other ) const
	{
		return { x * other.x, y * other.y };
	}
	Vector2d operator/( Vector2d other ) const
	{
		return { x / other.x, y / other.y };
	}
	Vector2d operator*( float other ) const
	{
		return { x * other, y * other };
	}
	Vector2d operator/( float other ) const
	{
		return { x / other, y / other };
	}
	Vector2d & operator+=( Vector2d other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	Vector2d & operator-=( Vector2d other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Vector2d & operator*=( Vector2d other )
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	Vector2d & operator/=( Vector2d other )
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	Vector2d & operator*=( float other )
	{
		x *= other;
		y *= other;
		return *this;
	}
	Vector2d & operator/=( float other )
	{
		x /= other;
		y /= other;
		return *this;
	}
};

struct AABB2d {
	vk2d::Vector2d				top_left			= {};
	vk2d::Vector2d				bottom_right		= {};

	AABB2d()							= default;
	AABB2d( float x1, float y1, float x2, float y2 ) : top_left( { x1, y1 } ), bottom_right( { x2, y2 } )
	{};
	AABB2d( const AABB2d & other )		= default;
	AABB2d( AABB2d && other )			= default;
	AABB2d( const std::initializer_list<vk2d::Vector2d> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) top_left = *e++;
		if( e ) bottom_right = *e++;
	}

	AABB2d & operator=( const AABB2d & other )	= default;
	AABB2d & operator=( AABB2d && other )		= default;

	AABB2d operator+( vk2d::Vector2d other ) const
	{
		return { top_left + other, bottom_right + other };
	}
	AABB2d operator-( vk2d::Vector2d other ) const
	{
		return { top_left - other, bottom_right - other };
	}
	AABB2d & operator+=( vk2d::Vector2d other )
	{
		top_left += other;
		bottom_right += other;
		return *this;
	}
	AABB2d & operator-=( vk2d::Vector2d other )
	{
		top_left -= other;
		bottom_right -= other;
		return *this;
	}
};

struct Color {
	float r;
	float g;
	float b;
	float a;
};

struct Vertex {
	Vector2d					vertex_coords;
	Vector2d					uv_coords;
	Color						color;
};

struct VertexIndex_2 {
	std::array<uint32_t, 2>		indices;
};

struct VertexIndex_3 {
	std::array<uint32_t, 3>		indices;
};


} // vk2d
