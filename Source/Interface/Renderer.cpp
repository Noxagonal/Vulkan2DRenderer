
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Renderer.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Core/QueueResolver.h"
#include "../../Include/Interface/ResourceManager.h"
#include "../../Include/Interface/Sampler.h"



#include <stdint.h>
#include <sstream>
#include <iostream>
#include <vector>

#if VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <Windows.h>
#endif



VK2D_API										vk2d::Renderer::Renderer(
	const vk2d::RendererCreateInfo			&	renderer_create_info
)
{
	impl = std::make_unique<vk2d::_internal::RendererImpl>( renderer_create_info );
	if( impl && impl->IsGood() ) {
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		if( renderer_create_info.report_function ) {
			renderer_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot create renderer implementation!" );
		}
		return;
	}
}



VK2D_API vk2d::Renderer::~Renderer()
{
	impl = nullptr;
}

VK2D_API std::vector<vk2d::Monitor*> VK2D_APIENTRY vk2d::Renderer::GetMonitors()
{
	return impl->GetMonitors();
}

VK2D_API vk2d::Monitor *VK2D_APIENTRY vk2d::Renderer::GetPrimaryMonitor()
{
	return impl->GetPrimaryMonitor();
}



VK2D_API void VK2D_APIENTRY vk2d::Renderer::SetMonitorUpdateCallback(
	vk2d::MonitorUpdateCallbackFun			monitor_update_callback_funtion
)
{
	impl->SetMonitorUpdateCallback(
		monitor_update_callback_funtion
	);
}

VK2D_API vk2d::Cursor * VK2D_APIENTRY vk2d::Renderer::CreateCursor(
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	return impl->CreateCursor(
		image_path,
		hot_spot
	);
}

VK2D_API vk2d::Cursor * VK2D_APIENTRY vk2d::Renderer::CreateCursor(
	vk2d::Vector2u							image_size,
	const std::vector<vk2d::Color8>		&	image_data,
	vk2d::Vector2i							hot_spot
)
{
	return impl->CreateCursor(
		image_size,
		image_data,
		hot_spot
	);
}

VK2D_API void VK2D_APIENTRY vk2d::Renderer::DestroyCursor(
	vk2d::Cursor						*	cursor )
{
	impl->DestroyCursor( cursor );
}

VK2D_API void VK2D_APIENTRY vk2d::Renderer::SetGamepadEventCallback(
	vk2d::GamepadEventCallbackFun		gamepad_event_callback_function
)
{
	impl->SetGamepadEventCallback( gamepad_event_callback_function );
}

VK2D_API bool VK2D_APIENTRY vk2d::Renderer::IsGamepadPresent(
	vk2d::Gamepad			gamepad
)
{
	return impl->IsGamepadPresent( gamepad );
}

VK2D_API std::string VK2D_APIENTRY vk2d::Renderer::GetGamepadName(
	vk2d::Gamepad		gamepad
)
{
	return impl->GetGamepadName( gamepad );
}

VK2D_API vk2d::GamepadState VK2D_APIENTRY vk2d::Renderer::QueryGamepadState(
	vk2d::Gamepad		gamepad
)
{
	return impl->QueryGamepadState( gamepad );
}

VK2D_API vk2d::Window * VK2D_APIENTRY vk2d::Renderer::CreateOutputWindow(
	WindowCreateInfo		&	window_create_info
)
{
	return impl->CreateOutputWindow( window_create_info );
}



VK2D_API void VK2D_APIENTRY vk2d::Renderer::CloseOutputWindow(
	vk2d::Window		*	window
)
{
	impl->CloseOutputWindow( window );
}

VK2D_API vk2d::Sampler * VK2D_APIENTRY vk2d::Renderer::CreateSampler(
	const vk2d::SamplerCreateInfo		&	sampler_create_info
)
{
	return impl->CreateSampler( sampler_create_info );
}

VK2D_API void VK2D_APIENTRY vk2d::Renderer::DestroySampler(
	vk2d::Sampler						*	sampler
)
{
	impl->DestroySampler( sampler );
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY vk2d::Renderer::GetMaximumSupportedMultisampling()
{
	return impl->GetMaximumSupportedMultisampling();
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY vk2d::Renderer::GetAllSupportedMultisampling()
{
	return impl->GetAllSupportedMultisampling();
}

VK2D_API vk2d::ResourceManager * VK2D_APIENTRY vk2d::Renderer::GetResourceManager()
{
	return impl->GetResourceManager();
}



VK2D_API std::unique_ptr<vk2d::Renderer>VK2D_APIENTRY vk2d::CreateRenderer(
	const vk2d::RendererCreateInfo		&	renderer_create_info
)
{
	auto renderer = std::unique_ptr<vk2d::Renderer>( new vk2d::Renderer( renderer_create_info ) );

	if( renderer->is_good ) {
		return renderer;
	}
	return {};
}
