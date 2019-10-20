#pragma once

#include "Common.h"

#include "RenderPrimitives.h"

#include <memory>
#include <string>
#include <vector>



namespace vk2d {

namespace _internal {
struct WindowDataImpl;
} // internal



class Renderer;



enum class Multisamples : uint32_t {
	SAMPLE_COUNT_1		= 1,
	SAMPLE_COUNT_2		= 2,
	SAMPLE_COUNT_4		= 4,
	SAMPLE_COUNT_8		= 8,
	SAMPLE_COUNT_16		= 16,
	SAMPLE_COUNT_32		= 32,
	SAMPLE_COUNT_64		= 64
};

struct WindowCreateInfo {
	bool			resizeable				= true;			// Can we use the cursor to resize the window
	bool			visible					= true;			// Is the window visible when created
	bool			decorated				= true;			// Does the window have default OS borders and buttons
	bool			focused					= true;			// Is the window focused and brought forth when created
	bool			maximized				= false;		// Is the window maximized to fill the screen when created
	bool			transparent_framebuffer	= false;		// Is the alpha value of the render interpreted as a transparent window background
	uint32_t		width					= 0;			// Window framebuffer initial width
	uint32_t		height					= 0;			// Window framebuffer initial height
	uint32_t		min_width				= 32;			// Minimum width of the window, will be adjusted to suit the hardware
	uint32_t		min_height				= 32;			// Minimum height of the window, will be adjusted to suit the hardware
	uint32_t		max_width				= UINT32_MAX;	// Maximum width of the window, will be adjusted to suit the hardware
	uint32_t		max_height				= UINT32_MAX;	// Maximum height of the window, will be adjusted to suit the hardware
	uint32_t		fullscreen_monitor		= 0;			// Fullscreen monitor index, 0 means windowed, 1 is primary, any value larger than amount of monitors goes to primary monitor
	uint32_t		fullscreen_refresh_rate	= UINT32_MAX;	// Refresh rate in fullscreen mode, UINT32_MAX uses maximum refresh rate available
	bool			vsync					= true;			// Vertical synchronization, works in both windowed and fullscreen modes, usually best left on for 2d graphics
	Multisamples	samples					= Multisamples::SAMPLE_COUNT_1;	// Multisampling, must be a value in Multisamples enum
	std::string		title					= "";			// Window title
};



class Window {
	friend class Renderer;

private:
	VK2D_API													Window(
		Renderer											*	renderer_parent,
		WindowCreateInfo									&	window_create_info );

public:
	VK2D_API													~Window();

	VK2D_API bool				VK2D_APIENTRY					BeginRender();
	VK2D_API bool				VK2D_APIENTRY					EndRender();

	VK2D_API void				VK2D_APIENTRY					Draw_TriangleList(
		bool													filled,
		std::vector<Vertex>									&	vertices,
		std::vector<VertexIndex_3>							&	indices );

	VK2D_API void				VK2D_APIENTRY					Draw_LineList(
		std::vector<Vertex>									&	vertices,
		std::vector<VertexIndex_2>							&	indices );

	VK2D_API void				VK2D_APIENTRY					Draw_PointList(
		std::vector<Vertex>										vertices );

	VK2D_API void				VK2D_APIENTRY					Draw_Line(
		Vertex													point_1,
		Vertex													point_2 );

	VK2D_API void				VK2D_APIENTRY					Draw_Box(
		bool													filled,
		Coords													top_left,
		Coords													bottom_right );

	VK2D_API void				VK2D_APIENTRY					Draw_Circle(
		bool													filled,
		float													edge_count,
		Coords													location,
		float													radius );

	VK2D_API void				VK2D_APIENTRY					Draw_Pie(
		bool													filled,
		float													edge_count,
		Coords													location,
		float													radius,
		float													begin_angle_radians,
		float													end_angle_radians );

private:
	std::unique_ptr<vk2d::_internal::WindowDataImpl>			data				= {};

	bool														is_good				= {};
};





}
