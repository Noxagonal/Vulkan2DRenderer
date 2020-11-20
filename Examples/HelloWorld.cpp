
// This program serves as a hello world of sorts to VK2D.
// It creates a instance, a window and renders 3 basic
// shapes to that window.

// Include VK2D.h header to the project, this will include everything.
#include <VK2D.h>



int main()
{
	// Create VK2D instance, this is the root node of everything else.
	// Return type is std::unique_ptr, this will properly destroy the instance
	// as soon as it goes out of scope and it will clean everything up for us.
	// Every object returned by VK2D is either a pointer or std::unique_ptr,
	// the idea is that if you get a std::unique_ptr then you're responsible
	// for the lifetime of the object, if you get just a simple pointer then
	// VK2D is responsible for the lifetime of the object.
	vk2d::InstanceCreateInfo instance_create_info {};
	auto instance = vk2d::CreateInstance( instance_create_info );
	if( !instance ) return -1;

	// Creating most objects in VK2D requires a lot of parameters, to make it easier
	// to pass those parameters we often use a CreateInfo struct of some sort.
	// Every field in the create info parameter is set to some sensible default
	// so if you're not sure what exactly it does you can in most cases just ignore it.
	// In this case we just want the initial size of 800 x 600 pixels.
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	auto window = instance->CreateOutputWindow( window_create_info );

	// Most VK2D objects can be checked for validity by converting it to bool.
	// Variable "window" in this case is a pointer (VK2D is responsible for this
	// object), CreateOutputWindow will return nullptr if the operation was not
	// successful. In every case however, all of the validity checks boil down
	// to "if( object ) then success else failure".
	if( !window ) return -1;

	// Run the window until the user closes it.
	// Windows do not just close on their own when the close button is pressed,
	// instead they will keep operating normally until the window is destroyed by
	// the application. It's up to the programmer to manually check if the window
	// should be closed and then close it. vk2d::Window::ShouldClose() will return
	// true if the window should be closed.
	// In this case we can just exit the loop and once instance goes out of scope
	// it'll destroy the window for us.

	// Instance::Run() function is a type of periodic update function for the instance.
	// Run() function should be run every frame or in a timely manner, that is, for
	// realtime applications where window is updated at every frame you should also
	// run this function once a frame, or in the case where windows are updated only when
	// needed (eg. editors), you should keep running this at the refresh rate of the
	// monitor or close to it. This is because Instance::Run() polls for input events
	// for all child windows of the instance and also notifies the OS that the application
	// is still alive and not locked up.
	// The return value tells if we can keep running the application or not, true if we
	// can keep running and false if instance wants to close down.

	while( instance->Run() && !window->ShouldClose() ) {
		// From the perspective of this library your program should be structured so
		// that you execute your game logic first and render everything second.
		// This is because BeginRender() synchronizes the frame, (it's a bit more
		// complicated than that internally but...), to overlap CPU and GPU execution
		// as much as possible you should call the BeginRender() as late as possible.

		// Normally your game logic code would go here. However we do not have any
		// logic yet so we'll only do the rendering part.

		// Call Window::BeginRender() before drawing anything.
		// It returns true if we can render and false if something went wrong.
		// This is called a "render block", it consists of the "BeginRender()",
		// drawing commands, and "EndRender()".
		if( !window->BeginRender() ) return -1;

		// Draw a simple rectangle
		window->DrawRectangle(
			vk2d::Vector2f( 200, 200 ),
			vk2d::Vector2f( 700, 400 )
		);

		// Draw an ellipse or a circle.
		window->DrawEllipse(
			vk2d::Vector2f( 450, 200 ),
			vk2d::Vector2f( 750, 500 ),
			true,
			12.0f,
			vk2d::Colorf( 0.2f, 0.5f, 0.8f, 0.2f )
		);

		// Draws an ellipse but with a slice cut out of it.
		window->DrawEllipsePie(
			vk2d::Vector2f( 50, 50 ),
			vk2d::Vector2f( 400, 350 ),
			0.3f,
			0.9f,
			true,
			64.0f,
			vk2d::Colorf( 1.0f, 0.8f, 0.1f, 0.8f )
		);

		// There are more basic types to draw, try them out.

		// Call Window::EndRender() after you're done drawing everything.
		// This actually submits all the work between BeginRender and EndRender
		// to the GPU and displays the results on the window surface once the GPU
		// is done rendering. Note that rendering on the GPU will take some time
		// so you will not see results immediately after this function returns.
		// Resulting image is displayed automatically and asynchronously when
		// it's ready. GPU heavy rendering tasks can be up to 2 frames behind.
		// This function returns true on success and false if something went wrong.
		if( !window->EndRender() ) return -1;
	}

	return 0;
}
