
#include <VK2D.h>

#include <chrono>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;


#include <string>
#include <iostream>


class EventHandler : public vk2d::WindowEventHandler {
public:
	// Keyboard button was pressed, released or kept down ( repeating ).
	void										VK2D_APIENTRY		EventKeyboard(
		vk2d::Window						*	window,
		vk2d::KeyboardButton					button,
		int32_t									scancode,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifierKeys
	)
	{
		if( action == vk2d::ButtonAction::PRESS ) {
			if( button == vk2d::KeyboardButton::KEY_ESCAPE ) {
				window->CloseWindow();
			}
		}
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
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_8;
	window_create_info.event_handler		= &event_handler;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	vk2d::RenderTargetTextureCreateInfo render_target_texture_create_info {};
	render_target_texture_create_info.coordinate_space	= vk2d::RenderCoordinateSpace::TEXEL_SPACE;
	render_target_texture_create_info.size				= vk2d::Vector2u( 512, 512 );
	auto render_target_texture = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);

	float counter = 0.0f;
	while( !window->ShouldClose() ) {

		if( !window->BeginRender() ) return -1;

		if( !window->EndRender() ) return -1;

		counter += 0.01f;
	}

	return 0;
}
