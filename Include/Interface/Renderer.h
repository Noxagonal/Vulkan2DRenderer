#pragma once

#include "../Core/Common.h"

#include "Window.h"
#include "../Core/Version.hpp"
#include "Sampler.h"

#include <string>
#include <memory>
#include <inttypes.h>

namespace vk2d {

namespace _internal {
class RendererImpl;
class MonitorImpl;
} // _internal


class Window;
class Monitor;
class ResourceManager;









enum class ReportSeverity : uint32_t {
	NONE					= 0,	// Not valid severity value
	INFO					= 1,	// Useful to know what the application is doing
	PERFORMANCE_WARNING		= 2,	// Serious bottlenecks in performance somewhere, you should check it out
	WARNING					= 3,	// Failed to load a resource so something might be missing but can still continue with visual defects
	NON_CRITICAL_ERROR		= 5,	// Error that still allows the application to continue running, might not get a picture though
	CRITICAL_ERROR			= 6,	// Critical error, abandon ship, application has no option but to terminate... Immediately
};





enum class Gamepad : int32_t {
	GAMEPAD_1			= 0,
	GAMEPAD_2			= 1,
	GAMEPAD_3			= 2,
	GAMEPAD_4			= 3,
	GAMEPAD_5			= 4,
	GAMEPAD_6			= 5,
	GAMEPAD_7			= 6,
	GAMEPAD_8			= 7,
	GAMEPAD_9			= 8,
	GAMEPAD_10			= 9,
	GAMEPAD_11			= 10,
	GAMEPAD_12			= 11,
	GAMEPAD_13			= 12,
	GAMEPAD_14			= 13,
	GAMEPAD_15			= 14,
	GAMEPAD_16			= 15,
	GAMEPAD_LAST		= GAMEPAD_16,
};

enum class GamepadEvent : int32_t {
	CONNECTED			= 0x00040001,
	DISCONNECTED		= 0x00040002,
};

enum class GamepadButtons : int32_t {
	A				= 0,
	B				= 1,
	X				= 2,
	Y				= 3,
	LEFT_BUMPER		= 4,
	RIGHT_BUMPER	= 5,
	BACK			= 6,
	START			= 7,
	GUIDE			= 8,
	LEFT_THUMB		= 9,
	RIGHT_THUMB		= 10,
	DPAD_UP			= 11,
	DPAD_RIGHT		= 12,
	DPAD_DOWN		= 13,
	DPAD_LEFT		= 14,
	LAST			= DPAD_LEFT,

	CROSS			= A,
	CIRCLE			= B,
	SQUARE			= X,
	TRIANGLE		= Y,
};

enum class GamepadAxis : int32_t {
	LEFT_X				= 0,
	LEFT_Y				= 1,
	RIGHT_X				= 2,
	RIGHT_Y				= 3,
	LEFT_TRIGGER		= 4,
	RIGHT_TRIGGER		= 5,
	LAST				= RIGHT_TRIGGER
};


class GamepadState {
public:
	GamepadState()		= default;
	~GamepadState()		= default;

	inline bool GetButton( vk2d::GamepadButtons gamepad_button )
	{
		assert( int32_t( gamepad_button ) <= int32_t( vk2d::GamepadButtons::LAST ) );
		return buttons[ int32_t( gamepad_button ) ];
	}
	inline float GetAxis( vk2d::GamepadAxis gamepad_axis )
	{
		assert( int32_t( gamepad_axis ) <= int32_t( vk2d::GamepadAxis::LAST ) );
		return axes[ int32_t( gamepad_axis ) ];
	}

