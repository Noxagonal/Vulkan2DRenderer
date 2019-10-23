
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



VK2D_API void VK2D_APIENTRY Window::Draw_TriangleList(
	bool								filled,
	std::vector<Vertex>			&		vertices,
	std::vector<VertexIndex_3>	&		indices )
{
	if( impl ) {
		impl->Draw_TriangleList(
			filled,
			vertices,
			indices
		);
	}
}





} // vk2d
