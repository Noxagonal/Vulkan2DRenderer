
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Instance.h"
#include "../Header/Impl/InstanceImpl.h"
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



VK2D_API										vk2d::Instance::Instance(
	const vk2d::InstanceCreateInfo			&	instance_create_info
)
{
	impl = std::make_unique<vk2d::_internal::InstanceImpl>( instance_create_info );
	if( impl && impl->IsGood() ) {
		is_good	= true;
	} else {
		is_good	= false;
		impl	= nullptr;
		if( instance_create_info.report_function ) {
			instance_create_info.report_function( vk2d::ReportSeverity::CRITICAL_ERROR, "Cannot create instance implementation!" );
		}
		return;
	}
}



VK2D_API vk2d::Instance::~Instance()
{
	impl = nullptr;
}

VK2D_API std::vector<vk2d::Monitor*> VK2D_APIENTRY vk2d::Instance::GetMonitors()
{
	return impl->GetMonitors();
}

VK2D_API vk2d::Monitor *VK2D_APIENTRY vk2d::Instance::GetPrimaryMonitor()
{
	return impl->GetPrimaryMonitor();
}



VK2D_API void VK2D_APIENTRY vk2d::Instance::SetMonitorUpdateCallback(
	vk2d::MonitorUpdateCallbackFun			monitor_update_callback_funtion
)
{
	impl->SetMonitorUpdateCallback(
		monitor_update_callback_funtion
	);
}

VK2D_API vk2d::Cursor * VK2D_APIENTRY vk2d::Instance::CreateCursor(
	const std::filesystem::path			&	image_path,
	vk2d::Vector2i							hot_spot
)
{
	return impl->CreateCursor(
		image_path,
		hot_spot
	);
}

VK2D_API vk2d::Cursor * VK2D_APIENTRY vk2d::Instance::CreateCursor(
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

VK2D_API void VK2D_APIENTRY vk2d::Instance::DestroyCursor(
	vk2d::Cursor						*	cursor )
{
	impl->DestroyCursor( cursor );
}

VK2D_API void VK2D_APIENTRY vk2d::Instance::SetGamepadEventCallback(
	vk2d::GamepadEventCallbackFun		gamepad_event_callback_function
)
{
	impl->SetGamepadEventCallback( gamepad_event_callback_function );
}

VK2D_API bool VK2D_APIENTRY vk2d::Instance::IsGamepadPresent(
	vk2d::Gamepad			gamepad
)
{
	return impl->IsGamepadPresent( gamepad );
}

VK2D_API std::string VK2D_APIENTRY vk2d::Instance::GetGamepadName(
	vk2d::Gamepad		gamepad
)
{
	return impl->GetGamepadName( gamepad );
}

VK2D_API vk2d::GamepadState VK2D_APIENTRY vk2d::Instance::QueryGamepadState(
	vk2d::Gamepad		gamepad
)
{
	return impl->QueryGamepadState( gamepad );
}

VK2D_API vk2d::Window * VK2D_APIENTRY vk2d::Instance::CreateOutputWindow(
	const WindowCreateInfo		&	window_create_info
)
{
	return impl->CreateOutputWindow( window_create_info );
}



VK2D_API void VK2D_APIENTRY vk2d::Instance::DestroyOutputWindow(
	vk2d::Window				*	window
)
{
	impl->DestroyOutputWindow( window );
}

VK2D_API vk2d::RenderTargetTexture * VK2D_APIENTRY vk2d::Instance::CreateRenderTargetTexture(
	const vk2d::RenderTargetTextureCreateInfo	&	render_target_texture_create_info
)
{
	return impl->CreateRenderTargetTexture( render_target_texture_create_info );
}

VK2D_API void VK2D_APIENTRY vk2d::Instance::DestroyRenderTargetTexture(
	vk2d::RenderTargetTexture					*	render_target_texture
)
{
	impl->DestroyRenderTargetTexture( render_target_texture );
}

VK2D_API vk2d::Sampler * VK2D_APIENTRY vk2d::Instance::CreateSampler(
	const vk2d::SamplerCreateInfo		&	sampler_create_info
)
{
	return impl->CreateSampler( sampler_create_info );
}

VK2D_API void VK2D_APIENTRY vk2d::Instance::DestroySampler(
	vk2d::Sampler						*	sampler
)
{
	impl->DestroySampler( sampler );
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY vk2d::Instance::GetMaximumSupportedMultisampling()
{
	return impl->GetMaximumSupportedMultisampling();
}

VK2D_API vk2d::Multisamples VK2D_APIENTRY vk2d::Instance::GetAllSupportedMultisampling()
{
	return impl->GetAllSupportedMultisampling();
}

VK2D_API vk2d::ResourceManager * VK2D_APIENTRY vk2d::Instance::GetResourceManager()
{
	return impl->GetResourceManager();
}



VK2D_API std::unique_ptr<vk2d::Instance>VK2D_APIENTRY vk2d::CreateInstance(
	const vk2d::InstanceCreateInfo		&	instance_create_info
)
{
	auto instance = std::unique_ptr<vk2d::Instance>( new vk2d::Instance( instance_create_info ) );

	if( instance->is_good ) {
		return instance;
	}
	return {};
}
