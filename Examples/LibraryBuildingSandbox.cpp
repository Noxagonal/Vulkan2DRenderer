
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>
#include <sstream>



constexpr double PI		= 3.14159265358979323846;
constexpr double RAD	= PI * 2.0;

float blur_test_value_direction		= 0.0f;
float blur_test_value				= 0.0f;


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
		if( button == vk2d::KeyboardButton::KEY_UP ) {
			if( action == vk2d::ButtonAction::PRESS ) {
				blur_test_value_direction = 1.0f;
			} else if( action == vk2d::ButtonAction::RELEASE ) {
				blur_test_value_direction = 0.0f;
			}
		}
		if( button == vk2d::KeyboardButton::KEY_DOWN ) {
			if( action == vk2d::ButtonAction::PRESS ) {
				blur_test_value_direction = -1.0f;
			} else if( action == vk2d::ButtonAction::RELEASE ) {
				blur_test_value_direction = 0.0f;
			}
		}
	};
};



// Simple class to calculate delta time.
class DeltaTimeCounter
{
public:
	DeltaTimeCounter()
	{
		last_time_point	= std::chrono::steady_clock::now();
	}

	// Tick once, returns delta time since last tick.
	float Tick()
	{
		auto now		= std::chrono::steady_clock::now();
		auto delta_time	= std::chrono::duration<float>( now - last_time_point ).count();
		last_time_point	= now;
		return delta_time;
	}

	std::chrono::steady_clock::time_point	last_time_point;
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
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_16;
	window_create_info.event_handler		= &event_handler;
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	auto resource_manager		= instance->GetResourceManager();
	auto texture_resource		= resource_manager->LoadTextureResource( "../../Data/GrafGear_128.png" );

	auto box					= vk2d::GenerateRectangleMesh(
		{ -50.0f, -50.0f, 50.0f, 50.0f },
		false
	);

	auto delta_time_counter		= DeltaTimeCounter();
	auto delta_time				= 0.0f;
	auto seconds_since_start	= 0.0f;

	while( instance->Run() && !window1->ShouldClose() ) {
		delta_time				= delta_time_counter.Tick();
		seconds_since_start		+= delta_time;

		if( !window1->BeginRender() ) return -1;

		auto origin = vk2d::Transform(
			{ 0.0f, 0.0f },
			{ std::sin( seconds_since_start ) * 0.5f + 1.0f, 1.0f },
			seconds_since_start / 3.0f
		);
		auto origin_matrix = origin.CalculateTransformationMatrix();

		auto child = vk2d::Transform(
			{ 150.0f, 0.0f },
			{ 1.0f, 1.0f },
			0.0f
		);
		auto child_matrix = child.CalculateTransformationMatrix();

		auto sub_child = vk2d::Transform(
			{ 0.0f, std::sin( seconds_since_start / 2.0f ) * 150.0f },
			{ 1.0f, 1.0f },
			std::sin( seconds_since_start )
		);
		auto sub_child_matrix = sub_child.CalculateTransformationMatrix();

		auto child_transform		= origin_matrix * child_matrix;
		auto sub_child_transform	= origin_matrix * child_matrix * sub_child_matrix;

		window1->DrawMesh( box, origin );
		window1->DrawMesh( box, { child_transform, sub_child_transform } );

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
