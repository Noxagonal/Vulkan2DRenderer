#pragma once

#include "Common.h"

#include <array>



namespace vk2d {


struct Coords {
	float x;
	float y;
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
