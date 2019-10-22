
#include "../Header/SourceCommon.h"

#include "../../Include/VK2D/Window.h"
#include "../Header/WindowImpl.h"

#include "../../Include/VK2D/Renderer.h"
#include "../Header/RendererImpl.h"

#include "../Header/MeshBuffer.h"

#include "../../Include/VK2D/Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <array>



namespace vk2d {



VK2D_API					Window::Window(
	_internal::RendererImpl		*	renderer_parent,
	WindowCreateInfo			&	window_create_info )
{
	impl	= std::make_unique<vk2d::_internal::WindowImpl>(
		renderer_parent,
		window_create_info
		);
	if( !impl ) {
		if( renderer_parent->GetReportFunction() ) {
			renderer_parent->GetReportFunction()( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return;
	}
	if( !impl->is_good ) return;

	is_good							= true;
}



VK2D_API Window::~Window()
{
	impl	= nullptr;
}



VK2D_API bool VK2D_APIENTRY Window::BeginRender()
{
	if( impl ) {
		return impl->BeginRender();
	}
	return {};
}



VK2D_API bool VK2D_APIENTRY Window::EndRender()
{
	if( impl ) {
		return impl->EndRender();
	}
	return {};
}



/*

VK2D_API void VK2D_APIENTRY Window::Draw_TriangleList(
	bool								filled,
	std::vector<Vertex>				&	vertices,
	std::vector<VertexIndex_3>		&	indices )
{
	// TODO: we need a mesh buffer that is able to keep track of changing vertices
	// and indices as well as record command to copy data from host to device.
	todo here;

	auto command_buffer			= data->command_buffers[ data->next_image ];
	auto & frame_mesh_buffer	= data->frame_mesh_buffer[ data->next_image ];

	if( filled ) {
		vkCmdBindPipeline(
			command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			data->pipelines[ uint32_t( _internal::PipelineType::FILLED_POLYGON_LIST ) ]
		);
	}

	vkCmdDrawIndexed(
		data->command_buffers[ data->next_image ],
		uint32_t( indices.size() ),
		1,
		uint32_t( indices.size() + frame_indices.size() ),
		frame_vertices.size(),
		0
	);

	frame_vertices.insert( frame_vertices.end(), vertices.begin(), vertices.end() );

	frame_indices.reserve( frame_vertices.size() + indices.size() * 3 );
	for( auto & i : indices ) {
		frame_indices.push_back( i.indices[ 0 ] );
		frame_indices.push_back( i.indices[ 1 ] );
		frame_indices.push_back( i.indices[ 2 ] );
	}
}

*/



} // vk2d
