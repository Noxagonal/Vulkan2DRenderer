
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



VK2D_API					vk2d::Window::Window(
	vk2d::_internal::RendererImpl	*	renderer_parent,
	vk2d::WindowCreateInfo			&	window_create_info
)
{
	impl	= std::make_unique<vk2d::_internal::WindowImpl>(
		this,
		renderer_parent,
		window_create_info
		);
	if( !impl ) {
		if( renderer_parent->GetReportFunction() ) {
			renderer_parent->GetReportFunction()( vk2d::ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return;
	}
	if( !impl->is_good ) {
		impl = nullptr;
		return;
	}
	is_good							= true;
}



VK2D_API vk2d::Window::~Window()
{
	impl	= nullptr;
}

VK2D_API void VK2D_APIENTRY vk2d::Window::CloseWindow()
{
	if( impl ) {
		impl->CloseWindow();
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::ShouldClose()
{
	if( impl ) {
		return impl->ShouldClose();
	}
	return true;
}

VK2D_API void VK2D_APIENTRY vk2d::Window::UpdateEvents()
{
	if( impl ) {
		impl->UpdateEvents();
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::TakeScreenshot(
	const std::filesystem::path		&	save_path,
	bool								include_alpha
)
{
	if( impl ) {
		impl->TakeScreenshot(
			save_path,
			include_alpha
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::Focus()
{
	if( impl ) {
		impl->Focus();
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetOpacity(
	float			opacity
)
{
	if( impl ) {
		impl->SetOpacity( opacity );
	}
}

VK2D_API float VK2D_APIENTRY vk2d::Window::GetOpacity()
{
	if( impl ) {
		return impl->GetOpacity();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::Hide(
	bool			hidden
)
{
	if( impl ) {
		impl->Hide( hidden );
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsHidden()
{
	if( impl ) {
		return impl->IsHidden();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DisableEvents(
	bool				disable_events
)
{
	if( impl ) {
		impl->DisableEvents( disable_events );
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::AreEventsDisabled()
{
	if( impl ) {
		return impl->AreEventsDisabled();
	}
	return true;
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetFullscreen(
	vk2d::Monitor		*	monitor,
	uint32_t				frequency )
{
	if( impl ) {
		impl->SetFullscreen( monitor, frequency );
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsFullscreen()
{
	if( impl ) {
		return impl->IsFullscreen();
	}
	return {};
}

VK2D_API vk2d::Vector2d VK2D_APIENTRY vk2d::Window::GetCursorPosition()
{
	if( impl ) {
		return impl->GetCursorPosition();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursorPosition(
	vk2d::Vector2d			new_position
)
{
	if( impl ) {
		impl->SetCursorPosition( new_position );
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursor(
	vk2d::Cursor		*	cursor
)
{
	if( impl ) {
		impl->SetCursor( cursor );
	}
}

VK2D_API std::string VK2D_APIENTRY vk2d::Window::GetClipboardString()
{
	if( impl ) {
		return impl->GetClipboardString();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetClipboardString( const std::string & str )
{
	if( impl ) {
		impl->SetClipboardString( str );
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetTitle( const std::string & title )
{
	if( impl ) {
		impl->SetTitle( title );
	}
}

VK2D_API std::string VK2D_APIENTRY vk2d::Window::GetTitle()
{
	if( impl ) {
		return impl->GetTitle();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetIcon(
	const std::vector<std::filesystem::path>		&	image_paths )
{
	if( impl ) {
		impl->SetIcon( image_paths );
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetPosition(
	vk2d::Vector2i			new_position
)
{
	if( impl ) {
		impl->SetPosition( new_position );
	}
}

VK2D_API vk2d::Vector2i VK2D_APIENTRY vk2d::Window::GetPosition()
{
	if( impl ) {
		return impl->GetPosition();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetSize(
	vk2d::Vector2u		new_size
)
{
	impl->SetSize( new_size );
}

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::Window::GetSize()
{
	return impl->GetSize();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::Iconify(
	bool		iconified
)
{
	if( impl ) {
		impl->Iconify( iconified );
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsIconified()
{
	if( impl ) {
		return impl->IsIconified();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetMaximized(
	bool		maximized
)
{
	if( impl ) {
		impl->SetMaximized( maximized );
	}
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::GetMaximized()
{
	if( impl ) {
		return impl->GetMaximized();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursorState(
	vk2d::CursorState		new_state
)
{
	if( impl ) {
		impl->SetCursorState( new_state );
	}
}

VK2D_API vk2d::CursorState VK2D_APIENTRY vk2d::Window::GetCursorState()
{
	if( impl ) {
		return impl->GetCursorState();
	}
	return {};
}







VK2D_API bool VK2D_APIENTRY vk2d::Window::BeginRender()
{
	if( impl ) {
		return impl->BeginRender();
	}
	return {};
}



VK2D_API bool VK2D_APIENTRY vk2d::Window::EndRender()
{
	if( impl ) {
		return impl->EndRender();
	}
	return {};
}



VK2D_API void VK2D_APIENTRY vk2d::Window::DrawTriangleList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	bool										filled,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	if( impl ) {
		impl->DrawTriangleList(
			vertices,
			indices,
			filled,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawLineList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	if( impl ) {
		impl->DrawLineList(
			vertices,
			indices,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	if( impl ) {
		impl->DrawPointList(
			vertices,
			texture
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawLine(
	vk2d::Vector2f					point_1,
	vk2d::Vector2f					point_2,
	vk2d::Colorf					color
)
{
	if( impl ) {
		impl->DrawLine(
			point_1,
			point_2,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	vk2d::Colorf					color
)
{
	if( impl ) {
		impl->DrawBox(
			top_left,
			bottom_right,
			filled,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawCircle(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	if( impl ) {
		impl->DrawCircle(
			top_left,
			bottom_right,
			filled,
			edge_count,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPie(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							end_angle_radians,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	if( impl ) {
		impl->DrawPie(
			top_left,
			bottom_right,
			begin_angle_radians,
			end_angle_radians,
			filled,
			edge_count,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPieBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	vk2d::Colorf					color
)
{
	if( impl ) {
		impl->DrawPieBox(
			top_left,
			bottom_right,
			begin_angle_radians,
			coverage,
			filled,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawTexture(
	vk2d::Vector2f				top_left,
	vk2d::Vector2f				bottom_right,
	vk2d::TextureResource	*	texture,
	vk2d::Colorf				color
)
{
	if( impl ) {
		impl->DrawTexture(
			top_left,
			bottom_right,
			texture,
			color
		);
	}
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawMesh(
	const vk2d::Mesh	&	mesh
)
{
	if( impl ) {
		impl->DrawMesh(
			mesh
		);
	}
}







VK2D_API vk2d::Cursor::Cursor(
	const std::filesystem::path		&	image_path,
	vk2d::Vector2i						hot_spot
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		image_path,
		hot_spot
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

VK2D_API vk2d::Cursor::Cursor(
	vk2d::Vector2u							image_size,
	const std::vector<vk2d::Color8>		&	image_data,
	vk2d::Vector2i							hot_spot
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		image_size,
		image_data,
		hot_spot
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

VK2D_API vk2d::Cursor::Cursor(
	vk2d::Cursor	&	other
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		other.impl->GetSize(),
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

VK2D_API vk2d::Cursor::~Cursor()
{}

VK2D_API vk2d::Cursor & VK2D_APIENTRY vk2d::Cursor::operator=(
	vk2d::Cursor	&	other )
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		other.impl->GetSize(),
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()
		);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}

	return *this;
}

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::Cursor::GetSize()
{
	if( impl ) {
		return impl->GetSize();
	}
	return {};
}

VK2D_API vk2d::Vector2i VK2D_APIENTRY vk2d::Cursor::GetHotSpot()
{
	if( impl ) {
		return impl->GetHotSpot();
	}
	return {};
}

VK2D_API std::vector<vk2d::Color8> VK2D_APIENTRY vk2d::Cursor::GetPixelData()
{
	if( impl ) {
		return impl->GetPixelData();
	}
	return {};
}







VK2D_API vk2d::Monitor::Monitor(
	std::unique_ptr<vk2d::_internal::MonitorImpl>	&&	preconstructed_impl
)
{
	impl	= std::move( preconstructed_impl );
	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		is_good		= false;
		impl		= nullptr;
	}
}

VK2D_API vk2d::Monitor::Monitor(
	const vk2d::Monitor			&	other
)
{
	impl = std::make_unique<vk2d::_internal::MonitorImpl>(
		other.impl->monitor,
		other.impl->position,
		other.impl->physical_size,
		other.impl->name,
		other.impl->current_video_mode,
		other.impl->video_modes
	);
	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		impl		= nullptr;
		is_good		= false;
	}
}

VK2D_API vk2d::MonitorVideoMode VK2D_APIENTRY vk2d::Monitor::GetCurrentVideoMode() const
{
	if( impl ) {
		return impl->GetCurrentVideoMode();
	}
	return {};
}

VK2D_API std::vector<vk2d::MonitorVideoMode>VK2D_APIENTRY vk2d::Monitor::GetVideoModes() const
{
	if( impl ) {
		return impl->GetVideoModes();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Monitor::SetGamma(
	float		gamma
)
{
	if( impl ) {
		impl->SetGamma( gamma );
	}
}

VK2D_API vk2d::GammaRamp VK2D_APIENTRY vk2d::Monitor::GetGammaRamp()
{
	if( impl ) {
		return impl->GetGammaRamp();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY vk2d::Monitor::SetGammaRamp(
	const vk2d::GammaRamp		&	ramp
)
{
	if( impl ) {
		impl->SetGammaRamp( ramp );
	}
}

VK2D_API vk2d::Monitor & VK2D_APIENTRY vk2d::Monitor::operator=(
	const vk2d::Monitor		&	other
)
{
	impl = std::make_unique<vk2d::_internal::MonitorImpl>(
		other.impl->monitor,
		other.impl->position,
		other.impl->physical_size,
		other.impl->name,
		other.impl->current_video_mode,
		other.impl->video_modes
		);
	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		impl		= nullptr;
		is_good		= false;
	}

	return *this;
}

VK2D_API bool VK2D_APIENTRY vk2d::Monitor::IsGood()
{
	return is_good;
}
