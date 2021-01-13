
// This program shows how to use Mesh objects in combination with
// transformations to draw shapes.
//
// Examples to see before trying this example:
// - HelloWorld
// - DrawMeshShapes
//
// Transformations are additional spacial calculations done
// to the vertex data and can be used to define scene hierarchy.
// VK2D transformations define location, rotation, and scale.

#include <VK2D.h>
#include <chrono>
#include <iostream>



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
	window_create_info.size					= { 800, 600 };
	window_create_info.coordinate_space		= vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED;
	auto window = instance->CreateOutputWindow( window_create_info );
	if( !window ) return -1;


	// We'll create a simple rectangle mesh, note that besides drawing
	// the mesh it's data is never touched inside the main loop.
	auto box_mesh = vk2d::GenerateRectangleMesh(
		{ -50.0f, -50.0f, 50.0f, 50.0f }
	);

	// For visual aid, shows where each corner lands.
	box_mesh.vertices[ 0 ].color	= { 1.0f, 0.0f, 0.0f, 1.0f };
	box_mesh.vertices[ 1 ].color	= { 0.0f, 1.0f, 0.0f, 1.0f };
	box_mesh.vertices[ 2 ].color	= { 0.0f, 0.0f, 1.0f, 1.0f };
	box_mesh.vertices[ 3 ].color	= { 1.0f, 1.0f, 1.0f, 1.0f };

	DeltaTimeCounter delta_time_counter;
	auto seconds_from_launch = 0.0f;

	bool first_frame = true;

	while( instance->Run() && !window->ShouldClose() ) {

		// Get delta time.
		auto delta_time			= delta_time_counter.Tick();
		seconds_from_launch		+= delta_time;


		// To make it easy to give transformation information to the mesh to be drawn, VK2D
		// incorporates Transform class which contains location, rotation and scale data.
		// Here we set:
		// Location follows a counter-clockwise rotation.
		// Animate the scale x axis, leave scale y axis as 1.0.
		// Animate rotation.
		auto origin = vk2d::Transform(
			{ glm::cos( -seconds_from_launch ) * 100.0f, glm::sin( -seconds_from_launch ) * 100.0f },
			{ glm::sin( seconds_from_launch ) * 0.5f + 1.0f, 1.0f },
			seconds_from_launch / 3.0f
		);

		// The final transform object can be composed to a transformation matrix.
		// This transformation matrix is useful when calculating parent-child
		// transformation relationships.
		auto origin_matrix = origin.CalculateTransformationMatrix();

		if( first_frame ) {
			std::cout << origin_matrix << "\n";
			first_frame = false;
		}

		// We want one of the box meshes to follow transformations of the origin,
		// so information given here is intented to be relative to "origin" transform.
		auto child = vk2d::Transform(
			{ 150.0f, 0.0f },
			{ 1.0f, 1.0f },
			0.0f
		);
		auto child_matrix = child.CalculateTransformationMatrix();

		// Same as "child" transform but this is intended to follow "child"'s transform.
		auto sub_child = vk2d::Transform(
			{ 0.0f, glm::sin( seconds_from_launch / 2.0f ) * 150.0f },
			{ 1.0f, 1.0f },
			glm::sin( seconds_from_launch )
		);
		auto sub_child_matrix = sub_child.CalculateTransformationMatrix();


		// Calculate final transformations. Parent matrix is on the left side of the multiplication
		// and child matrix is on the right side. The final result is that the child follows the
		// parent's transformation. In fact the child's entire coordinate space becomes local to the
		// parent, eg. shrink the parent and you don't only shrink the child but you also move the
		// child closer to the parent on the world coordinate space, in effect you shrunk the
		// coordinate space of the child.
		auto child_transformation_matrix = origin_matrix * child_matrix;

		// There is no limits how many sub-children there can be, in this case the "sub_child"
		// follows "child" which follows "origin". Another way of writing this is:
		// sub_child_transformation_matrix		= child_transformation_matrix * sub_child_matrix;
		// Which allows full scene hierarchy structures.
		auto sub_child_transformation_matrix = origin_matrix * child_matrix * sub_child_matrix;



		if( !window->BeginRender() ) return -1;

		// Window::DrawMesh() can take 3 types of transformation parameters:
		// vk2d::Transform object, std::array of vk2d::Transform objects or a std::array of
		// glm::mat4. Matrices are the only things capable of scene parent-child hierarchy.
		// Here vk2d::Transform object is used directly when drawing a mesh for easier use.
		window->DrawMesh( box_mesh, origin );

		// If you omit the transformations then default transformations are used:
		// window->DrawMesh( box_mesh ); -> is same as -> window->DrawMesh( box_mesh, {} );
		// Where {} defaults to location 0.0, 0.0, scale 1.0, 1.0, rotation 0.0.

		// Draw "box_mesh" twice using 2 different transformations. You can use 2 draw calls
		// with separate matrices and it will work just as fine:
		// window->DrawMesh( box_mesh, { child_transformation_matrix } );
		// window->DrawMesh( box_mesh, { sub_child_transformation_matrix } );
		// However the point of using only one draw command is to take advantage of instancing
		// which is more efficient, especially in cases where you need to draw loads of small
		// objects. So all this does is draws the same mesh twice using 2 different transformations.
		window->DrawMesh( box_mesh, { child_transformation_matrix, sub_child_transformation_matrix } );

		// Since we're actually just drawing the same mesh 3 times we could combine all draw
		// calls into one like this:
		// window->DrawMesh( box_mesh, { origin_matrix, child_transformation_matrix, sub_child_transformation_matrix } );

		// You can also use a std::vector of vk2d::Transform objects to draw them all at once.
		// Try out this code, just bare in mind that you won't get scene hierarchy this way.
		// window->DrawMesh( box_mesh, { origin, child, sub_child } );

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
