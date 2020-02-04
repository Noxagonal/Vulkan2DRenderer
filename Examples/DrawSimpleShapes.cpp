
// This program serves as a hello world of sorts to VK2D




#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;


#include <string>
#include <iostream>
#include <chrono>


int main()
{
	// Create renderer, this is the root node of everything else.
	// Return type is std::unique_ptr, this will properly destroy the renderer
	// as soon as it goes out of scope and it will clean everything up for us.
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	// Create window with size of 800, 600 pixels.
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	auto window = renderer->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	// Initialize some counters and run the program for 5 seconds before shutting down.
	auto start_time = std::chrono::steady_clock::now();
	while( std::chrono::steady_clock::now() - start_time < std::chrono::seconds( 5 ) ) {

		// General structure of the program is 
		// This area is where you should put your game logic code.

		// Call Window::BeginRender() before drawing anything on screen.
		if( !window->BeginRender() ) return -1;

		window->DrawBox(
			vk2d::Vector2f( 200, 200 ),
			vk2d::Vector2f( 700, 400 )
		);
		window->DrawCircle(
			vk2d::Vector2f( 450, 200 ),
			vk2d::Vector2f( 750, 500 ),
			true,
			64.0f,
			vk2d::Colorf( 0.2f, 0.5f, 0.8f, 0.2f )
		);
		window->DrawPie(
			vk2d::Vector2f( 50, 50 ),
			vk2d::Vector2f( 400, 350 ),
			0.3f,
			0.9f,
			true,
			64.0f,
			vk2d::Colorf( 1.0f, 0.8f, 0.1f, 0.8f )
		);

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
