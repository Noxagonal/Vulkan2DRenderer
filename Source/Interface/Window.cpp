
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Window.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Instance.h"
#include "../Header/Impl/InstanceImpl.h"
#include "../Header/Core/MeshBuffer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <array>



VK2D_API					vk2d::Window::Window(
	vk2d::_internal::InstanceImpl	*	instance_parent,
	const vk2d::WindowCreateInfo	&	window_create_info
)
{
	impl	= std::make_unique<vk2d::_internal::WindowImpl>(
		this,
		instance_parent,
		window_create_info
		);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		instance_parent->Report( vk2d::ReportSeverity::CRITICAL_ERROR, "Internal error: Cannot create window implementation!" );
		is_good			= false;
	}
}



VK2D_API vk2d::Window::~Window()
{
	impl	= nullptr;
}

VK2D_API void VK2D_APIENTRY vk2d::Window::CloseWindow()
{
	impl->CloseWindow();
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::ShouldClose()
{
	return impl->ShouldClose();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::UpdateEvents()
{
	impl->UpdateEvents();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::TakeScreenshotToFile(
	const std::filesystem::path		&	save_path,
	bool								include_alpha
)
{
	impl->TakeScreenshotToFile(
		save_path,
		include_alpha
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::TakeScreenshotToData(
	bool		include_alpha
)
{
	impl->TakeScreenshotToData( include_alpha );
}

VK2D_API void VK2D_APIENTRY vk2d::Window::Focus()
{
	impl->Focus();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetOpacity(
	float			opacity
)
{
	impl->SetOpacity( opacity );
}

VK2D_API float VK2D_APIENTRY vk2d::Window::GetOpacity()
{
	return impl->GetOpacity();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::Hide(
	bool			hidden
)
{
	impl->Hide( hidden );
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsHidden()
{
	return impl->IsHidden();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DisableEvents(
	bool				disable_events
)
{
	impl->DisableEvents( disable_events );
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::AreEventsDisabled()
{
	return impl->AreEventsDisabled();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetFullscreen(
	vk2d::Monitor		*	monitor,
	uint32_t				frequency )
{
	impl->SetFullscreen( monitor, frequency );
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsFullscreen()
{
	return impl->IsFullscreen();
}

VK2D_API vk2d::Vector2d VK2D_APIENTRY vk2d::Window::GetCursorPosition()
{
	return impl->GetCursorPosition();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursorPosition(
	vk2d::Vector2d			new_position
)
{
	impl->SetCursorPosition( new_position );
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursor(
	vk2d::Cursor		*	cursor
)
{
	impl->SetCursor( cursor );
}

VK2D_API std::string VK2D_APIENTRY vk2d::Window::GetClipboardString()
{
	return impl->GetClipboardString();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetClipboardString( const std::string & str )
{
	impl->SetClipboardString( str );
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetTitle( const std::string & title )
{
	impl->SetTitle( title );
}

VK2D_API std::string VK2D_APIENTRY vk2d::Window::GetTitle()
{
	return impl->GetTitle();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetIcon(
	const std::vector<std::filesystem::path>		&	image_paths )
{
	impl->SetIcon( image_paths );
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetPosition(
	vk2d::Vector2i			new_position
)
{
	impl->SetPosition( new_position );
}

VK2D_API vk2d::Vector2i VK2D_APIENTRY vk2d::Window::GetPosition()
{
	return impl->GetPosition();
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
	impl->Iconify( iconified );
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::IsIconified()
{
	return impl->IsIconified();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetMaximized(
	bool		maximized
)
{
	impl->SetMaximized( maximized );
}

VK2D_API bool VK2D_APIENTRY vk2d::Window::GetMaximized()
{
	return impl->GetMaximized();
}

VK2D_API void VK2D_APIENTRY vk2d::Window::SetCursorState(
	vk2d::CursorState		new_state
)
{
	impl->SetCursorState( new_state );
}

VK2D_API vk2d::CursorState VK2D_APIENTRY vk2d::Window::GetCursorState()
{
	return impl->GetCursorState();
}







VK2D_API bool VK2D_APIENTRY vk2d::Window::BeginRender()
{
	return impl->BeginRender();
}



VK2D_API bool VK2D_APIENTRY vk2d::Window::EndRender()
{
	return impl->EndRender();
}



VK2D_API void VK2D_APIENTRY vk2d::Window::DrawTriangleList(
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	bool										filled,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawTriangleList(
		indices,
		vertices,
		texture_channels,
		filled,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawLineList(
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawLineList(
		indices,
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::TextureResource					*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawPointList(
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPoint(
	vk2d::Vector2f			location,
	vk2d::Colorf			color,
	float					size
)
{
	impl->DrawPoint(
		location,
		color,
		size
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawLine(
	vk2d::Vector2f					point_1,
	vk2d::Vector2f					point_2,
	vk2d::Colorf					color
)
{
	impl->DrawLine(
		point_1,
		point_2,
		color
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	vk2d::Colorf					color
)
{
	impl->DrawBox(
		top_left,
		bottom_right,
		filled,
		color
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawCircle(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	impl->DrawCircle(
		top_left,
		bottom_right,
		filled,
		edge_count,
		color
	);
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

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawPieBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	vk2d::Colorf					color
)
{
	impl->DrawPieBox(
		top_left,
		bottom_right,
		begin_angle_radians,
		coverage,
		filled,
		color
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawTexture(
	vk2d::Vector2f				top_left,
	vk2d::Vector2f				bottom_right,
	vk2d::TextureResource	*	texture,
	vk2d::Colorf				color
)
{
	impl->DrawTexture(
		top_left,
		bottom_right,
		texture,
		color
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Window::DrawMesh(
	const vk2d::Mesh	&	mesh
)
{
	impl->DrawMesh(
		mesh
	);
}







VK2D_API vk2d::Cursor::Cursor(
	vk2d::_internal::InstanceImpl		*	instance,
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		instance,
		image_path,
		hot_spot
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		is_good			= false;
		impl			= nullptr;
	}
}

VK2D_API vk2d::Cursor::Cursor(
	vk2d::_internal::InstanceImpl		*	instance,
	vk2d::Vector2u							image_size,
	const std::vector<vk2d::Color8>		&	image_data,
	vk2d::Vector2i							hot_spot
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		instance,
		image_size,
		image_data,
		hot_spot
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		is_good			= false;
		impl			= nullptr;
	}
}

VK2D_API vk2d::Cursor::Cursor(
	vk2d::Cursor	&	other
)
{
	impl		= std::make_unique<vk2d::_internal::CursorImpl>(
		other.impl->GetInstance(),
		other.impl->GetSize(),
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()
	);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		other.impl->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
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
		other.impl->GetInstance(),
		other.impl->GetSize(),
		other.impl->GetPixelData(),
		other.impl->GetHotSpot()
		);
	if( impl && impl->IsGood() ) {
		is_good			= true;
	} else {
		other.impl->GetInstance()->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		is_good			= false;
		impl			= nullptr;
	}

	return *this;
}

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::Cursor::GetSize()
{
	return impl->GetSize();
}

VK2D_API vk2d::Vector2i VK2D_APIENTRY vk2d::Cursor::GetHotSpot()
{
	return impl->GetHotSpot();
}

VK2D_API std::vector<vk2d::Color8> VK2D_APIENTRY vk2d::Cursor::GetPixelData()
{
	return impl->GetPixelData();
}

VK2D_API bool VK2D_APIENTRY vk2d::Cursor::IsGood()
{
	return is_good;
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
	return impl->GetCurrentVideoMode();
}

VK2D_API std::vector<vk2d::MonitorVideoMode>VK2D_APIENTRY vk2d::Monitor::GetVideoModes() const
{
	return impl->GetVideoModes();
}

VK2D_API void VK2D_APIENTRY vk2d::Monitor::SetGamma(
	float		gamma
)
{
	impl->SetGamma( gamma );
}

VK2D_API vk2d::GammaRamp VK2D_APIENTRY vk2d::Monitor::GetGammaRamp()
{
	return impl->GetGammaRamp();
}

VK2D_API void VK2D_APIENTRY vk2d::Monitor::SetGammaRamp(
	const vk2d::GammaRamp		&	ramp
)
{
	impl->SetGammaRamp( ramp );
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
		is_good		= false;
		impl		= nullptr;
	}

	return *this;
}

VK2D_API bool VK2D_APIENTRY vk2d::Monitor::IsGood()
{
	return is_good;
}
