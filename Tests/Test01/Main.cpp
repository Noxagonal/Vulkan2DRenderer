
#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



#include <string>
#include <iostream>



class EventHandler : public vk2d::WindowEventHandler {
public:
	void								VK2D_APIENTRY		EventWindowClose(
		vk2d::Window				*	window
	)
	{
		window->CloseWindow();
	};

	void								VK2D_APIENTRY		EventKeyboard(
		vk2d::Window				*	window,
		vk2d::KeyboardButton			button,
		int								scancode,
		vk2d::ButtonAction				action,
		vk2d::ModifierKeyFlags			modifierKeys
	)
	{
		if( action == vk2d::ButtonAction::PRESS ) {
			if( button == vk2d::KeyboardButton::KEY_ESCAPE ) {
				window->CloseWindow();
			}
			if( button == vk2d::KeyboardButton::KEY_PRINT_SCREEN ) {
				window->TakeScreenshot( "../../Screenshots/01.jpg", false );
			}
		}
	};

	void								VK2D_APIENTRY		EventScreenshot(
		vk2d::Window				*	window,
		const std::filesystem::path	&	path,
		bool							success,
		const std::string			&	errorMessage
	)
	{
		if( success ) {
			std::cout << "Screenshot saved successfully." << std::endl;
		} else {
			std::cout << "Screenshot error: " << errorMessage << std::endl;
		}
	};
};



int main()
{
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	vk2d::SamplerCreateInfo sampler_create_info {};
	sampler_create_info.address_mode_u		= vk2d::SamplerAddressMode::MIRRORED_REPEAT;
	sampler_create_info.address_mode_v		= vk2d::SamplerAddressMode::CLAMP_TO_BORDER;
	sampler_create_info.border_color		= vk2d::Colorf( 0.0f, 0.2f, 1.0f, 0.5f );
	auto sampler			= renderer->CreateSampler( sampler_create_info );

	auto texture			= renderer->GetResourceManager()->LoadTextureResource( "../../TestData/GrafGear_128.png" );
	auto font				= renderer->GetResourceManager()->LoadFontResource( "../../TestData/Fonts/Ethnocentric/ethnocentric rg.ttf" );
//	auto font				= renderer->GetResourceManager()->LoadFontResource( "../../TestData/Fonts/DroidSandMono/DroidSansMono.ttf" );

	EventHandler							event_handler;
	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	window_create_info.coordinate_space		= vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
//	window_create_info.samples				= renderer->GetMaximumSupportedMultisampling();
	window_create_info.event_handler		= &event_handler;
	auto window = renderer->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	size_t frame_counter = 0;
	while( !window->ShouldClose() ) {
		++frame_counter;
		if( !window->BeginRender() ) return -1;
		

//		window->DrawTexture(
//			{ -100, -100 },
//			{ +100, +100 },
//			texture,
//			{ 0.1f, 0.4f, 0.6f, 1.0f }
//		);

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
		lattice_mesh.SetVertexColorGradient(
			{ 0.0, 1.0, 0.0, 1.0 },
			{ 0.0, 0.0, 1.0, 1.0 },
			{ -200, -200 },
			{ 200, 200 }
		);
		lattice_mesh.SetMeshType( vk2d::MeshType::TRIANGLE_FILLED );
		lattice_mesh.SetTexture( font->GetTextureResource() );
//		lattice_mesh.SetTexture( texture );
		lattice_mesh.SetSampler( sampler );
//		lattice_mesh.Rotate( frame_counter / 234.0f, { +0.5f, +0.0f } );
		lattice_mesh.texture_channel_weights.resize( size_t( font->GetTextureResource()->GetLayerCount() ) * lattice_mesh.vertices.size() );
		srand( 20 );
		for( size_t i = 0; i < lattice_mesh.vertices.size(); ++i ) {
			size_t offset = i * font->GetTextureResource()->GetLayerCount();
			lattice_mesh.texture_channel_weights[ offset + 0 ]	= ( rand() % 1000 ) / 1000.0f;
			lattice_mesh.texture_channel_weights[ offset + 1 ]	= ( rand() % 1000 ) / 1000.0f;
			lattice_mesh.texture_channel_weights[ offset + 2 ]	= ( rand() % 1000 ) / 1000.0f;
			lattice_mesh.texture_channel_weights[ offset + 3 ]	= ( rand() % 1000 ) / 1000.0f;
//			lattice_mesh.texture_channel_weights[ offset + 1 ]	= 1.0f - lattice_mesh.texture_channel_weights[ offset + 0 ];
		}

		lattice_mesh.Wave(
			frame_counter / 500.0f,
			1.0f,
			frame_counter / 60.0f,
			{ 25.0f, 25.0f } );

		window->DrawMesh( lattice_mesh );

		{
			std::vector<vk2d::VertexIndex_3> indices( 1 );
			indices[ 0 ]	= { 0, 1, 2 };
			std::vector<vk2d::Vertex> vertices( 3 );
			vertices[ 0 ].vertex_coords		= { 0, -200 };
			vertices[ 0 ].uv_coords			= { 0.5, 0.0 };
			vertices[ 0 ].color				= { 1.0, 0.0, 0.0, 1.0 };
			vertices[ 0 ].point_size		= 1;
			vertices[ 1 ].vertex_coords		= { -200, 200 };
			vertices[ 1 ].uv_coords			= { 0.0, 1.0 };
			vertices[ 1 ].color				= { 0.0, 1.0, 0.0, 1.0 };
			vertices[ 1 ].point_size		= 1;
			vertices[ 2 ].vertex_coords		= { 200, 200 };
			vertices[ 2 ].uv_coords			= { 1.0, 1.0 };
			vertices[ 2 ].color				= { 0.0, 0.0, 1.0, 1.0 };
			vertices[ 2 ].point_size		= 1;

			window->DrawTriangleList( indices, vertices, {} );
		}

//		auto circle_mesh = vk2d::GenerateCircleMesh(
//			vk2d::Vector2f( -300, -300 ),
//			vk2d::Vector2f( -100, -100 ),
//			true,
//			64.0f
//		);
//		circle_mesh.SetVertexColor( vk2d::Colorf( 1, 0, 0, 1 ) );
//		circle_mesh.SetMeshType( vk2d::MeshType::TRIANGLE_WIREFRAME );
//		circle_mesh.Wave(
//			frame_counter / 500.0f,
//			1.0f,
//			frame_counter / 60.0f,
//			{ 15.0f, 15.0f },
//			{ -200.0f, -200.0f } );
//
//		window->DrawMesh(
//			circle_mesh
//		);


		/*
		auto box = vk2d::GenerateBoxMesh(
			{ -200, -200 },
			{ +200, +200 } );
		box.ScaleUV(
			{ 3.1f, 3.1f }
		);
		box.SetTexture( texture );
		box.SetSampler( sampler );

		window->DrawMesh( box );
		*/

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
