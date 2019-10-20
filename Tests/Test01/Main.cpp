
#include <Vulkan2DRenderer.h>

int main()
{
	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	vk2d::WindowCreateInfo window_create_info {};
	window_create_info.width	= 800;
	window_create_info.height	= 600;
	auto window = renderer->CreateWindowOutput( window_create_info );

	size_t frame_count_remining = 200;
	while( frame_count_remining ) {
		--frame_count_remining;
		if( !window->BeginRender() ) return -1;
		if( !window->EndRender() ) return -1;
	}

	return 0;
}
