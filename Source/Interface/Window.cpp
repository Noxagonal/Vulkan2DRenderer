
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Window.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Renderer.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Core/MeshBuffer.h"

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
	bool										filled,
	const std::vector<Vertex>				&	vertices,
	const std::vector<VertexIndex_3>		&	indices,
	TextureResource							*	texture
)
{
	if( impl ) {
		impl->Draw_TriangleList(
			filled,
			vertices,
			indices,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_LineList(
	const std::vector<Vertex>				&	vertices,
	const std::vector<VertexIndex_2>		&	indices,
	TextureResource							*	texture
)
{
	if( impl ) {
		impl->Draw_LineList(
			vertices,
			indices,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_PointList(
	const std::vector<Vertex>				&	vertices,
	TextureResource							*	texture
)
{
	if( impl ) {
		impl->Draw_PointList(
			vertices,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Line(
	Vector2d							point_1,
	Vector2d							point_2,
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
	Vector2d							top_left,
	Vector2d							bottom_right,
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
	Vector2d							top_left,
	Vector2d							bottom_right,
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
	Vector2d							top_left,
	Vector2d							bottom_right,
	float							begin_angle_radians,
	float							end_angle_radians,
	float							edge_count,
	Color							color
)
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
	Vector2d							top_left,
	Vector2d							bottom_right,
	float							begin_angle_radians,
	float							coverage,
	Color							color
)
{
	if( impl ) {
		impl->Draw_PieBox(
			filled,
			top_left,
			bottom_right,
			begin_angle_radians,
			coverage,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Texture(
	Vector2d						top_left,
	Vector2d						bottom_right,
	vk2d::TextureResource	*	texture,
	Color						color,
	bool						filled
)
{
	if( impl ) {
		impl->Draw_Texture(
			top_left,
			bottom_right,
			texture,
			color,
			filled
		);
	}
}

VK2D_API void VK2D_APIENTRY Window::Draw_Mesh(
	const vk2d::Mesh	&	mesh
)
{
	if( impl ) {
		impl->Draw_Mesh(
			mesh
		);
	}
}





} // vk2d
