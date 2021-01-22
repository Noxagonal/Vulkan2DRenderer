
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
			if( button == vk2d::KeyboardButton::KEY_1 ) {
				window->SetRenderCoordinateSpace( vk2d::RenderCoordinateSpace::TEXEL_SPACE );
			}
			if( button == vk2d::KeyboardButton::KEY_2 ) {
				window->SetRenderCoordinateSpace( vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED );
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
	window_create_info.size					= { 600, 600 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_1;
	window_create_info.event_handler		= &event_handler;
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	uint32_t rtt_pixels_x = 16;
	uint32_t rtt_pixels_y = 16;
	vk2d::RenderTargetTextureCreateInfo rtt_create_info {};
	rtt_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	rtt_create_info.size					= { rtt_pixels_x, rtt_pixels_y };
	auto rtt = instance->CreateRenderTargetTexture( rtt_create_info );

	vk2d::SamplerCreateInfo sampler_create_info {};
	sampler_create_info.minification_filter		= vk2d::SamplerFilter::NEAREST;
	sampler_create_info.magnification_filter	= vk2d::SamplerFilter::NEAREST;
	auto pixelated_sampler = instance->CreateSampler( sampler_create_info );



	vk2d::tr1::Mesh<0, 0> tmesh1( 0, 0 );
	vk2d::tr1::Mesh<1, 1, glm::vec2> tmesh2( 0, 0 );
	vk2d::tr1::Mesh<0, 2> tmesh3( 0, 0 );

	tmesh1.AppendVertex( glm::vec2( 1, 2 ) );
	tmesh2.AppendVertex( glm::vec2( 1, 2 ), { glm::vec2( 3, 4 ) }, { vk2d::Colorf::GREY() }, glm::vec2( 5, 6 ) );
	tmesh3.AppendVertex( glm::vec2( 1, 2 ), { vk2d::Colorf::GREEN(), vk2d::Colorf::BLUE() } );
	auto v1 = tmesh1[ 0 ];
	auto v2 = tmesh2[ 0 ];
	auto v3 = tmesh3[ 0 ];


	auto resource_manager		= instance->GetResourceManager();

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
		glm::vec2 text_location { std::cos( seconds_since_start / 3.0f ) * 300.0f, std::sin( seconds_since_start / 3.0f ) * 300.0f };
		glm::vec2 text_scale { std::cos( seconds_since_start * 5.0f ) * 0.5f + 1.5f, std::cos( seconds_since_start * 5.0f ) * 0.5f + 1.5f };

		rtt->BeginRender();
		rtt->DrawEllipse( { -4, -4, 4, 4 }, true, 8 );
		rtt->EndRender();

		if( !window1->BeginRender() ) return -1;

		{
			auto rtt_mesh = vk2d::GenerateLatticeMesh( { -200, -200, 200, 200 }, { float( rtt_pixels_x - 1 ), float( rtt_pixels_y - 1 ) } );
			rtt_mesh.SetTexture( rtt );
			rtt_mesh.SetSampler( pixelated_sampler );
			window1->DrawMesh( rtt_mesh );

			rtt_mesh.SetTexture( nullptr );
			rtt_mesh.SetSampler( nullptr );
			rtt_mesh.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
			window1->DrawMesh( rtt_mesh );

			//window1->DrawRectangle( text_calculated_area + text_location, false );
		}

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
