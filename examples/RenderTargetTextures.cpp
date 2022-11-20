
// This program shows how to use render target textures.
//
// Examples to see before trying this example:
// - HelloWorld
// - DrawMeshShapes
//

#include <VK2D.hpp>



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

	vk2d::WindowCreateInfo window_create_info {};
	window_create_info.size					= { 800, 600 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	// Create render target texture with size 512 x 512. You can use any size you want but
	// keep in mind that internally this is a collection of 4 to 8 images and thus memory
	// requirements are larger for render target textures than regular textures.
	// You can enable multisampling just like for a window surface, as well as enable
	// real time blur.
	// However you should only enable multisampling and blur if you need them as they will
	// increase video memory consumption and performance requirements further.
	// In here we purposfully use very low resolution render target texture, in real
	// application you'd probably want to use larger size.
	vk2d::RenderTargetTextureCreateInfo render_target_texture_create_info {};
	render_target_texture_create_info.size				= { 128, 128 };
	render_target_texture_create_info.coordinate_space	= vk2d::RenderCoordinateSpace::NORMALIZED_SPACE_CENTERED;
	render_target_texture_create_info.enable_blur		= false;
	render_target_texture_create_info.samples			= vk2d::Multisamples::SAMPLE_COUNT_1;
	auto render_target_texture = instance->CreateRenderTargetTexture( render_target_texture_create_info );

	// This is just to make it more obvious that we're indeed drawing into a texture.
	auto mesh = vk2d::GenerateLatticeMesh(
		{ -300.0f, -300.0f,
		300.0f, 300.0f },
		{ 32.0f, 32.0f }
	);

	// Setting a texture to mesh is fine like this as a mesh is only a collection of
	// settings used to draw, the texture is only stored as a pointer for now and only
	// used when we're actually drawing the mesh to something.
	mesh.SetTexture( render_target_texture );

	DeltaTimeCounter delta_time_counter;
	auto seconds_from_launch = 0.0f;

	while( instance->Run() &&  !window->ShouldClose() ) {

		// Get delta time
		auto delta_time = delta_time_counter.Tick();
		seconds_from_launch += delta_time;

		// Render target texture behaves similarly to window surface when it comes to
		// drawing to it. You must call BeginRender() before rendering operations.
		// You can draw to the render target texture outside the main loop, it does
		// not need to be re-drawn each frame. You can also use render target textures
		// inside other render target textures.
		render_target_texture->BeginRender();

		render_target_texture->DrawEllipsePie(
			{ -0.9f, -0.9f,
			0.9f, 0.9f },
			( std::sin( seconds_from_launch / 2.0f ) * 0.5f + 0.5f ) * 3.14f * 2.0f,
			std::sin( seconds_from_launch / 4.0f ) * 0.5f + 0.5f,
			true,
			64.0f,
			{ 0.5f, 0.2f, 0.7f, 1.0f }
		);

		// Similarly to window, you must call EndRender() once you're done drawing to
		// the render target texture. RenderTargetTexture::EndRender() also takes in
		// blur amount parameters if you have blur enabled, this is the only way to
		// blur things in VK2D. Try enabling the blur and adjust the blur amount here.
		render_target_texture->EndRender( { 20.0f, 20.0f } );



		// Render target texture render block must finish before the next render
		// block that uses the render target texture begins.
		// In this case, "render_target_texture" is used by a draw command inside
		// "window" render block so "render_target_texture->EndRender()" must be
		// called before "window->BeginRender()" is called. If window didn't use
		// "render_target_texture" then you could overlap the render blocks.
		if( !window->BeginRender() ) return -1;

		// Make a copy of the mesh, usually you should do this outside of a render
		// block but to keep everything tied together for illustration purposes,
		// this is fine for now.
		auto mesh_copy = mesh;

		// For illustration purposes we'll apply a wave modifier.
		mesh_copy.DirectionalWave(
			1.2f,
			2.0f,
			0.0f,
			{ 20.0f, 20.0f }
		);

		// Draw the mesh copy to window. Note that "mesh_copy" is a copy of "mesh"
		// which stores a pointer to the "render_target_texture", so this means that
		// "render_target_texture" is now actually used in this render block.
		window->DrawMesh( mesh_copy );

		// Rest of this is only for visual aid by drawing a wireframe on top.
		mesh_copy.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
		mesh_copy.SetTexture( nullptr );
		mesh_copy.SetVertexColorGradient(
			{ 0.2f, 1.0f, 1.0f, 0.0f },
			{ 0.2f, 1.0f, 0.4f, 0.3f },
			{ -100.0f, -250.0f },
			{ 100.0f, 300.0f }
		);
		window->DrawMesh( mesh_copy );

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
