
// This program shows how to use Mesh objects to draw stuff with.
//
// Examples to see before trying this example:
// - HelloWorld
//
// Meshes are objects that contain everything required to draw
// something on the window or render target texture.
// You could consider them like a reusable collection of commands
// needed to draw something visible.
// Mesh objects contain everything required for the draw, vertices,
// polygon list, UVs, texture, sampler...

#include <VK2D.h>
#include <chrono>



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

	vk2d::WindowCreateInfo					window_create_info {};
	window_create_info.size					= { 800, 800 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;

	// You can generate a mesh object at any point in your application, it does not rely
	// on any other VK2D objects to create. Generating a mesh is fast but not entirely
	// trivial, so for performance reasons it is better to generate static meshes outside
	// the main loop whenever possible.
	auto box_mesh = vk2d::GenerateRectangleMesh(
		{ -300.0f, -300.0f, 300.0f, 300.0f }
	);

	// This is a mesh modifier, every modifier does something to the vertex or index data.
	// In this case every vertice's color is set to a value.
	// You can modify the data directly as well, in fact this function is equivalent to:
	// for( auto & v : box_mesh.vertices ) {
	//     v.color = { 1.0f, 1.0f, 1.0f, 0.2f };
	// }
	box_mesh.SetVertexColor(
		{ 1.0f, 1.0f, 1.0f, 0.15f }
	);

	// Generate lattice mesh.
	// Lattice mesh is a grid like mesh with variable grid size on both X and Y axis.
	auto lattice_mesh = vk2d::GenerateLatticeMesh(
		{ -250.0f, -250.0f, 250.0f, 250.0f },
		{ 32.0f, 32.0f },
		false
	);

	// Mesh modifier, sets all vertex colors in gradient from the first
	// color to the second color linearly between the two given points.
	lattice_mesh.SetVertexColorGradient(
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ -300.0f, -300.0f },
		{ 300.0f, 300.0f }
	);

	DeltaTimeCounter delta_time_counter;
	auto seconds_from_launch = 0.0f;

	while( instance->Run() && !window->ShouldClose() ) {

		// Get delta time.
		auto delta_time			= delta_time_counter.Tick();
		seconds_from_launch		+= delta_time;

		// We created a lattice mesh earlier and we would like to modify it.
		// These "modifiers" transform the internal data of the mesh object
		// and if we did that continously to the original object it would
		// be a garbled mess after a couple of frames, so instead we make
		// a copy of the original and only modify the copy.
		auto lattice_mesh_copy = lattice_mesh;

		// Wave modifier, applies a simple wave animation to any mesh.
		lattice_mesh_copy.Wave(
			1.10f,
			2.0f,
			seconds_from_launch,
			{ 30.0f, 30.0f }
		);

		if( !window->BeginRender() ) return -1;

		// A mesh object contains everything needed to draw it, including texture.
		window->DrawMesh( box_mesh );
		window->DrawMesh( lattice_mesh_copy );

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
