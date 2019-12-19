
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

namespace vk2d {



VK2D_API										Renderer::Renderer(
	const RendererCreateInfo				&	renderer_create_info )
{
	impl = std::make_unique<vk2d::_internal::RendererImpl>( renderer_create_info );
	if( !impl ) {
		if( renderer_create_info.report_function ) {
			renderer_create_info.report_function( ReportSeverity::CRITICAL_ERROR, "Out of host ram!" );
		}
		return;
	}
	if( !impl->IsGood() ) {
		impl	= nullptr;
		return;
	}
	is_good		= true;
}



VK2D_API Renderer::~Renderer()
{
	impl = nullptr;
}

VK2D_API std::vector<vk2d::Monitor*> VK2D_APIENTRY Renderer::GetMonitors()
{
	if( impl ) {
		return impl->GetMonitors();
	}
	return {};
}

VK2D_API vk2d::Monitor *VK2D_APIENTRY Renderer::GetPrimaryMonitor()
{
	if( impl ) {
		return impl->GetPrimaryMonitor();
	}
	return {};
}



VK2D_API void VK2D_APIENTRY Renderer::SetMonitorUpdateCallback( MonitorUpdateCallbackFun monitor_update_callback_funtion )
{
	if( impl ) {
		impl->SetMonitorUpdateCallback(
			monitor_update_callback_funtion
		);
	}
}

VK2D_API void VK2D_APIENTRY Renderer::SetGamepadEventCallback( vk2d::GamepadEventCallbackFun gamepad_event_callback_function )
{
	if( impl ) {
		impl->SetGamepadEventCallback( gamepad_event_callback_function );
	}
}

VK2D_API bool VK2D_APIENTRY Renderer::IsGamepadPresent( vk2d::Gamepad gamepad )
{
	if( impl ) {
		return impl->IsGamepadPresent( gamepad );
	}
	return {};
}

VK2D_API std::string VK2D_APIENTRY Renderer::GetGamepadName( vk2d::Gamepad gamepad )
{
	if( impl ) {
		return impl->GetGamepadName( gamepad );
	}
	return {};
}

VK2D_API vk2d::GamepadState VK2D_APIENTRY Renderer::QueryGamepadState( vk2d::Gamepad gamepad )
{
	if( impl ) {
		return impl->QueryGamepadState( gamepad );
	}
	return {};
}

VK2D_API Window * VK2D_APIENTRY Renderer::CreateOutputWindow(
	WindowCreateInfo		&	window_create_info )
{
	if( impl ) return impl->CreateOutputWindow( window_create_info );
	return {};
}



VK2D_API void VK2D_APIENTRY Renderer::CloseOutputWindow( Window * window )
{
	if( impl ) {
		impl->CloseOutputWindow( window );
	}
}

VK2D_API vk2d::Sampler * VK2D_APIENTRY Renderer::CreateSampler(
	const vk2d::SamplerCreateInfo		&	sampler_create_info
)
{
	if( impl ) {
		return impl->CreateSampler( sampler_create_info );
	}
	return {};
}

VK2D_API void VK2D_APIENTRY Renderer::DestroySampler(
	vk2d::Sampler						*	sampler
)
{
	if( impl ) {
		impl->DestroySampler( sampler );
	}
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY Renderer::GetMaximumSupportedMultisampling()
{
	if( impl ) {
		return impl->GetMaximumSupportedMultisampling();
	}
	return vk2d::Multisamples::SAMPLE_COUNT_1;
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY Renderer::GetAllSupportedMultisampling()
{
	if( impl ) {
		return impl->GetAllSupportedMultisampling();
	}
	return vk2d::Multisamples::SAMPLE_COUNT_1;
}

VK2D_API ResourceManager * VK2D_APIENTRY Renderer::GetResourceManager()
{
	if( impl ) return impl->GetResourceManager();
	return {};
}



VK2D_API std::unique_ptr<Renderer>VK2D_APIENTRY CreateRenderer( const RendererCreateInfo & renderer_create_info )
{
	auto renderer = std::unique_ptr<Renderer>( new Renderer( renderer_create_info ) );

	if( renderer->is_good ) {
		return renderer;
	}
	return {};
}



}