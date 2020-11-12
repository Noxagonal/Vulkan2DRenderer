
#include <VK2D.h>

#include <chrono>
#include <string>
#include <iostream>
#include <sstream>



constexpr double PI		= 3.14159265358979323846;
constexpr double RAD	= PI * 2.0;

float blur_test_value_direction		= 0.0f;
float blur_test_value				= 50.0f;


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



void DrawRenderTargetTextureContent1(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture,
	vk2d::Texture					*	texture_resource );

void DrawRenderTargetTextureContent2(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture,
	vk2d::Texture					*	texture_resource );

void DrawTextRenderTargetTextureContent(
	vk2d::RenderTargetTexture		*	text_layer_1,
	vk2d::RenderTargetTexture		*	text_layer_2,
	vk2d::FontResource				*	font_resource );


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

	//auto window2 = instance->CreateOutputWindow( window_create_info );
	//if( !window2 ) return -1;

	vk2d::RenderTargetTextureCreateInfo render_target_texture_create_info {};
	render_target_texture_create_info.coordinate_space	= vk2d::RenderCoordinateSpace::TEXEL_SPACE;
	render_target_texture_create_info.size				= vk2d::Vector2u( 512, 512);
	render_target_texture_create_info.samples			= vk2d::Multisamples::SAMPLE_COUNT_1;
	render_target_texture_create_info.enable_blur		= false;
	auto render_target_texture1 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);
	render_target_texture_create_info.enable_blur		= true;
	auto render_target_texture2 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);

	render_target_texture_create_info.size				= vk2d::Vector2u( 800, 200 );
	render_target_texture_create_info.samples			= vk2d::Multisamples::SAMPLE_COUNT_1;
	render_target_texture_create_info.enable_blur		= true;
	auto text_layer_1 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);
	render_target_texture_create_info.enable_blur		= false;
	auto text_layer_2 = instance->CreateRenderTargetTexture(
		render_target_texture_create_info
	);

	auto resource_manager = instance->GetResourceManager();
	auto texture_resource = resource_manager->LoadTextureResource( "../../Data/GrafGear_128.png" );
	auto font_resource		= resource_manager->LoadFontResource( "../../Data/Fonts/DroidSandMono/DroidSansMono.ttf", 28 );

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

	//DrawRenderTargetTextureContent(
	//	delta_time,
	//	animation_counter,
	//	render_target_texture1,
	//	render_target_texture2,
	//	texture_resource
	//);

	auto fps_counter			= 0;
	auto fps_counter_timer		= 0.0f;

	while( !window1->ShouldClose() ) {
		{
			auto now = std::chrono::high_resolution_clock::now();
			delta_time = std::chrono::duration<float>( now - delta_time_time_point ).count();
			delta_time_time_point = now;

			fps_counter_timer += delta_time;
			if( fps_counter_timer > 1.0f ) {
				fps_counter_timer -= floor( fps_counter_timer );
				std::stringstream ss;
				ss << "FPS: " << fps_counter;
				window1->SetTitle( ss.str() );
				fps_counter = 0;
			}
			++fps_counter;
		}
		blur_test_value += blur_test_value_direction * delta_time * 150.0f;
		if( blur_test_value < 0.0f ) blur_test_value = 0.0f;
		if( blur_test_value > 500.0f ) blur_test_value = 500.0f;

		animation_counter += delta_time;
		++frame_counter;

		DrawTextRenderTargetTextureContent(
			text_layer_1,
			text_layer_2,
			font_resource
		);

		if( frame_counter % 1 == 0 ) {
			DrawRenderTargetTextureContent1(
				delta_time,
				animation_counter,
				render_target_texture1,
				texture_resource
			);
			DrawRenderTargetTextureContent2(
				delta_time,
				animation_counter,
				render_target_texture2,
				render_target_texture1
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
			grid.SetVertexColor( vk2d::Colorf( 0.1f, 1.0f, 0.3f, 1.0f ) );
			window1->DrawMesh( grid );

			window1->DrawTexture(
				{ -400.0f, 200.0f },
				text_layer_1,
				{ 1.0f, 1.0f, 1.0f, 5.0f }
			);
			window1->DrawTexture(
				{ -400.0f, 200.0f },
				text_layer_2
			);

			if( !window1->EndRender() ) return -1;
		}
		/*
		{
			if( !window2->BeginRender() ) return -1;

			auto draw_rect_size = vk2d::Vector2f( 512, 512 );

			auto textured_box = vk2d::GenerateBoxMesh(
				draw_rect_size * vk2d::Vector2f( -0.5, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 0.5, 0.5 )
			);
			textured_box.SetTexture( render_target_texture2 );
			textured_box.SetSampler( pixel_sampler );
			window2->DrawMesh( textured_box );

			if( !window2->EndRender() ) return -1;
		}
		*/
	}
	return 0;
}



