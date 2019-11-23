#pragma once

#include "../Core/Common.h"

#include "RenderPrimitives.h"

#include <memory>
#include <string>
#include <vector>



namespace vk2d {

namespace _internal {
class RendererImpl;
class WindowImpl;
} // internal



class Renderer;
class TextureResource;
class Mesh;



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
	friend class _internal::RendererImpl;

private:
	// Only accessible through Renderer::CreateWindowOutput();
	VK2D_API																		Window(
		_internal::RendererImpl						*	renderer_parent,
		WindowCreateInfo							&	window_create_info );

public:
	VK2D_API																		~Window();

	VK2D_API bool										VK2D_APIENTRY				BeginRender();
	VK2D_API bool										VK2D_APIENTRY				EndRender();

	VK2D_API void										VK2D_APIENTRY				DrawTriangleList(
		const std::vector<Vertex>					&	vertices,
		const std::vector<VertexIndex_3>			&	indices,
		bool											filled						= true,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLineList(
		const std::vector<Vertex>					&	vertices,
		const std::vector<VertexIndex_2>			&	indices,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawPointList(
		const std::vector<Vertex>					&	vertices,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLine(
		Vector2d										point_1,
		Vector2d										point_2,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawBox(
		Vector2d										top_left,
		Vector2d										bottom_right,
		bool											filled						= true,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawCircle(
		Vector2d										top_left,
		Vector2d										bottom_right,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPie(
		Vector2d										top_left,
		Vector2d										bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPieBox(
		Vector2d										top_left,
		Vector2d										bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawTexture(
		Vector2d										top_left,
		Vector2d										bottom_right,
		vk2d::TextureResource						*	texture,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh );


private:
	std::unique_ptr<_internal::WindowImpl>				impl				= {};

	bool												is_good				= {};
};





}
