
#include <VK2D.h>

#include "TestCommon.h"

#include "DrawShapesRenderTestSamples.h"

constexpr double PI = 3.14159265358979323846;
constexpr double RAD = PI * 2.0;

// Set to 1 to generate render samples, set to 0 if
// you want to test with already generated sample set.
#define GENERATE_RENDER_SAMPLES		0

#include <string>
#include <iostream>

class EventHandler : public vk2d::WindowEventHandler
{
	void VK2D_APIENTRY EventScreenshot(
		vk2d::Window					*	window,
		const std::filesystem::path		&	screenshot_path,
		const vk2d::ImageData			&	screenshot_data,
		bool								success,
		const std::string				&	errorMessage
	)
	{
		#if GENERATE_RENDER_SAMPLES	// Set to 1 to create samples, 0 to verify samples.
		// Create render samples.
		auto samples = GenerateSamples(
			screenshot_data,
			1000
		);
		SaveSamplesToHeaderFile(
			samples,
			"../../Tests/DrawShapesRenderTestSamples.h",
			"draw_shapes_render_samples"
		);
		#else
		// Verify render samples.
		if( VerifyImageWithSamples(
			draw_shapes_render_samples,
			screenshot_data
		) )
		{
			ExitWithCode( ExitCodes::SUCCESS );
		} else {
			ExitWithCode( ExitCodes::RENDER_DOES_NOT_MATCH_EXPECTED_RESULT );
		}
		#endif
	}
};

int main()
{
	vk2d::InstanceCreateInfo instance_create_info{};
	auto instance = vk2d::CreateInstance(instance_create_info);
	if (!instance) ExitWithCode( ExitCodes::CANNOT_CREATE_INSTANCE );

	EventHandler event_handler;
	vk2d::WindowCreateInfo				window_create_info{};
	window_create_info.size				= { 512, 512 };
	window_create_info.event_handler	= &event_handler;
	window_create_info.coordinate_space = vk2d::RenderCoordinateSpace::TEXEL_SPACE;
	auto window = instance->CreateOutputWindow(window_create_info);
	if (!window) ExitWithCode( ExitCodes::CANNOT_CREATE_WINDOW );

	window->TakeScreenshotToData( true );

	CoordGrid grid(
		window->GetSize(),
		vk2d::Vector2f( 100.0f, 100.0f )
	);

	size_t frame_counter = 500;
	while( frame_counter ) {

		if( !window->BeginRender() ) ExitWithCode( ExitCodes::CANNOT_BEGIN_RENDER );

		window->DrawBox(
			Coords( grid ),
			true,
			vk2d::Colorf( 1.0f, 0.0f, 0.0f, 1.0f )
		);

		window->DrawBox(
			Coords( grid ),
			false,
			vk2d::Colorf( 1.0f, 0.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			3.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			3.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			5.5f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			5.5f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			16.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			16.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			32.0f,
			vk2d::Colorf( 0.0f, 1.0f, 1.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			32.0f,
			vk2d::Colorf( 0.0f, 1.0f, 1.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			32.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			32.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			true,
			64.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawEllipse(
			Coords( grid ),
			false,
			64.0f,
			vk2d::Colorf( 0.0f, 1.0f, 0.0f, 1.0f )
		);

		window->DrawPie(
			Coords( grid ),
			1.1f,
			0.9f,
			true,
			64.0f,
			vk2d::Colorf( 0.0f, 0.0f, 1.0f, 1.0f )
		);

		window->DrawPie(
			Coords( grid ),
			1.1f,
			0.9f,
			false,
			64.0f,
			vk2d::Colorf( 0.0f, 0.0f, 1.0f, 1.0f )
		);

		window->DrawPie(
			Coords( grid ),
			0.6f,
			0.33f,
			true,
			64.0f,
			vk2d::Colorf( 0.0f, 0.0f, 1.0f, 1.0f )
		);

		window->DrawPie(
			Coords( grid ),
			0.6f,
			0.33f,
			false,
			64.0f,
			vk2d::Colorf( 0.0f, 0.0f, 1.0f, 1.0f )
		);

		window->DrawPieBox(
			Coords( grid ),
			1.1f,
			0.9f,
			true,
			vk2d::Colorf( 1.0f, 0.7f, 0.2f, 1.0f )
		);

		window->DrawPieBox(
			Coords( grid ),
			1.1f,
			0.9f,
			false,
			vk2d::Colorf( 1.0f, 0.7f, 0.2f, 1.0f )
		);

		window->DrawPieBox(
			Coords( grid ),
			0.6f,
			0.77f,
			true,
			vk2d::Colorf( 1.0f, 0.7f, 0.2f, 1.0f )
		);

		window->DrawPieBox(
			Coords( grid ),
			0.6f,
			0.77f,
			false,
			vk2d::Colorf( 1.0f, 0.7f, 0.2f, 1.0f )
		);

		window->DrawLine(
			Coords( grid ),
			vk2d::Colorf( 0.0f, 0.0f, 1.0f, 1.0f )
		);

		window->DrawPoint(
			grid.InsertTopLeft(),
			vk2d::Colorf( 1.0f, 0.0f, 1.0f, 1.0f ),
			10.0f
		);

		grid.Reset();

		if( !window->EndRender() ) ExitWithCode( ExitCodes::CANNOT_END_RENDER );

		--frame_counter;
	}

	return 0;
}
