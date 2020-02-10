
// This program serves as a hello world of sorts to VK2D.
// It creates a instance, a window and renders 3 basic
// shapes to that window.



#include <VK2D.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;


#include <string>
#include <iostream>
#include <chrono>


int main()
{
	// Create instance, this is the root node of everything else.
	// Return type is std::unique_ptr, this will properly destroy the instance
	// as soon as it goes out of scope and it will clean everything up for us.
	vk2d::InstanceCreateInfo instance_create_info {};
	auto instance = vk2d::CreateInstance( instance_create_info );
	if( !instance ) return -1;

	// Create window with size of 800, 600 pixels.
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	auto texture = instance->GetResourceManager()->LoadTextureResource("../../../Data/GrafGear_128.png");

	// Run the window until the user closes it.
	// Windows do not just close on their own when the close button is pressed,
	// instead they will keep operating normally until the window is destroyed by
	// the application. It's up to the programmer to manually check if the window
	// should be closed and then close it. vk2d::Window::ShouldClose() will return
	// true if the window should be closed.
	// In this case we can just exit the loop and once instance goes out of scope
	// it'll destroy the window for us.
	float frame_counter = 0.0f;

	while( !window->ShouldClose() ) {
		frame_counter += 0.03f;
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
			12.0f,
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
