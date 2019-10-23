
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

	std::vector<vk2d::Vertex> vertices;
	std::vector<vk2d::VertexIndex_3> indices;
	vertices.push_back( {
		{ 0.0f, -0.5f },			// vertex_coords (xy, 0,0 -> screen centre )
		{},							// uv_coords (uv)
		{ 1.0f, 0.0f, 0.0f, 1.0f }	// color (rgba)
		} );
	vertices.push_back( {
		{ -0.5f, 0.5f },			// vertex_coords (xy, 0,0 -> screen centre )
		{},							// uv_coords (uv)
		{ 0.0f, 1.0f, 0.0f, 1.0f }	// color (rgba)
		} );
	vertices.push_back( {
		{ 0.5f, 0.5f },				// vertex_coords (xy, 0,0 -> screen centre )
		{},							// uv_coords (uv)
		{ 0.0f, 0.0f, 1.0f, 1.0f }	// color (rgba)
		} );
	indices.push_back( {
		{ 0, 1, 2 }					// Form a triangle between these vertices
		} );


	size_t frame_count_remining = 200;
	while( frame_count_remining ) {
		--frame_count_remining;
		if( !window->BeginRender() ) return -1;

		// TODO: PROBLEM AT WindowImpl.h : ~350
		// Problem I didn't consider before is that the vertex buffer needs to be bound first,
		// Sounds obvious in retrospect. -_-
		// Possible solutions:
		// First one is to just postpone command buffer recording till EndRender()
		// and to just keep track internally of what gets recorded and where.
		// Second option is a bit easier and might perform better in the end is to
		// create multiple fixed lenght mesh buffers dynamically as needed and bind them
		// just before recording the draw command, then record a command to copy the
		// vertex buffer data to memory as usual.
		/*
		window->Draw_TriangleList(
			true,
			vertices,
			indices
		);
		*/
		if( !window->EndRender() ) return -1;
	}

	return 0;
}
