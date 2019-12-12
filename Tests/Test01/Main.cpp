
#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



#include <string>
#include <iostream>



class EventHandler : public vk2d::WindowEventHandler {
public:
	void							VK2D_APIENTRY		EventWindowClose(
		vk2d::Window			*	window
	)
	{
		window->CloseWindow();
	};

	void							VK2D_APIENTRY		EventKeyboard(
		vk2d::Window			*	window,
		vk2d::KeyboardButton		button,
		int							scancode,
		vk2d::ButtonAction			action,
		vk2d::ModifierKeyFlags		modifierKeys )
	{
		if( button == vk2d::KeyboardButton::KEY_ESCAPE && action == vk2d::ButtonAction::PRESS ) {
			window->CloseWindow();
		}
	};
};



int main()
{
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	auto texture			= renderer->GetResourceManager()->LoadTextureResource( "../../TestData/GrafGear_128.png" );

	EventHandler							event_handler;
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.width				= 800;
	window_create_info.height				= 600;
	window_create_info.coordinate_space		= vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
	window_create_info.event_handler		= &event_handler;
	auto window = renderer->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	size_t frame_counter = 0;
	while( !window->ShouldClose() ) {
		++frame_counter;
		if( !window->BeginRender() ) return -1;

		auto lattice_mesh = vk2d::GenerateLatticeMesh(
			{ -300.0f, -300.0f },
			{ +300.0f, +300.0f },
			{ 32.0f, 32.0f },
			true
		);
		for( auto & v : lattice_mesh.vertices ) {
			v.point_size	= 8.0f;
		}
		lattice_mesh.SetLineSize( 1.0f );
		lattice_mesh.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
		lattice_mesh.SetTexture( texture );
//		lattice_mesh.Rotate( frame_counter / 234.0f, { +0.5f, +0.0f } );

		// Wave is broken, investigate.
		lattice_mesh.Wave(
			0, //frame_counter / 50000.0f,
			5.5f,
			frame_counter / 60000.0f,
			{ 16.0f, 16.0f } );

		window->DrawMesh( lattice_mesh );

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
