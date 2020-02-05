
#include <VK2D.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;


#include <string>
#include <iostream>


int main()
{
	vk2d::InstanceCreateInfo instance_create_info {};
	auto instance = vk2d::CreateInstance( instance_create_info );
	if( !instance ) return -1;

	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 600 };
	window_create_info.coordinate_space		= vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	size_t frame_counter = 10;
	while( frame_counter ) {

		if( !window->BeginRender() ) return -1;

		if( !window->EndRender() ) return -1;

		--frame_counter;
	}

	return 0;
}
