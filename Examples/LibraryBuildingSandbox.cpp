
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>



constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



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



void DrawRenderTargetTextureContent(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture1,
	vk2d::RenderTargetTexture		*	render_target_texture2,
	vk2d::Texture					*	texture_resource );



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
	auto window1 = instance->CreateOutputWindow( window_create_info );
	if( !window1 ) return -1;

	auto window2 = instance->CreateOutputWindow( window_create_info );
	if( !window2 ) return -1;

	vk2d::RenderTargetTextureCreateInfo render_target_texture_create_info {};
	render_target_texture_create_info.coordinate_space	= vk2d::RenderCoordinateSpace::TEXEL_SPACE;
	render_target_texture_create_info.size				= vk2d::Vector2u( 512, 512 );
	auto render_target_texture1 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);
	auto render_target_texture2 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);

	auto resource_manager = instance->GetResourceManager();
	auto texture_resource = resource_manager->LoadTextureResource( "../../Data/GrafGear_128.png" );

	vk2d::SamplerCreateInfo sampler_create_info {};
	sampler_create_info.minification_filter				= vk2d::SamplerFilter::NEAREST;
	sampler_create_info.magnification_filter			= vk2d::SamplerFilter::NEAREST;
	sampler_create_info.mipmap_mode						= vk2d::SamplerMipmapMode::NEAREST;
	sampler_create_info.address_mode_u					= vk2d::SamplerAddressMode::CLAMP_TO_EDGE;
	sampler_create_info.address_mode_v					= vk2d::SamplerAddressMode::CLAMP_TO_EDGE;
	sampler_create_info.border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
	sampler_create_info.mipmap_enable					= false;
	sampler_create_info.mipmap_max_anisotropy			= 16.0f;
	sampler_create_info.mipmap_level_of_detail_bias		= 0.0f;
	sampler_create_info.mipmap_min_level_of_detail		= 0.0f;
	sampler_create_info.mipmap_max_level_of_detail		= 128.0f;
	auto pixel_sampler		= instance->CreateSampler( sampler_create_info );

	auto delta_time_time_point = std::chrono::high_resolution_clock::now();
	float animation_counter = 0.0f;
	auto delta_time = 0.0f;
	size_t frame_counter = 0;

	DrawRenderTargetTextureContent(
		delta_time,
		animation_counter,
		render_target_texture1,
		render_target_texture2,
		texture_resource
	);

	while( !window1->ShouldClose() && !window2->ShouldClose() ) {
		{
			auto now = std::chrono::high_resolution_clock::now();
			delta_time = std::chrono::duration<float>( now - delta_time_time_point ).count();
			delta_time_time_point = now;
		}
		animation_counter += delta_time;
		++frame_counter;

		if( frame_counter % 10 == 0 ) {
			DrawRenderTargetTextureContent(
				delta_time,
				animation_counter,
				render_target_texture1,
				render_target_texture2,
				texture_resource
			);
		}

		{
			if( !window1->BeginRender() ) return -1;

			auto draw_rect_size = vk2d::Vector2f( 512, 512 );

			auto textured_box = vk2d::GenerateBoxMesh(
				draw_rect_size * vk2d::Vector2f( -1, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 0, 0.5 )
			);
			textured_box.SetTexture( render_target_texture2 );
			textured_box.SetSampler( pixel_sampler );
			window1->DrawMesh( textured_box );

			auto grid = vk2d::GenerateLatticeMesh(
				draw_rect_size * vk2d::Vector2f( 0, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 1, 0.5 ),
				vk2d::Vector2f( 30, 30 ),
				true
			);
			grid.SetTexture( render_target_texture2 );
			grid.SetSampler( pixel_sampler );
			grid.Wave(
				2.36f,
				1.4f,
				animation_counter,
				vk2d::Vector2f( 30, 30 )
			);
			window1->DrawMesh( grid );

			grid.SetTexture( nullptr );
			grid.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
			grid.SetVertexColor( vk2d::Colorf( 0.1f, 1.0f, 0.3f, 0.2f ) );
			window1->DrawMesh( grid );

			if( !window1->EndRender() ) return -1;
		}

		{
			if( !window2->BeginRender() ) return -1;

			auto draw_rect_size = vk2d::Vector2f( 512, 512 );

			auto textured_box = vk2d::GenerateBoxMesh(
				draw_rect_size * vk2d::Vector2f( -0.5, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 0.5, 0.5 )
			);
			textured_box.SetTexture( render_target_texture1 );
			textured_box.SetSampler( pixel_sampler );
			window2->DrawMesh( textured_box );

			if( !window2->EndRender() ) return -1;
		}
	}
	return 0;
}



void DrawRenderTargetTextureContent(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture1,
	vk2d::RenderTargetTexture		*	render_target_texture2,
	vk2d::Texture					*	texture_resource
)
{
	{
		auto render_target1_size_f = vk2d::Vector2f( render_target_texture1->GetSize().x, render_target_texture1->GetSize().y );

		render_target_texture1->BeginRender();

		auto textured_box = vk2d::GenerateBoxMesh(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f
		);
		textured_box.SetTexture( texture_resource );
		textured_box.SetVertexColor( vk2d::Colorf( 1, 1, 1, 1 ) );
		render_target_texture1->DrawMesh( textured_box );


		render_target_texture1->DrawPieBox(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f,
			std::sin( basic_animation_counter * 0.125f ) * PI * 2.0f,
			std::sin( basic_animation_counter * 0.237f ) * 0.5f + 0.5f,
			true,
			vk2d::Colorf( 1.0f, 1.0f, 1.0f, 0.6f )
		);

		render_target_texture1->DrawPoint(
			vk2d::Vector2f( 0, 0 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture1->DrawPoint(
			vk2d::Vector2f( 512, 0 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture1->DrawPoint(
			vk2d::Vector2f( 0, 512 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture1->DrawPoint(
			vk2d::Vector2f( 512, 512 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture1->EndRender();
	}



	{
		auto render_target2_size_f = vk2d::Vector2f( render_target_texture2->GetSize().x, render_target_texture2->GetSize().y );

		render_target_texture2->BeginRender();

		auto lattice_mesh = vk2d::GenerateLatticeMesh(
			vk2d::Vector2f( 0, 0 ),
			render_target2_size_f,
			vk2d::Vector2f( 20, 20 )
		);
		lattice_mesh.SetTexture( render_target_texture1 );
		lattice_mesh.Wave(
			5.36f,
			1.6f,
			basic_animation_counter,
			vk2d::Vector2f( 20, 20 )
		);

		render_target_texture2->DrawMesh( lattice_mesh );

		render_target_texture2->EndRender();
	}
}
