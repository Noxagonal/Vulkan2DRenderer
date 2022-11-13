
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <format>



uint32_t frame = 0;



class EventHandler : public vk2d::WindowEventHandler {
public:
	// Keyboard button was pressed, released or kept down ( repeating ).
	void										EventKeyboard(
		vk2d::Window						&	window,
		vk2d::KeyboardButton					button,
		int32_t									scancode,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifierKeys
	)
	{
		if( action == vk2d::ButtonAction::PRESS ) {
			if( button == vk2d::KeyboardButton::KEY_ESCAPE ) {
				window.CloseWindow();
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
				window.SetRenderCoordinateSpace( vk2d::RenderCoordinateSpace::TEXEL_SPACE );
			}
			if( button == vk2d::KeyboardButton::KEY_2 ) {
				window.SetRenderCoordinateSpace( vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED );
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



struct VertexPropertiesOffsets
{
	size_t			vertex_coords			= 0;
	size_t			uv_coords				= 0;
	size_t			color					= 0;
	size_t			point_size				= 0;
	size_t			single_texture_layer	= 0;
};

template<vk2d::vk2d_internal::VertexBaseDerivedType T>
constexpr VertexPropertiesOffsets GetVertexPropertiesOffsets()
{
	static_assert( vk2d::vk2d_internal::SupportedMeshVertexCoordinateTypes<decltype( T::vertex_coords )> );
	static_assert( vk2d::vk2d_internal::SupportedMeshUVCoordinateTypes<decltype( T::uv_coords )> );

	auto ret					= VertexPropertiesOffsets();
	auto temp_vertex			= T();

	ret.vertex_coords			= reinterpret_cast<const uint8_t*>( &temp_vertex.vertex_coords )		- reinterpret_cast<const uint8_t*>( &temp_vertex );
	ret.uv_coords				= reinterpret_cast<const uint8_t*>( &temp_vertex.uv_coords )			- reinterpret_cast<const uint8_t*>( &temp_vertex );
	ret.color					= reinterpret_cast<const uint8_t*>( &temp_vertex.color )				- reinterpret_cast<const uint8_t*>( &temp_vertex );
	ret.point_size				= reinterpret_cast<const uint8_t*>( &temp_vertex.point_size )			- reinterpret_cast<const uint8_t*>( &temp_vertex );
	ret.single_texture_layer	= reinterpret_cast<const uint8_t*>( &temp_vertex.single_texture_layer )	- reinterpret_cast<const uint8_t*>( &temp_vertex );

	return ret;
};



int main()
{
	// TESTING!!!
	{
		vk2d::StandardVertex std_vertex;
		//std_vertex.position = { 50, 50 };
		auto alignment_test = alignof( vk2d::StandardVertex );
		auto size_test1 = sizeof( vk2d::StandardVertex );
		auto size_test2 = sizeof( vk2d::StandardVertex::Base );

		std::cout << std::format(
			"Standard vertex alignment: {}\n"
			"Standard vertex size:      {}\n"
			"Standard vertex base size: {}\n\n",
			alignment_test,
			size_test1,
			size_test2
		);

		std_vertex.vertex_coords = { 50, 50 };

		auto offsets			= GetVertexPropertiesOffsets<vk2d::StandardVertex>();
		auto previous_offset	= size_t( 0 );

		std::cout << std::format( "vertex_coords offset from start:        {} - Offset from previous: {}\n", offsets.vertex_coords, offsets.vertex_coords - previous_offset );					previous_offset = offsets.vertex_coords;
		std::cout << std::format( "uv_coords offset from start:            {} - Offset from previous: {}\n", offsets.uv_coords, offsets.uv_coords - previous_offset );							previous_offset = offsets.uv_coords;
		std::cout << std::format( "color offset from start:                {} - Offset from previous: {}\n", offsets.color, offsets.color - previous_offset );									previous_offset = offsets.color;
		std::cout << std::format( "point_size offset from start:           {} - Offset from previous: {}\n", offsets.point_size, offsets.point_size - previous_offset );						previous_offset = offsets.point_size;
		std::cout << std::format( "single_texture_layer offset from start: {} - Offset from previous: {}\n", offsets.single_texture_layer, offsets.single_texture_layer - previous_offset );	previous_offset = offsets.single_texture_layer;

		std::cout << "asdf\n";

		// TODO: Need reflection for proper struct implementation with member names. May be doable with a macro.
		// On the other hand, it may be an overkill.
		// 
		// Proposal 1: Introduce getter functions to get a specific member, eg. std_vertex.position() = { 50, 50 };
		// Ugly, not very proper C++.
		// 
		// Proposal 2: We could just ignore extra members... Whatever function accepts a vertex list, could accept any type, it
		// could then take only the VertexBase class from every element and ignore the rest, this will require base class to be
		// typedeffed and it will be somewhat slower as we need to only partially copy memory. In terms of simplicity, I think
		// this should be the way to go.
		// CANDIDATE!
		// 
		// Proposal 3: Whatever function taking a vertex or vertex list could only take a predetermined type, for build-in types
		// we could use whatever members are provided, for custom types, use only Get<N>(). Not a fan of this because it makes
		// things more complicated to design and it somewhat promotes the build-in types. Custom designs should feel like
		// first-party members.
		//
		// Proposal 4: Use union... Needs more thinking, may not work. This is kinda unsafe if it even works.
		// WILL NOT WORK! Does not allow members from parent.
		//
		// Proposal 5: Use macros... Needs more thinking, may not work. A little fiddly and a little uggly but could potentially
		// solve all problems.
		// WILL NOT WORK! Problem extracting specific parameters.
		//
		// Proposal 6: See if lambdas could be used here somehow.
		// WILL NOT WORK! Can only be invoked with parenthesis.
		//
	}



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
