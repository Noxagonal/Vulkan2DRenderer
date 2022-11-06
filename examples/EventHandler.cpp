
// This program shows how to use render target textures.
// 
// Examples to see before trying this example:
// - HelloWorld
//

#include <VK2D.h>



// We create our own event handler class inheriting from vk2d::WindowEventHandler.
// vk2d::WindowEventHandler has some virtual methods we can override, corresponding
// to different events we can catch. These are usually user input, window events
// and events sent from VK2D. All these events are specific to a certain window,
// gamepads for example are not window specific so they're handled differently.
// Below is 3 example overridden functions but there are quite a few more. Check
// vk2d::WindowEventHandler to see more functions that can be overridden.
class EventHandler : public vk2d::WindowEventHandler
{
public:
	void								EventMouseButton(
		vk2d::Window				&	window,
		vk2d::MouseButton				button,
		vk2d::ButtonAction				action,
		vk2d::ModifierKeyFlags			modifier_keys )		override
	{
		if( button == vk2d::MouseButton::BUTTON_LEFT ) {
			if( action == vk2d::ButtonAction::PRESS ) {
				mouse_button_down = true;
			}
			if( action == vk2d::ButtonAction::RELEASE ) {
				mouse_button_down = false;
				mouse_position_1 = mouse_position_2;
			}
		}
	}

	void								EventCursorPosition(
		vk2d::Window				&	window,
		glm::dvec2						position )			override
	{
		if( mouse_button_down ) {
			mouse_position_2 = glm::vec2( float( position.x ), float( position.y ) );
		} else {
			mouse_position_1 = glm::vec2( float( position.x ), float( position.y ) );
			mouse_position_2 = mouse_position_1;
		}
	}

	void								EventKeyboard(
		vk2d::Window				&	window,
		vk2d::KeyboardButton			button,
		int32_t							scancode,
		vk2d::ButtonAction				action,
		vk2d::ModifierKeyFlags			modifier_keys )		override
	{
		// Set window to close if we pressed the ESC key.
		if( action == vk2d::ButtonAction::PRESS && button == vk2d::KeyboardButton::KEY_ESCAPE ) {
			window.CloseWindow();
		}
	}

	glm::vec2							mouse_position_1	= {};
	glm::vec2							mouse_position_2	= {};
	bool								mouse_button_down	= {};
};



int main()
{
	vk2d::InstanceCreateInfo instance_create_info {};
	auto instance = vk2d::CreateInstance( instance_create_info );
	if( !instance ) return -1;

	// We give the event handler to the window here. What we're basically telling
	// to the VK2D is that, with this specific window, we want to use this specific
	// event handler. Single event handler object can be used by multiple windows,
	// in which case they share the same data, however order of what window is
	// processed first is not guaranteed. Different event handler objects of same
	// type can of course be shared by multiple windows in which case different
	// windows share the same behavior but not the same data.
	EventHandler event_handler;
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 1280, 756 };
	window_create_info.event_handler		= &event_handler;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	while( instance->Run() &&  !window->ShouldClose() ) {

		if( !window->BeginRender() ) return -1;

		// Rest of the code is just for visual aid to see the event handler working.
		window->DrawLine(
			{ event_handler.mouse_position_1.x, 0.0f },
			{ event_handler.mouse_position_1.x, float( window->GetSize().y ) }
		);
		window->DrawLine(
			{ 0.0f, event_handler.mouse_position_1.y },
			{ float( window->GetSize().x ), event_handler.mouse_position_1.y }
		);
		window->DrawRectangle(
			{ event_handler.mouse_position_1, event_handler.mouse_position_2 },
			true,
			{ 0.2f, 0.5f, 0.8f, 0.4f }
		);
		window->DrawRectangle(
			{ event_handler.mouse_position_1, event_handler.mouse_position_2 },
			false,
			{ 0.3f, 0.6f, 0.9f, 1.0f }
		);

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