void DrawRenderTargetTextureContent1(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture,
	vk2d::Texture					*	texture_resource
)
{
	{
		auto render_target1_size_f = vk2d::Vector2f( render_target_texture->GetSize().x, render_target_texture->GetSize().y );

		render_target_texture->BeginRender();

		auto textured_box = vk2d::GenerateBoxMesh(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f
		);
		textured_box.SetTexture( texture_resource );
		textured_box.SetVertexColor( vk2d::Colorf( 1, 1, 1, 1 ) );
		render_target_texture->DrawMesh( textured_box );


		render_target_texture->DrawPieBox(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f,
			std::sin( basic_animation_counter * 0.125f ) * PI * 2.0f,
			std::sin( basic_animation_counter * 0.237f ) * 0.5f + 0.5f,
			true,
			vk2d::Colorf( 1.0f, 1.0f, 1.0f, 0.5f )
		);
		/*
		render_target_texture->DrawPieBox(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f,
			std::sin( basic_animation_counter * 0.0185f ) * PI * 2.0f,
			std::sin( basic_animation_counter * 0.0267f ) * 0.5f + 0.5f,
			true,
			vk2d::Colorf( 1.0f, 1.0f, 1.0f, 0.5f )
		);

		render_target_texture->DrawPieBox(
			vk2d::Vector2f( 0, 0 ),
			render_target1_size_f,
			std::sin( basic_animation_counter * 0.169f ) * PI * 2.0f,
			std::sin( basic_animation_counter * 0.278f ) * 0.5f + 0.5f,
			true,
			vk2d::Colorf( 1.0f, 1.0f, 1.0f, 0.5f )
		);
		*/
		render_target_texture->DrawPoint(
			vk2d::Vector2f( 0, 0 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture->DrawPoint(
			vk2d::Vector2f( 512, 0 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture->DrawPoint(
			vk2d::Vector2f( 0, 512 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture->DrawPoint(
			vk2d::Vector2f( 512, 512 ),
			vk2d::Colorf( 1, 1, 1, 1 ),
			10.0f
		);

		render_target_texture->EndRender();
	}
}

void DrawRenderTargetTextureContent2(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture,
	vk2d::Texture					*	texture_resource
)
{
	{
		auto render_target2_size_f = vk2d::Vector2f( render_target_texture->GetSize().x, render_target_texture->GetSize().y );

		render_target_texture->BeginRender();

		auto lattice_mesh = vk2d::GenerateLatticeMesh(
			vk2d::Vector2f( 0, 0 ),
			render_target2_size_f,
			vk2d::Vector2f( 20, 20 )
		);
		lattice_mesh.SetTexture( texture_resource );
		lattice_mesh.Wave(
			5.36f,
			1.6f,
			basic_animation_counter,
			vk2d::Vector2f( 20, 20 )
		);

		render_target_texture->DrawMesh( lattice_mesh );

		render_target_texture->EndRender( { blur_test_value, blur_test_value } );
	}
}



void DrawTextRenderTargetTextureContent(
	vk2d::RenderTargetTexture	*	text_layer_1,
	vk2d::RenderTargetTexture	*	text_layer_2,
	vk2d::FontResource			*	font_resource
)
{
	std::stringstream ss;
	ss << "Current amount = " << blur_test_value;

	auto line1 = vk2d::GenerateTextMesh(
		font_resource,
		{ 40.0f, 48.0f },
		"Press up or down to change blur amount."
	);
	auto line2 = vk2d::GenerateTextMesh(
		font_resource,
		{ 40.0f, 48.0f + 28.0f },
		ss.str()
	);

	text_layer_2->BeginRender();
	text_layer_2->DrawMesh( line1 );
	text_layer_2->DrawMesh( line2 );
	text_layer_2->EndRender();

	line1.SetVertexColor( { 0.0f, 0.0f, 0.0f, 1.0f } );
	line2.SetVertexColor( { 0.0f, 0.0f, 0.0f, 1.0f } );
	text_layer_1->BeginRender();
	text_layer_1->DrawMesh( line1 );
	text_layer_1->DrawMesh( line2 );
	text_layer_1->EndRender( { 30.0f, 30.0f } );
}
