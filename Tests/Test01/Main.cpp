
#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



#include <string>
#include <iostream>



int main()
{
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	auto texture			= renderer->GetResourceManager()->LoadTextureResource( "../../TestData/GrafGear_128.png" );

	vk2d::WindowCreateInfo window_create_info {};
	window_create_info.width	= 800;
	window_create_info.height	= 600;
	auto window = renderer->CreateWindowOutput( window_create_info );
	if( !window ) return -1;

	size_t frame_counter = 0;
	while( true ) {
		++frame_counter;
		if( !window->BeginRender() ) return -1;
		/*
		window->Draw_PieBox(
			true,
			{ -0.95f, -0.5f },
			{ -0.05f, +0.5f },
			frame_counter / 100.0f,
			std::sin( frame_counter / 123.0f ) / 2.0f + 0.5f
		);

		window->Draw_Pie(
			true,
			{ +0.05f, -0.5f },
			{ +0.95f, +0.5f },
			frame_counter / 100.0f,
			16.0f
		);

		vk2d::Vector2d position { -0.9f, -0.9f };

		window->Draw_Texture(
			position,
			position + vk2d::Vector2d( ( std::cos( frame_counter / 123.0f ) / 10.0f + 0.3f ), ( std::sin( frame_counter / 123.0f ) / 10.0f + 0.3f ) ),
			texture,
			{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
		*/

		auto pie_box_mesh = vk2d::GeneratePieBoxMesh(
			{ -1.0f, -0.5f },
			{ +0.0f, +0.5f },
			frame_counter / 100.0f,
			std::sin( frame_counter / 123.0f ) / 2.0f + 0.5f,
			true );
//		pie_box_mesh.line_width		= 16.0f;
//		pie_box_mesh.mesh_type		= vk2d::MeshType::TRIANGLE_WIREFRAME;
		pie_box_mesh.texture		= texture;
		pie_box_mesh.Translate( { std::cos( frame_counter / 100.0f ) / 10.0f, std::sin( frame_counter / 100.0f ) / 10.0f } );
		window->DrawMesh( pie_box_mesh );

		auto lattice_mesh = vk2d::GenerateLatticeMesh(
			{ -0.0f, -0.5f },
			{ +1.0f, +0.5f },
			{ ( std::cos( frame_counter / 123.0f ) * 0.5f + 0.5f ) * 6.0f + 64.0f, ( std::sin( frame_counter / 123.0f ) * 0.5f + 0.5f ) * 6.0f + 64.0f },
			false
		);
		for( auto & v : lattice_mesh.vertices ) {
			v.point_size	= 8.0f;
		}
		lattice_mesh.line_width	= 1.0f;
		lattice_mesh.mesh_type	= vk2d::MeshType::POINT;
		lattice_mesh.texture	= texture;
		lattice_mesh.Rotate( frame_counter / 234.0f, { +0.5f, +0.0f } );

		window->DrawMesh( lattice_mesh );

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
