
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
	WindowCreateInfo			&	window_create_info
)
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



VK2D_API void VK2D_APIENTRY Window::Draw_TriangleList(
	bool								filled,
	std::vector<Vertex>			&		vertices,
	std::vector<VertexIndex_3>	&		indices
)
{
	if( impl ) {
		impl->Draw_TriangleList(
			filled,
			vertices,
			indices
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_LineList(
	std::vector<Vertex>			&	vertices,
	std::vector<VertexIndex_2>	&	indices
)
{
	if( impl ) {
		impl->Draw_LineList(
			vertices,
			indices
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_PointList(
	std::vector<Vertex>			&	vertices
)
{
	if( impl ) {
		impl->Draw_PointList(
			vertices
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Line(
	Coords							point_1,
	Coords							point_2,
	Color							color
)
{
	if( impl ) {
		impl->Draw_Line(
			point_1,
			point_2,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Box(
	bool							filled,
	Coords							top_left,
	Coords							bottom_right,
	Color							color
)
{
	if( impl ) {
		impl->Draw_Box(
			filled,
			top_left,
			bottom_right,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Circle(
	bool							filled,
	Coords							top_left,
	Coords							bottom_right,
	float							edge_count,
	Color							color
)
{
	if( impl ) {
		impl->Draw_Circle(
			filled,
			top_left,
			bottom_right,
			edge_count,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Pie(
	bool							filled,
	Coords							top_left,
	Coords							bottom_right,
	float							begin_angle_radians,
	float							end_angle_radians,
	float							edge_count,
	Color							color )
{
	if( impl ) {
		impl->Draw_Pie(
			filled,
			top_left,
			bottom_right,
			begin_angle_radians,
			end_angle_radians,
			edge_count,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_PieBox(
	bool							filled,
	Coords							top_left,
	Coords							bottom_right,
	float							radius,
	float							begin_angle_radians,
	float							end_angle_radians,
	Color							color )
{
	if( impl ) {
		impl->Draw_PieBox(
			filled,
			top_left,
			bottom_right,
			radius,
			begin_angle_radians,
			end_angle_radians,
			color
		);
	}
}





} // vk2d
