#pragma once

#include "Common.h"

#include "Window.h"

#include "Version.hpp"

#include <string>
#include <memory>

namespace vk2d {

namespace _internal {
struct RendererDataImpl;
} // _internal


class Window;


enum class ReportSeverity : uint32_t {
	NONE					= 0,	// Not valid severity value
	INFO					= 1,	// Useful to know what the application is doing
	PERFORMANCE_WARNING		= 2,	// Serious bottlenecks in performance somewhere, you should check it out
	WARNING					= 3,	// Failed to load a resource so something might be missing but can still continue with visual defects
	NON_CRITICAL_ERROR		= 5,	// Error that still allows the application to continue running, might not get a picture though
	CRITICAL_ERROR			= 6,	// Critical error, abandon ship, application has no option but to terminate... Immediately
};


typedef void ( *PFN_VK2D_ReportFunction )( ReportSeverity severity, std::string message );


struct RendererCreateInfo {
	std::string					application_name;
	Version						application_version;
	std::string					engine_name;
	Version						engine_version;
	PFN_VK2D_ReportFunction		report_function;
};


class Renderer {
	friend std::unique_ptr<vk2d::Renderer> CreateRenderer( const RendererCreateInfo & renderer_create_info );
	friend class Window;

private:
	// Do not use directly, instead use vk2d::CreateRender() to get a renderer.
	VK2D_API													Renderer(
		const RendererCreateInfo							&	renderer_create_info );

public:
	VK2D_API													~Renderer();

	VK2D_API Window						*	VK2D_APIENTRY		CreateWindowOutput(
		WindowCreateInfo									&	window_create_info );
	VK2D_API void							VK2D_APIENTRY		CloseWindowOutput(
		Window												*	window );

private:
	std::unique_ptr<vk2d::_internal::RendererDataImpl>			data;

	static uint64_t												renderer_count;					// used to keep track of Renderer instances

	bool														is_good					= {};
};


VK2D_API std::unique_ptr<Renderer>			VK2D_APIENTRY		CreateRenderer(
	const RendererCreateInfo								&	renderer_create_info );


}