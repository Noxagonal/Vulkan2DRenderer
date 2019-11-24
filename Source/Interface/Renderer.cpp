
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Renderer.h"
#include "../Header/Impl/RendererImpl.h"
#include "../Header/Impl/WindowImpl.h"
#include "../../Include/Interface/Window.h"
#include "../Header/Core/QueueResolver.h"
#include "../../Include/Interface/ResourceManager.h"



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



VK2D_API Window * VK2D_APIENTRY Renderer::CreateWindowOutput(
	WindowCreateInfo		&	window_create_info )
{
	if( impl ) return impl->CreateWindowOutput( window_create_info );
	return {};
}



VK2D_API void VK2D_APIENTRY Renderer::CloseWindowOutput( Window * window )
{
	if( impl ) {
		impl->CloseWindowOutput( window );
	}
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