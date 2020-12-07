
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>
#include <sstream>



constexpr double PI		= 3.14159265358979323846;
constexpr double RAD	= PI * 2.0;

uint32_t frame = 20;


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
				std::stringstream ss;
				ss << "../../Docs/Images/SamplerLODBias_";
				ss << std::setfill( '0' ) << std::setw( 2 ) << frame;
				ss << ".png";
				window->TakeScreenshotToFile( ss.str() );

				// TODO: Taking screenshots crashes. Figure out why before merging into master.
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
	window_create_info.size					= { 256, 256 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::NORMALIZED_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_1;
	window_create_info.event_handler		= &event_handler;
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	auto resource_manager		= instance->GetResourceManager();
	auto texture_resource		= resource_manager->LoadTextureResource( "../../Docs/Images/sampler_filter_example_source_2.png" );

	vk2d::SamplerCreateInfo		sampler_create_info {};
	sampler_create_info.magnification_filter		= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.minification_filter			= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.mipmap_mode					= vk2d::SamplerMipmapMode::LINEAR;
	sampler_create_info.anisotropy_enable			= true;
	sampler_create_info.mipmap_max_anisotropy		= 16.0f;
	sampler_create_info.mipmap_level_of_detail_bias	= frame / 3.0f - 3.0f;
	sampler_create_info.mipmap_min_level_of_detail	= 0.0f;
	sampler_create_info.mipmap_max_level_of_detail	= 128.0f;
	auto sampler_linear			= instance->CreateSampler( sampler_create_info );

	sampler_create_info.magnification_filter		= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.minification_filter			= vk2d::SamplerFilter::LINEAR;
	sampler_create_info.mipmap_level_of_detail_bias	= 0.0f;
	auto sampler_nearest		= instance->CreateSampler( sampler_create_info );

	auto box					= vk2d::GenerateLatticeMesh(
		{ -1.0f, -1.0f, 1.0f, 1.0f },
		{ 50.0f, 50.0f },
		true
	);
	/*
	for( size_t y = 0; y < 2; ++y ) {
		for( size_t x = 0; x < 27; ++x ) {
			auto & vx = box.vertices[ y * 27 + x ].vertex_coords.x;
			vx += 1.0f;
			vx *= x * 0.168f;
			vx -= 1.0f;

			vx = -std::cos( ( x / 27.0f ) * vk2d::PI / 2.0 ) * 2.0;
		}
	}
	*/
	/*
	box.Wave(
		1.12f,
		2.0f,
		0.0f,
		{ 0.12f, 0.12f }
	);
	*/
	box.SetTexture( texture_resource );
	box.SetLineWidth( 10.0f );
//	box.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );

	auto delta_time_counter		= DeltaTimeCounter();
	auto delta_time				= 0.0f;
	auto seconds_since_start	= 0.0f;

	while( instance->Run() && !window1->ShouldClose() ) {
		delta_time				= delta_time_counter.Tick();
		seconds_since_start		+= delta_time;

		if( !window1->BeginRender() ) return -1;

		vk2d::Transform t1, t2;
		t1.Rotate( 0.2f );
		t1.Translate( { 0.0f, 0.0f } );
//		t1.Translate( { -1.0f, 0.0f } );
//		t2.Translate( { +1.0f, 0.0f } );

		box.SetSampler( sampler_linear );
		window1->DrawMesh( box, t1 );

//		box.SetSampler( sampler_nearest );
//		window1->DrawMesh( box, t2 );

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
