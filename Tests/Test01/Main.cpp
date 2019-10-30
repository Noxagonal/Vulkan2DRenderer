
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

		window->Draw_PieBox(
			false,
			{ -0.5f, -0.5f },
			{ +0.5f, +0.5f },
			frame_counter / 100.0f,
			std::sin( frame_counter / 123.0f ) / 2.0f + 0.5f
		);

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
