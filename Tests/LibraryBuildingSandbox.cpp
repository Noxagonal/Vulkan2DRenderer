
#include <VK2D.h>
#include "TestCommon.h"

#include <chrono>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;


#include <string>
#include <iostream>


class EventHandler : public vk2d::WindowEventHandler {
public:
	// Keyboard button was pressed, released or kept down ( repeating ).
	virtual void								VK2D_APIENTRY		EventKeyboard(
		vk2d::Window						*	window,
		vk2d::KeyboardButton					button,
		int32_t									scancode,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifierKeys )
	{
		if( action == vk2d::ButtonAction::PRESS ) {
			if( button == vk2d::KeyboardButton::KEY_ESCAPE ) {
				window->CloseWindow();
			}
			if( button == vk2d::KeyboardButton::KEY_F1 ) {
				window->TakeScreenshotToData( true );
			}
		}
	};

	// Screenshot events, called when screenshot save was successfully saved on disk or if there was an error, if error, error message is also given.
	virtual void								VK2D_APIENTRY		EventScreenshotToData(
		vk2d::Window						*	window,
		const vk2d::ImageData				&	image_data,
		bool									success,
		const std::string					&	errorMessage
	)
	{
		std::cout << "Screenshot to data:\nSize: " << image_data.data.size() << "\n";
	};
};


int main()
{
	vk2d::InstanceCreateInfo instance_create_info {};
	auto instance = vk2d::CreateInstance( instance_create_info );
	if( !instance ) return -1;

	EventHandler event_handler;
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	window_create_info.coordinate_space		= vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_8;
	window_create_info.coordinate_space		= vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.event_handler		= &event_handler;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	float counter = 0.0f;
	while( !window->ShouldClose() ) {

		if( !window->BeginRender() ) return -1;

		window->DrawLine(
			vk2d::Vector2f( -200, -200 ),
			vk2d::Vector2f( 200, 200 )
		);

		window->DrawCircle(
			vk2d::Vector2f( -300, -200 ),
			vk2d::Vector2f( 300, 200 ),
			true,
			64,
			vk2d::Colorf( 1, 1, 1, 0.2f )
		);

		window->DrawPoint(
			vk2d::Vector2f( -120, 0 ),
			vk2d::Colorf( 1, 0, 0, 1 ),
			10
		);
		window->DrawPoint(
			vk2d::Vector2f( -110, 0 ),
			vk2d::Colorf( 0, 1, 0, 1 ),
			10
		);
		window->DrawPoint(
			vk2d::Vector2f( -100, 0 ),
			vk2d::Colorf( 0, 0, 1, 1 ),
			10
		);

		if( !window->EndRender() ) return -1;

		counter += 0.01f;
	}

	return 0;
}
