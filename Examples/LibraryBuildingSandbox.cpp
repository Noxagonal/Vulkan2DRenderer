
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
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	auto resource_manager		= instance->GetResourceManager();

	auto blue_circle			= vk2d::GenerateEllipseMesh(
		{ -4, -4, 4, 4 }
	);
	auto blue_line				= vk2d::GenerateLineMeshFromList(
		{ { 0, -5000 }, { 0, 5000 } },
		{ { 0, 1 } }
	);
	blue_circle.SetVertexColor( { 0.0f, 0.3f, 1.0f, 1.0f } );
	blue_line.SetVertexColor( { 0.0f, 0.3f, 1.0f, 0.05f } );


	auto red_circle			= vk2d::GenerateEllipseMesh(
		{ -7, -7, 7, 7 }
	);
	auto red_line				= vk2d::GenerateLineMeshFromList(
		{ { 0, -5000 }, { 0, 5000 } },
		{ { 0, 1 } }
	);
	red_circle.SetVertexColor( { 1.0f, 0.4f, 0.3f, 1.0f } );
	red_line.SetVertexColor( { 1.0f, 0.4f, 0.3f, 0.05f } );


	auto delta_time_counter		= DeltaTimeCounter();
	auto delta_time				= 0.0f;
	auto seconds_since_start	= 0.0f;

	while( instance->Run() && !window1->ShouldClose() ) {
		delta_time				= delta_time_counter.Tick();
		seconds_since_start		+= delta_time;

		if( !window1->BeginRender() ) return -1;



		{
			float pc1 = ( std::cos( seconds_since_start * 0.414f ) * 0.5f + 0.5f ) * 30.0f + 3.0f;
			float pc2 = ( std::sin( seconds_since_start * 0.231f ) * 0.5f + 0.5f ) * 30.0f + 3.0f;

			float pc1_scale = window1->GetSize().x / ( pc1 - 1.0f );
			float pc2_scale = window1->GetSize().x / ( pc2 - 1.0f );

			std::vector<float> pc1_values( std::ceil( pc1 ) + 1 );
			std::vector<float> pc2_values( std::ceil( pc2 ) );

			for( size_t i = 0; i < std::size( pc1_values ); ++i ) {
				pc1_values[ i ] = std::sin( seconds_since_start + i * pc1_scale * 0.01f );
			}

			float s = ( pc1 - 1.0f ) / ( pc2 - 1.0f );
			for( int32_t i = 0; i < std::size( pc2_values ); ++i ) {
				float	pc1_total_offset	= i * s;
				size_t	pc1_index			= size_t( std::floor( pc1_total_offset ) );
				float	pc1_offset			= pc1_total_offset - float( pc1_index );

				auto pc1_value1				= ( pc1_index < std::size( pc1_values ) ) ? pc1_values[ pc1_index ] : pc1_values.back();
				auto pc1_value2				= ( pc1_index + 1 < std::size( pc1_values ) ) ? pc1_values[ pc1_index + 1 ] : pc1_values.back();

				pc2_values[ i ] = pc1_value1 * ( 1.0f - pc1_offset ) + pc1_value2 * pc1_offset;
			}
			//pc2_values.back() = pc1_values.back();

			std::vector<vk2d::Matrix4f> pc1_point_transformations( size_t( std::ceil( pc1 ) ) );
			std::vector<vk2d::Matrix4f> pc2_point_transformations( std::size( pc2_values ) );

			for( int32_t i = 0; i < std::size( pc1_point_transformations ); ++i ) {
				vk2d::Transform t;
				t.Translate( { i * pc1_scale - window1->GetSize().x / 2, pc1_values[ i ] * 300.0f } );
				pc1_point_transformations[ i ] = t.CalculateTransformationMatrix();
			}
			for( int32_t i = 0; i < std::size( pc2_point_transformations ); ++i ) {
				vk2d::Transform t;
				t.Translate( { i * pc2_scale - window1->GetSize().x / 2, pc2_values[ i ] * 300.0f } );
				pc2_point_transformations[ i ] = t.CalculateTransformationMatrix();
			}

			window1->DrawMesh(
				blue_line,
				pc1_point_transformations
			);
			window1->DrawMesh(
				blue_circle,
				pc1_point_transformations
			);

			window1->DrawMesh(
				red_line,
				pc2_point_transformations
			);
			window1->DrawMesh(
				red_circle,
				pc2_point_transformations
			);
		}

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
