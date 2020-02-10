
#include <VK2D.h>

#include "TestCommon.h"

constexpr double PI = 3.14159265358979323846;
constexpr double RAD = PI * 2.0;


#include <string>
#include <iostream>


int main()
{
	vk2d::InstanceCreateInfo instance_create_info{};
	auto instance = vk2d::CreateInstance(instance_create_info);
	if (!instance) ExitWithCode(ExitCodes::CANNOT_CREATE_INSTANCE);

	vk2d::WindowCreateInfo					window_create_info{};
	window_create_info.size = { 512, 512 };
	window_create_info.coordinate_space = vk2d::WindowCoordinateSpace::TEXEL_SPACE_CENTERED;
	auto window = instance->CreateOutputWindow(window_create_info);
	if (!window) ExitWithCode(ExitCodes::CANNOT_CREATE_WINDOW);

	size_t frame_counter = 3;
	while (frame_counter) {

		if (!window->BeginRender()) ExitWithCode(ExitCodes::CANNOT_BEGIN_RENDER);

		if (!window->EndRender()) ExitWithCode(ExitCodes::CANNOT_END_RENDER);

		--frame_counter;
	}

	ExitWithCode( ExitCodes::SUCCESS );
}
