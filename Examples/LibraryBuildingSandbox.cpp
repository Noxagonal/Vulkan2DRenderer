
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
	vk2d::RenderTargetTexture		*	render_target_texture,
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
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	vk2d::RenderTargetTextureCreateInfo render_target_texture_create_info {};
	render_target_texture_create_info.coordinate_space	= vk2d::RenderCoordinateSpace::TEXEL_SPACE;
	render_target_texture_create_info.size				= vk2d::Vector2u( 64, 64 );
	auto render_target_texture = instance->CreateRenderTargetTexture(
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

	TODO this shit below;
	// This shouldn't work at all at this point, render target is rendered once and then reused, double check the path the
	// render target takes. Timeline semaphore value is not updated between reuses and not other path is given at this point.
	// Use render target's "has_been_submitted" variable to detect if render target is pending render. if it's false,
	// then we don't have to wait for the semaphore, nor update it's value, just use without any dependency barriers like
	// regular textures.
	DrawRenderTargetTextureContent(
		delta_time,
		animation_counter,
		render_target_texture,
		texture_resource
	);

	while( !window->ShouldClose() ) {
		{
			auto now = std::chrono::high_resolution_clock::now();
			delta_time = std::chrono::duration<float>( now - delta_time_time_point ).count();
			delta_time_time_point = now;
		}
		animation_counter += delta_time;

		// Using render target textures introduces a new dependency tree
		// Example 1:
		//
		// render_target_texture_1->BeginRender();
		// normal renders and textures
		// render_target_texture_1->EndRender();
		//
		// render_target_texture_2->BeginRender();
		// render with render_target_texture_1
		// render_target_texture_2->EndRender();
		//
		// render_target_texture_1 must complete render before render_target_texture_2.
		//
		//
		// Example 2:
		//
		// render_target_texture_1->BeginRender();
		// use render_target_texture_2 from previous frame.
		// render_target_texture_1->EndRender();
		//
		// render_target_texture_2->BeginRender();
		// normal renders and textures
		// render_target_texture_2->EndRender();
		//
		// render_target_texture_2 from previous frame must render before render_target_texture_1.
		//
		//
		// Example 3:
		//
		// render_target_texture_1->BeginRender();
		// use render_target_texture_2 from previous frame.
		// render_target_texture_1->EndRender();
		//
		// render_target_texture_2->BeginRender();
		// use render_target_texture_1
		// render_target_texture_2->EndRender();
		//
		// render_target_texture_2 from previous frame must render before render_target_texture_1.
		// render_target_texture_1 must render before render_target_texture_2.
		//
		//
		// Considering these, it should be possible to account all 3 example scenarios for as long
		// as we use double buffering and include which buffer the dependency is for.
		// Each buffer must wait for each other so that one is always free for recording commands.
		// It would be best to use a single queue submission from window render, this complicates things
		// as each render target texture gets queued with dependencies, we might need two queue submits
		// In example 3 for instance:
		// render_target_texture_1 uses previous frame from render_target_texture_2 so that's now locked
		// until render_target_texture_1 finishes render.
		// render_target_texture_2 then uses current frame from render_target_texture_1, so now
		// render_target_texture_2 also has a self dependendency to it's other buffer.
		//
		// Render target textures should also be usable accross windows so we need to prevent
		// double command buffer submissions somehow.
		//
		// In short, need to queue submit render targets before giving them to windows and need to
		// track render state per buffer. In case of conflicts we need a way of waiting for the
		// previous render to finish before starting the next one.
		//
		// Ideas:
		// - Submit on another thread.
		// - Submit after each render target end render, might be too slow even on a separate thread.
		// - Might be a good idea to queue and submit all render target textures on window->BeginRender().
		// - Need a way to submit render targets independently from window fps, Window::BeginRender() and
		//   RenderTargetTexture::BeginRender()s are good indicators, once Window::BeginRender() is called
		//   we can submit all render target texture command buffers there, we can reuse the render target
		//   until RenderTargetTexture::BeginRender() is called again, rinse and repeat.
		//
		// TODO: Timeline semaphores per render target texture is fine, however it should not be reset,
		// it's not allowed to roll backwards, increasing only. So we'll need a counter of how many times
		// the render target texture has been recorded, then once render finishes on the render target
		// we'll set the timeline semaphore to the number of times recorded. We'll also pass that value
		// on to other parts as a value to wait for when depending on the render target to finish render.

		//DrawRenderTargetTextureContent(
		//	delta_time,
		//	animation_counter,
		//	render_target_texture,
		//	texture_resource
		//);

		{
			if( !window->BeginRender() ) return -1;

			auto draw_rect_size = vk2d::Vector2f( 512, 512 );

			auto textured_box = vk2d::GenerateBoxMesh(
				draw_rect_size * vk2d::Vector2f( -1, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 0, 0.5 )
			);
			textured_box.SetTexture( render_target_texture );
			textured_box.SetSampler( pixel_sampler );
			window->DrawMesh( textured_box );

			auto grid = vk2d::GenerateLatticeMesh(
				draw_rect_size * vk2d::Vector2f( 0, -0.5 ),
				draw_rect_size * vk2d::Vector2f( 1, 0.5 ),
				vk2d::Vector2f( 30, 30 ),
				true
			);
			grid.SetTexture( render_target_texture );
			grid.SetSampler( pixel_sampler );
			grid.Wave(
				2.36f,
				1.4f,
				animation_counter,
				vk2d::Vector2f( 30, 30 )
			);
			grid.SetLineSize( 5 );

			window->DrawMesh( grid );

			grid.SetTexture( nullptr );
			grid.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
			grid.SetVertexColor( vk2d::Colorf( 0.1f, 1.0f, 0.3f, 0.2f ) );
			window->DrawMesh( grid );
		}

		if( !window->EndRender() ) return -1;
	}

	return 0;
}



void DrawRenderTargetTextureContent(
	float								delta_time,
	float								basic_animation_counter,
	vk2d::RenderTargetTexture		*	render_target_texture,
	vk2d::Texture					*	texture_resource
)
{
	auto render_target_size_f = vk2d::Vector2f( render_target_texture->GetSize().x, render_target_texture->GetSize().y );

	render_target_texture->BeginRender();

	auto textured_box = vk2d::GenerateBoxMesh(
		vk2d::Vector2f( 0, 0 ),
		render_target_size_f
	);
	textured_box.SetTexture( texture_resource );
	textured_box.SetVertexColor( vk2d::Colorf( 1, 1, 1, 1 ) );
	render_target_texture->DrawMesh( textured_box );


	render_target_texture->DrawPieBox(
		vk2d::Vector2f( 0, 0 ),
		render_target_size_f,
		std::sin( basic_animation_counter * 0.125f ) * PI * 2.0f,
		std::sin( basic_animation_counter * 0.237f ) * 0.5f + 0.5f,
		true,
		vk2d::Colorf( 1.0f, 1.0f, 1.0f, 0.6f )
	);

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
