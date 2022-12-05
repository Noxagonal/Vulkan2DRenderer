
#include <VK2D.hpp>

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



const auto test_vertex_shader_glsl = std::string_view {
R"glsl(

void main()
{
	vec4 raw_vertex_coords			= vec4( vertex_buffer.data[ gl_VertexIndex ].coords, 0.0, 1.0 );
	vertex_output_UV				= vertex_buffer.data[ gl_VertexIndex ].UVs;
	vertex_output_color				= vertex_buffer.data[ gl_VertexIndex ].color;
	vertex_output_original_coords	= raw_vertex_coords.xy;
	vertex_output_vertex_index		= gl_VertexIndex;
	vertex_output_texture_channel	= vertex_buffer.data[ gl_VertexIndex ].single_texture_channel;

	mat4 transformation_matrix		= transformation_buffer.data[ gl_InstanceIndex + push_constants.transformation_offset ];

	vec2 transformed_vertex_coords	= ( transformation_matrix * raw_vertex_coords ).xy;
	vec2 viewport_vertex_coords		= transformed_vertex_coords * window_frame_data.multiplier + window_frame_data.offset;

	gl_Position						= vec4( viewport_vertex_coords, 0.5, 1.0 );
	gl_PointSize					= vertex_buffer.data[ gl_VertexIndex ].point_size;
}

)glsl"
};



int main()
{
	auto instance			= vk2d::CreateInstance();
	auto resource_manager	= instance->GetResourceManager();

	/// !!! TESTING !!!
	auto material = vk2d::MaterialResourceHandle<vk2d::StandardVertex>();
	{
		auto material_create_info = vk2d::MaterialCreateInfo();
		material_create_info.shader_create_infos.push_back(
			vk2d::ShaderCreateInfo(
				vk2d::ShaderStage::VERTEX,
				"Test Vertex shader",
				test_vertex_shader_glsl
			)
		);

		material = resource_manager->CreateMaterialResource<vk2d::StandardVertex>(
			{ "test1", "test2", "test3", "test4", "test5" },
			material_create_info
		);
	}

	EventHandler event_handler;
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 600, 600 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.samples				= vk2d::Multisamples::SAMPLE_COUNT_1;
	window_create_info.event_handler		= &event_handler;
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	vk2d::SamplerCreateInfo sampler_create_info {};
	sampler_create_info.minification_filter		= vk2d::SamplerFilter::NEAREST;
	sampler_create_info.magnification_filter	= vk2d::SamplerFilter::NEAREST;
	auto pixelated_sampler = instance->CreateSampler( sampler_create_info );

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

		if( !window1->BeginRender() ) return -1;

		{
			auto lattice_mesh = vk2d::mesh_generators::GenerateLatticeMesh( { -200, -200, 200, 200 }, { 16, 16 }, false );

			//lattice_mesh.SetSampler( pixelated_sampler );
			window1->DrawMesh( lattice_mesh );
			window1->DrawEllipse( { -50, -50, 50, 50 } );

			//lattice_mesh.SetTexture( nullptr );
			//lattice_mesh.SetSampler( nullptr );
			//lattice_mesh.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
			//window1->DrawMesh( lattice_mesh );

			//window1->DrawRectangle( text_calculated_area + text_location, false );
		}

		if( !window1->EndRender() ) return -1;
	}
	return 0;
}
