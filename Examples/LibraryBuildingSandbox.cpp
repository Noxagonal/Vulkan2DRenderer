
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>
#include <sstream>



uint32_t frame = 0;



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
			if( button == vk2d::KeyboardButton::KEY_PRINT_SCREEN ) {
				/*
				std::stringstream ss;
				ss << "../../Docs/Images/SamplerLODBias_";
				ss << std::setfill( '0' ) << std::setw( 2 ) << frame;
				ss << ".png";
				window->TakeScreenshotToFile( ss.str() );
				*/
			}
		}
	};
};



template<typename T>
std::ostream& operator<<( std::ostream & os, const std::vector<T> & v )
{
	auto vs = std::size( v );
	if( vs ) {
		os << "[";
		for( size_t i = 0; i < vs - 1; ++i ) {
			os << v[ i ] << ", ";
		}
		os << v.back() << "]";
	} else {
		os << "[]";
	}
	return os;
}



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
	window_create_info.size					= { 1200, 600 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_1;
	window_create_info.event_handler		= &event_handler;
	window_create_info.vsync				= false;
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	std::string s;
	s = "Testing";

	vk2d::Text t = "Testing";
	t = "Test2";
	t = "Testing this thing even more...";

	vk2d::Array<int32_t> arr1;
	vk2d::Array<int32_t> arr2 { 5, 8 };
	arr1 = arr2;
	vk2d::Array<uint32_t> arr = { 0, 1, 2, 3, 4 };
	std::cout << "Single array element: " << arr[ 0 ] << "\n";
	std::cout << "Entire array: " << arr << "\n";
	arr[ 4 ] = 1000;
	std::cout << "Modified array: " << arr << "\n";
	std::cout << "Array size: " << std::size( arr ) << "\n";

	arr.PushBack( 50 );
	std::cout << "Array appended: " << arr << "\n";

	std::vector<uint32_t> vec = arr;

	auto resource_manager		= instance->GetResourceManager();

	auto font = resource_manager->LoadFontResource(
		"../../Data/Fonts/ubuntu-font-family-0.83/Ubuntu-M.ttf"
	);

	std::vector<vk2d::Vector4f> stl_vector { {}, { 10.0f, 10.4f, 10.04f }, { 50.1f, float( vk2d::PI ), 40.03f } };
	std::cout << stl_vector << "\n";

	auto delta_time_counter		= DeltaTimeCounter();
	auto delta_time				= 0.0f;
	auto seconds_since_start	= 0.0f;
	auto fps_time_counter		= 0.0f;
	auto fps_counter			= 0;

	while( instance->Run() && !window1->ShouldClose() ) {
		delta_time				= delta_time_counter.Tick();
		seconds_since_start		+= delta_time;

		fps_time_counter		+= delta_time;
		++fps_counter;
		if( fps_time_counter >= 1.0f ) {
			window1->SetTitle( std::to_string( fps_counter ) );
			fps_time_counter -= 1.0f;
			fps_counter = 0;
		}



		std::string text = "Testing...";
		vk2d::Vector2f text_location { std::cos( seconds_since_start / 3.0f ) * 300.0f, std::sin( seconds_since_start / 3.0f ) * 300.0f };
		vk2d::Vector2f text_scale { std::cos( seconds_since_start * 5.0f ) * 0.5f + 1.5f, std::cos( seconds_since_start * 5.0f ) * 0.5f + 1.5f };

		auto text_calculated_area = font->CalculateRenderedSize(
			text,
			0,
			text_scale,
			false,
			0,
			true
		);
		auto text_mesh = vk2d::GenerateTextMesh(
			font,
			text_location,
			text,
			0,
			text_scale,
			false,
			0,
			true
		);



		if( !window1->BeginRender() ) return -1;

		{
			window1->DrawEllipse(
				{ -300, -300, 300, 300 },
				false,
				64,
				vk2d::Colorf::CYAN()
			);
			window1->DrawMesh( text_mesh );
			window1->DrawRectangle( text_mesh.aabb, false );
			//window1->DrawRectangle( text_calculated_area + text_location, false );
		}

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