	std::array<bool, 15>	buttons;
	std::array<float, 6>	axes;
};







typedef void ( VK2D_APIENTRY *PFN_VK2D_ReportFunction )(
	ReportSeverity					severity,
	std::string						message );

typedef void ( VK2D_APIENTRY *MonitorUpdateCallbackFun )(
	void );

typedef void ( VK2D_APIENTRY *GamepadEventCallbackFun )(
	vk2d::Gamepad					joystick,
	vk2d::GamepadEvent				event,
	const std::string			&	joystickName );


struct RendererCreateInfo {
	std::string					application_name				= {};
	Version						application_version				= {};
	std::string					engine_name						= {};
	Version						engine_version					= {};
	PFN_VK2D_ReportFunction		report_function					= {};
	uint32_t					resource_loader_thread_count	= UINT32_MAX;
};


class Renderer {
	friend VK2D_API std::unique_ptr<vk2d::Renderer> VK2D_APIENTRY CreateRenderer( const RendererCreateInfo & renderer_create_info );
	friend class Window;
	 
private:
	// Do not use directly, instead use vk2d::CreateRender() to get a renderer.
	VK2D_API																			Renderer(
		const RendererCreateInfo													&	renderer_create_info );

public:
	VK2D_API																			~Renderer();

	// Get all monitors currently attached to the system.
	// Also see vk2d::Renderer::SetMonitorUpdateCallback().
	// Returns:
	// Vector of pointers to Monitor objects.
	VK2D_API std::vector<vk2d::Monitor*>			VK2D_APIENTRY						GetMonitors();

	// Get the primary monitor of the system.
	// Also see SetMonitorUpdateCallback().
	// Returns:
	// Pointer to Monitor object.
	VK2D_API vk2d::Monitor						*	VK2D_APIENTRY						GetPrimaryMonitor();

	// Set monitor update callback, this is to notify your application that some of the
	// monitors got removed or new monitors were plugged into the system. If you use
	// different monitors all the time, you should call GetPrimaryMonitor() and GetMonitors()
	// immediately to get the updated monitors and to avoid possible crashes.
	// Parameters:
	// [in] monitor_update_callback_function: Function that gets called if monitor was removed or added to the system.
	VK2D_API void									VK2D_APIENTRY						SetMonitorUpdateCallback(
		MonitorUpdateCallbackFun					monitor_update_callback_funtion );

	// Set gamepad event callback function, the callback function gets
	// called if a gamepad gets added or removed from the system.
	// Parameters:
	// [in] gamepad_event_callback_function: Function that gets called if a gamepad was removed or added to the system.
	VK2D_API void									VK2D_APIENTRY						SetGamepadEventCallback(
		vk2d::GamepadEventCallbackFun				gamepad_event_callback_function );

	// Checks if a specific gamepad is currently attached to the system.
	// Parameters:
	// [in] gamepad: Specific gamepad to check if it's present.
	// Returns:
	// true if gamepad is connected to the system, false if not.
	VK2D_API bool									VK2D_APIENTRY						IsGamepadPresent(
		vk2d::Gamepad								gamepad );

	// Checks a name of a gamepad if it's attached to the system.
	// Parameters:
	// [in] gamepad: Specific gamepad to check the name for.
	// Returns:
	// Name of the specific gamepad. Name might not be unique.
	VK2D_API std::string							VK2D_APIENTRY						GetGamepadName(
		vk2d::Gamepad								gamepad );

	// Gets the button presses and axis of the gamepad.
	// Parameters:
	// [in] gamepad: Specific gamepad to check the state for.
	// Returns:
	// GamepadState object which tells which buttons were pressed and state of the axis.
	VK2D_API vk2d::GamepadState						VK2D_APIENTRY						QueryGamepadState(
		vk2d::Gamepad								gamepad );
	 
	// TODO: gamepad mapping
//	VK2D_API void									VK2D_APIENTRY						SetGamepadMapping();


	VK2D_API Window								*	VK2D_APIENTRY						CreateOutputWindow(
		WindowCreateInfo															&	window_create_info );
	VK2D_API void									VK2D_APIENTRY						CloseOutputWindow(
		Window																		*	window );

	VK2D_API vk2d::Sampler						*	VK2D_APIENTRY						CreateSampler(
		const vk2d::SamplerCreateInfo			&	sampler_create_info );

	VK2D_API void									VK2D_APIENTRY						DestroySampler(
		vk2d::Sampler							*	sampler );

	VK2D_API ResourceManager					*	VK2D_APIENTRY						GetResourceManager();

private:
	std::unique_ptr<vk2d::_internal::RendererImpl>	impl;

	bool											is_good					= {};
};


VK2D_API std::unique_ptr<Renderer>					VK2D_APIENTRY						CreateRenderer(
	const RendererCreateInfo					&	renderer_create_info );


}