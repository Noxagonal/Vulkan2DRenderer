
#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;

int main()
{
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	vk2d::WindowCreateInfo window_create_info {};
	window_create_info.width	= 800;
	window_create_info.height	= 600;
	auto window = renderer->CreateWindowOutput( window_create_info );

	size_t frame_counter = 0;
	while( true ) {
		++frame_counter;
		if( !window->BeginRender() ) return -1;

//		window->Draw_Line( { -0.5f, -0.3f }, { +0.3, +0.5 } );
//		window->Draw_Box( true, { 0.1, 0.1 }, { 0., 0.3 } );
		window->Draw_Pie(
			true,
			{ -0.8, -0.5f },
			{ -0.1f, +0.5f },
			std::sin( frame_counter / 120.0f ),
			( ( std::sin( frame_counter / 200.0f ) + 0.9f ) / 1.8f ),
			( std::sin( frame_counter / 100.0f ) + 2.0f ) * 10.0f
		);

		window->Draw_Pie(
			true,
			{ +0.1f, -0.5f },
			{ +0.8f, +0.5f },
			0.0f,
			0.75f
		);

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
