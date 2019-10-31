#pragma once

#include "Common.h"

#include <array>
#include <assert.h>



namespace vk2d {


struct Coords {
	float x		= 0.0f;
	float y		= 0.0f;

	Coords() = default;
	Coords( float x, float y ) : x( x ), y( y ) {};
	Coords( const Coords & other )	= default;
	Coords( const std::initializer_list<float> & elements )
	{
		assert( elements.size() <= 2 );
		auto e = elements.begin();
		if( e ) x = *e++;
		if( e ) y = *e++;
	}

	Coords & operator=( const Coords & other )	= default;

	Coords operator+( Coords other )
	{
		return { x + other.x, y + other.y };
	}
	Coords operator-( Coords other )
	{
		return { x - other.x, y - other.y };
	}
	Coords operator*( Coords other )
	{
		return { x * other.x, y * other.y };
	}
	Coords operator/( Coords other )
	{
		return { x / other.x, y / other.y };
	}
	Coords operator*( float other )
	{
		return { x * other, y * other };
	}
	Coords operator/( float other )
	{
		return { x / other, y / other };
	}
	Coords & operator+=( Coords other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	Coords & operator-=( Coords other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Coords & operator*=( Coords other )
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	Coords & operator/=( Coords other )
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	Coords & operator*=( float other )
	{
		x *= other;
		y *= other;
		return *this;
	}
	Coords & operator/=( float other )
	{
		x /= other;
		y /= other;
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
	Coords						vertex_coords;
	Coords						uv_coords;
	Color						color;
};

struct VertexIndex_2 {
	std::array<uint32_t, 2>		indices;
};

struct VertexIndex_3 {
	std::array<uint32_t, 3>		indices;
};


} // vk2d
