
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
		this,
		renderer_parent,
		window_create_info
		);
	if( !impl ) {
		if( renderer_parent->GetReportFunction() ) {
			renderer_parent->GetReportFunction()( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return;
	}
	if( !impl->is_good ) {
		impl = nullptr;
		return;
	}
	is_good							= true;
}



VK2D_API Window::~Window()
{
	impl	= nullptr;
}

VK2D_API void VK2D_APIENTRY Window::CloseWindow()
{
	if( impl ) {
		impl->CloseWindow();
	}
}

VK2D_API bool VK2D_APIENTRY Window::ShouldClose()
{
	if( impl ) {
		return impl->ShouldClose();
	}
	return true;
}

VK2D_API void VK2D_APIENTRY Window::UpdateEvents()
{
	if( impl ) {
		impl->UpdateEvents();
	}
}

VK2D_API void VK2D_APIENTRY Window::TakeScreenshot(
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

VK2D_API void VK2D_APIENTRY Window::Focus()
{
	if( impl ) {
		impl->Focus();
	}
}

VK2D_API void VK2D_APIENTRY Window::SetOpacity(
	float			opacity
)
{
	if( impl ) {
		impl->SetOpacity( opacity );
	}
}

VK2D_API float VK2D_APIENTRY Window::GetOpacity()
{
	if( impl ) {
		return impl->GetOpacity();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::Hide(
	bool			hidden
)
{
	if( impl ) {
		impl->Hide( hidden );
	}
}

VK2D_API bool VK2D_APIENTRY Window::IsHidden()
{
	if( impl ) {
		return impl->IsHidden();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::DisableEvents(
	bool				disable_events
)
{
	if( impl ) {
		impl->DisableEvents( disable_events );
	}
}

VK2D_API bool VK2D_APIENTRY Window::AreEventsDisabled()
{
	if( impl ) {
		return impl->AreEventsDisabled();
	}
	return true;
}

VK2D_API void VK2D_APIENTRY Window::SetFullscreen(
	vk2d::Monitor		*	monitor,
	uint32_t				frequency )
{
	if( impl ) {
		impl->SetFullscreen( monitor, frequency );
	}
}

VK2D_API bool VK2D_APIENTRY Window::IsFullscreen()
{
	if( impl ) {
		return impl->IsFullscreen();
	}
	return {};
}

VK2D_API std::array<double, 2>VK2D_APIENTRY Window::GetCursorPosition()
{
	if( impl ) {
		return impl->GetCursorPosition();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetCursorPosition(
	double			x,
	double			y
)
{
	if( impl ) {
		impl->SetCursorPosition( x, y );
	}
}

VK2D_API void VK2D_APIENTRY Window::SetCursor(
	vk2d::Cursor		*	cursor
)
{
	if( impl ) {
		impl->SetCursor( cursor );
	}
}

VK2D_API std::string VK2D_APIENTRY Window::GetClipboardString()
{
	if( impl ) {
		return impl->GetClipboardString();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetClipboardString( const std::string & str )
{
	if( impl ) {
		impl->SetClipboardString( str );
	}
}

VK2D_API void VK2D_APIENTRY Window::SetTitle( const std::string & title )
{
	if( impl ) {
		impl->SetTitle( title );
	}
}

VK2D_API std::string VK2D_APIENTRY Window::GetTitle()
{
	if( impl ) {
		return impl->GetTitle();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetIcon(
	const std::vector<std::filesystem::path>		&	image_paths )
{
	if( impl ) {
		impl->SetIcon( image_paths );
	}
}

VK2D_API void VK2D_APIENTRY Window::SetPosition(
	int32_t				x,
	int32_t				y
)
{
	if( impl ) {
		impl->SetPosition( x, y );
	}
}

VK2D_API std::array<int32_t, 2>VK2D_APIENTRY Window::GetPosition()
{
	if( impl ) {
		return impl->GetPosition();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetSize( vk2d::Vector2du new_size )
{
	impl->SetSize( new_size );
}

VK2D_API vk2d::Vector2du VK2D_APIENTRY Window::GetSize()
{
	return impl->GetSize();
}

VK2D_API void VK2D_APIENTRY Window::Iconify( bool iconified )
{
	if( impl ) {
		impl->Iconify( iconified );
	}
}

VK2D_API bool VK2D_APIENTRY Window::IsIconified()
{
	if( impl ) {
		return impl->IsIconified();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetMaximized( bool maximized )
{
	if( impl ) {
		impl->SetMaximized( maximized );
	}
}

VK2D_API bool VK2D_APIENTRY Window::GetMaximized()
{
	if( impl ) {
		return impl->GetMaximized();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Window::SetCursorState( vk2d::CursorState new_state )
{
	if( impl ) {
		impl->SetCursorState( new_state );
	}
}

VK2D_API vk2d::CursorState VK2D_APIENTRY Window::GetCursorState()
{
	if( impl ) {
		return impl->GetCursorState();
	}
	return {};
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



VK2D_API void VK2D_APIENTRY Window::DrawTriangleList(
	const std::vector<Vertex>				&	vertices,
	const std::vector<VertexIndex_3>		&	indices,
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

VK2D_API void VK2D_APIENTRY Window::DrawLineList(
	const std::vector<Vertex>				&	vertices,
	const std::vector<VertexIndex_2>		&	indices,
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

VK2D_API void VK2D_APIENTRY Window::DrawPointList(
	const std::vector<Vertex>				&	vertices,
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

VK2D_API void VK2D_APIENTRY Window::DrawLine(
	Vector2d						point_1,
	Vector2d						point_2,
	Color							color
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

VK2D_API void VK2D_APIENTRY Window::DrawBox(
	Vector2d						top_left,
	Vector2d						bottom_right,
	bool							filled,
	Color							color
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

VK2D_API void VK2D_APIENTRY Window::DrawCircle(
	Vector2d						top_left,
	Vector2d						bottom_right,
	bool							filled,
	float							edge_count,
	Color							color
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

VK2D_API void VK2D_APIENTRY Window::DrawPie(
	Vector2d						top_left,
	Vector2d						bottom_right,
	float							begin_angle_radians,
	float							end_angle_radians,
	bool							filled,
	float							edge_count,
	Color							color
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

VK2D_API void VK2D_APIENTRY Window::DrawPieBox(
	Vector2d						top_left,
	Vector2d						bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	Color							color
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

VK2D_API void VK2D_APIENTRY Window::DrawTexture(
	Vector2d					top_left,
	Vector2d					bottom_right,
	vk2d::TextureResource	*	texture,
	Color						color
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

VK2D_API void VK2D_APIENTRY Window::DrawMesh(
	const vk2d::Mesh	&	mesh
)
{
	if( impl ) {
		impl->DrawMesh(
			mesh
		);
	}
}







Cursor::Cursor(
	const std::filesystem::path		&	image_path,
	int32_t								hot_spot_x,
	int32_t								hot_spot_y
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		image_path,
		hot_spot_x,
		hot_spot_y
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

Cursor::Cursor(
	uint32_t							image_size_x,
	uint32_t							image_size_y,
	const std::vector<vk2d::Color>	&	image_data,
	int32_t								hot_spot_x,
	int32_t								hot_spot_y
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		image_size_x,
		image_size_y,
		image_data,
		hot_spot_x,
		hot_spot_y
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

Cursor::Cursor(
	vk2d::Cursor	&	other
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		other.impl->GetSize()[ 0 ],
		other.impl->GetSize()[ 1 ],
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()[ 0 ],
		other.impl->GetHotSpot()[ 1 ]
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}

Cursor::~Cursor()
{}

VK2D_API vk2d::Cursor & VK2D_APIENTRY Cursor::operator=(
	vk2d::Cursor	&	other )
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		other.impl->GetSize()[ 0 ],
		other.impl->GetSize()[ 1 ],
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()[ 0 ],
		other.impl->GetHotSpot()[ 1 ]
		);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}

	return *this;
}

VK2D_API std::array<uint32_t, 2> VK2D_APIENTRY Cursor::GetSize()
{
	if( impl ) {
		return impl->GetSize();
	}
	return {};
}

VK2D_API std::array<int32_t, 2> VK2D_APIENTRY Cursor::GetHotSpot()
{
	if( impl ) {
		return impl->GetHotSpot();
	}
	return {};
}

VK2D_API std::vector<vk2d::Color> VK2D_APIENTRY Cursor::GetPixelData()
{
	if( impl ) {
		return impl->GetPixelData();
	}
	return {};
}







Monitor::Monitor(
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

vk2d::Monitor::Monitor(
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

VK2D_API vk2d::MonitorVideoMode VK2D_APIENTRY Monitor::GetCurrentVideoMode() const
{
	if( impl ) {
		return impl->GetCurrentVideoMode();
	}
	return {};
}

VK2D_API std::vector<vk2d::MonitorVideoMode>VK2D_APIENTRY Monitor::GetVideoModes() const
{
	if( impl ) {
		return impl->GetVideoModes();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Monitor::SetGamma(
	float		gamma
)
{
	if( impl ) {
		impl->SetGamma( gamma );
	}
}

VK2D_API vk2d::GammaRamp VK2D_APIENTRY Monitor::GetGammaRamp()
{
	if( impl ) {
		return impl->GetGammaRamp();
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Monitor::SetGammaRamp(
	const vk2d::GammaRamp		&	ramp
)
{
	if( impl ) {
		impl->SetGammaRamp( ramp );
	}
}

VK2D_API vk2d::Monitor & VK2D_APIENTRY Monitor::operator=(
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

} // vk2d
