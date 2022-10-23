#pragma once

#include "core/Common.h"
#include "core/Version.hpp"

#include "types/Color.hpp"

#include "interface/Window.h"
#include "interface/RenderTargetTexture.h"
#include "interface/Sampler.h"

#include <string>
#include <memory>
#include <inttypes.h>

namespace vk2d {

namespace vk2d_internal {
class InstanceImpl;
} // vk2d_internal


class ResourceManager;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Nature and severity of the reported action.
/// 
/// @see		PFN_VK2D_ReportFunction()
enum class ReportSeverity : uint32_t {

	/// @brief		Not valid severity value, used to detect invalid severity values.
	NONE					= 0,

	/// @brief		Reports everything, usually too much information.
	VERBOSE,

	/// @brief		Useful to know what the application is doing.
	INFO,

	/// @brief		Serious bottlenecks in performance somewhere, you should check it out.
	PERFORMANCE_WARNING,

	/// @brief		Failed to load a resource so something might be missing but can still continue with visual defects.
	WARNING,

	/// @brief		Error that still allows the application to continue running, might not get a picture though.
	NON_CRITICAL_ERROR,

	/// @brief		Critical error, application has no option but to terminate immediately.
	CRITICAL_ERROR,

	/// @brief		 Similar to critical error, this means the GPU crashed and we need to terminate immediately.
	DEVICE_LOST,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Enumerator to indicate specific gamepad.
enum class Gamepad : int32_t {
	GAMEPAD_1				= 0,
	GAMEPAD_2				= 1,
	GAMEPAD_3				= 2,
	GAMEPAD_4				= 3,
	GAMEPAD_5				= 4,
	GAMEPAD_6				= 5,
	GAMEPAD_7				= 6,
	GAMEPAD_8				= 7,
	GAMEPAD_9				= 8,
	GAMEPAD_10				= 9,
	GAMEPAD_11				= 10,
	GAMEPAD_12				= 11,
	GAMEPAD_13				= 12,
	GAMEPAD_14				= 13,
	GAMEPAD_15				= 14,
	GAMEPAD_16				= 15,
	GAMEPAD_LAST			= GAMEPAD_16,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Gamepad connection events.
///
///				This lets you know if a gamepad was connected to the system or disconnected.
///
/// @see		Instance::QueryGamepadState()
/// @see		Instance::GetGamepadEventCallback()
/// @see		Instance::SetGamepadEventCallback()
enum class GamepadConnectionEvent : int32_t {
	CONNECTED				= 0x00040001,
	DISCONNECTED			= 0x00040002,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Enumerator for gamepad buttons.
///
///				This mapping includes both XBox and PS style layouts.
enum class GamepadButtons : int32_t {
	A						= 0,			///< XBox A button
	B						= 1,			///< XBox B button
	X						= 2,			///< XBox X button
	Y						= 3,			///< XBox Y button
	LEFT_BUMPER				= 4,			///< LB button
	RIGHT_BUMPER			= 5,			///< RB button
	BACK					= 6,			///< Back/backwards button
	START					= 7,			///< Start/forwards/pause/options button
	GUIDE					= 8,			///< XBox/PS logo button
	LEFT_THUMB				= 9,			///< Left joystick button
	RIGHT_THUMB				= 10,			///< Right joystick button
	DPAD_UP					= 11,			///< DPad up button
	DPAD_RIGHT				= 12,			///< DPad right button
	DPAD_DOWN				= 13,			///< DPad down button
	DPAD_LEFT				= 14,			///< DPad left button
	LAST					= DPAD_LEFT,	///< (NOT A BUTTON, just tells the amount of buttons)

	CROSS					= A,			///< PS cross
	CIRCLE					= B,			///< PS circle
	SQUARE					= X,			///< PS square
	TRIANGLE				= Y,			///< PS triangle
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Enumerator for gamepad axis.
///
///				This mapping includes both XBox and PS style layouts.
enum class GamepadAxis : int32_t {
	LEFT_X					= 0,			///< Left joystick X axis rotation
	LEFT_Y					= 1,			///< Left joystick Y axis rotation
	RIGHT_X					= 2,			///< Right joystick X axis rotation
	RIGHT_Y					= 3,			///< Right joystick Y axis rotation
	LEFT_TRIGGER			= 4,			///< LT rotation
	RIGHT_TRIGGER			= 5,			///< RT rotation
	LAST					= RIGHT_TRIGGER	///< (NOT AN AXIS, just tells the amount of axis)
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This represents the gamepad state at a given time.
///
///				You can query the current state of the gamepad at any given moment with Instance::QueryGamepadState(),
///				the results are stored inside this class.
class GamepadState {
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get gamepad button state.
	///
	/// @param[in]	gamepad_button
	///				Which button's state we wish to query.
	///
	/// @return		true if button was held down, false if button was up.
	inline bool GetButton( GamepadButtons gamepad_button )
	{
		assert( int32_t( gamepad_button ) <= int32_t( GamepadButtons::LAST ) );
		return buttons[ int32_t( gamepad_button ) ];
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get gamepad axis state.
	///
	/// @param		gamepad_axis
	///				Which axis state we wish to query.
	///
	/// @return		Floating point number telling the current rotation of the axis. Usually for gamepad joysticks this range is from
	///				-1.0 to 1.0. For triggers this value ranges from 0.0 to 1.0.
	inline float GetAxis( GamepadAxis gamepad_axis )
	{
		assert( int32_t( gamepad_axis ) <= int32_t( GamepadAxis::LAST ) );
		return axes[ int32_t( gamepad_axis ) ];
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::array<bool, 15>	buttons;
	std::array<float, 6>	axes;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Function pointer type for VK2D report function.
/// 
///				Report function is called every time VK2D has something to report, either errors or warnings. You can create
///				your own in which case your report function signature must match this:
///				<br>
/// @code
///				void VK2D_APIENTRY VK2D_ReportFunction(
///					ReportSeverity					severity,
///					std::string_view						message
///				) {}
/// @endcode
///
/// @param[in]	severity
///				Tells how severe the report was, this can range from being informative, to being a critical error. See
///				ReportSeverity for more info.
/// 
/// @param[in]	message
///				Message from the VK2D that you can pass on to wherever you wish.
using PFN_VK2D_ReportFunction				= void( VK2D_APIENTRY* )(
	ReportSeverity							severity,
	std::string_view						message
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Function pointer type for monitor update callback.
/// 
///				Whenever a new monitor is plugged in or removed VK2D can report about it through a callback function which
///				signature matches this:
///				<br>
/// @code
///				void VK2D_APIENTRY MonitorUpdateCallback () {}
/// @endcode
using PFN_MonitorUpdateCallback				= void ( VK2D_APIENTRY* )( void );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Function pointer type for gamepad connection events.
/// 
///				Whenever a gamepad is connected onto the system or removed VK2D can report about it through a callback matching
///				this signature:
///				<br>
/// @code
///				void VK2D_APIENTRY GamepadConnectionEventCallback(
///					Gamepad						gamepad,
///					GamepadConnectionEvent		event,
///					const std::string		&	gamepad_name
///				) {}
/// @endcode
/// 
/// @param[in]	gamepad
///				Which gamepad was plugged in or removed.
/// 
/// @param[in]	event
///				Connection event that happened, either connected or disconnected.
/// 
/// @param[in]	gamepad_name
///				Reported name of the gamepad that was plugged in.
using PFN_GamepadConnectionEventCallback	= void ( VK2D_APIENTRY* )(
	Gamepad									gamepad,
	GamepadConnectionEvent					event,
	const std::string					&	gamepad_name
);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Parameters to construct a Instance.
struct InstanceCreateInfo {
	std::string								application_name				= {};			///< Name of your application, can be left empty.
	Version									application_version				= {};			///< Version of your application, can be left empty.
	std::string								engine_name						= {};			///< Name of your game engine, can be left empty.
	Version									engine_version					= {};			///< Version of your game engine, can be left empty.
	PFN_VK2D_ReportFunction					report_function					= {};			///< Function to relay VK2D system messages, if left empty VK2D prints to standard output.
	uint32_t								resource_loader_thread_count	= UINT32_MAX;	///< VK2D loads all resources on a separate thread, this parameter allows the host application to control how many threads are used for this. Default = system thread count.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		VK2D Instance which is the basis of which everything else relies on.
///
///				Before you can use VK2D for creating windows or rendering you must create a Instance object somewhere in
///				your application. Everything inside VK2D is created from a Instance object.
///
///				Your application should not have more than one VK2D instance at a time, however it is possible to create more
///				than one if you really need it. Objects created from one instance cannot be shared with another instance.
class Instance {
	friend VK2D_API std::unique_ptr<vk2d::Instance>		VK2D_APIENTRY						CreateInstance(
		const vk2d::InstanceCreateInfo				&	instance_create_info
	);
	friend class Window;
	 
private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Instance constructor. Do not use directly, instead use CreateInstance() factory function.
	/// 
	///	@note		Multithreading: Any thread originally, that thread will then be considered as the main thread for all vk2d
	///				objects created from this instance.
	/// 
	/// @param[in]	instance_create_info
	///				Instance creation parameters struct object. This is handy as creating some objects can require a lot of
	///				parameters.
	VK2D_API										Instance(
		const InstanceCreateInfo				&	instance_create_info );

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destructor.
	///
	///				Automatically destroys everything that was ever created from this instance.
	///
	/// @note		Multithreading: Main thread only. ( The thread which created the instance initially. )
	VK2D_API										~Instance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		General instance update function.
	///
	///				This function should be called once every game loop, even if you don't update all window content each frame you
	///				should still call this function in a somewhat timely manner. 60 times a second or however fast your game loop is
	///				going. This function polls input events from the OS and gives an opportunity for the instance to schedule
	///				resource cleanup and other housekeeping tasks. Perfect place to call this function is in the while loop:
	///				<br>
	/// @code
	///				while( instance->Run() ) {
	///					// Main loop.
	///				}
	/// @endcode
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true when instance can be kept running, false if instance should be shut down.
	VK2D_API bool									VK2D_APIENTRY						Run();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the resource manager needed to load textures, fonts and other resources.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Resource loader created by the instance.
	VK2D_API ResourceManager					*	VK2D_APIENTRY						GetResourceManager();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///	@brief		Get a list of monitors connected to the system.
	///
	///				this will be needed later if the vk2d application is ran fullscreen mode.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Instance::SetMonitorUpdateCallback()
	/// 
	/// @return		A list of handles to monitors.
	VK2D_API std::vector<Monitor*>					VK2D_APIENTRY						GetMonitors();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the primary monitor of the system.
	///
	///				this will be needed later if the vk2d application is ran fullscreen mode.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Instance::SetMonitorUpdateCallback()
	/// 
	/// @return		A handle to the primary monitor attached to the system.
	VK2D_API Monitor							*	VK2D_APIENTRY						GetPrimaryMonitor();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set monitor update callback.
	///
	///				this is to notify your application that some of the monitors got removed or new monitors were plugged into the
	///				system.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	monitor_update_callback_funtion
	///				Function pointer to callback that will be called when monitor is added or removed from the system.
	VK2D_API void									VK2D_APIENTRY						SetMonitorUpdateCallback(
		PFN_MonitorUpdateCallback					monitor_update_callback_funtion );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a new cursor from an image file.
	/// 
	///				Cursor object is needed to set OS cursor image.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Cursor
	/// @see		Instance::DestroyCursor()
	/// 
	/// @param[in]	image_path
	///				Path to the image file, either absolute or relative to the working directory / executable.
	///				Supported formats are JPG, PNG, TGA, BMP, PSD, GIF, PIC.
	/// 
	/// @param[in]	hot_spot
	///				hot spot is an offset from the image 0x0 coords to the tip of the cursor. Eg, circular cursor where you want the
	///				exact centre of the image to be the "tip" and the the image is 64x64 pixels, the hot spot would be 32x32 pixels.
	/// 
	/// @return		Handle to new Cursor object.
	VK2D_API Cursor								*	VK2D_APIENTRY						CreateCursor(
		const std::filesystem::path				&	image_path,
		glm::ivec2									hot_spot );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a new cursor from raw data directly.
	///
	///				Cursor object is needed to set OS cursor image.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Cursor
	/// @see		Color8
	/// @see		Instance::DestroyCursor()
	/// 
	/// @param[in]	image_size
	///				Size of the image in pixels. ( width * height ).
	/// 
	/// @param[in]	image_data
	///				Data for the image is a vector of Color8 objects, input vector data should be organized from left-to-right
	///				top-to-bottom fashion and size of this vector needs to be at least width * height in size or no cursor object is
	///				created.
	/// 
	/// @param[in]	hot_spot
	///				hot spot is an offset from the image 0x0 coords to the tip of the cursor. Eg, circular cursor where you want the
	///				exact centre of the image to be the "tip" and the the image is 64x64 pixels, the hot spot would be 32x32 pixels.
	/// 
	/// @return		Handle to new Cursor object.
	VK2D_API Cursor								*	VK2D_APIENTRY						CreateCursor(
		glm::uvec2									image_size,
		const std::vector<Color8>				&	image_data,
		glm::ivec2									hot_spot );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Manually destroy cursor.
	///
	/// @warning	Cursor must be destroyed by the same Instance that created it.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Cursor
	/// @see		Instance::CreateCursor()
	/// 
	/// @param[in]	cursor
	///				Handle to Cursor object that was created by the same instance, or nullptr.
	VK2D_API void									VK2D_APIENTRY						DestroyCursor(
		Cursor									*	cursor );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the gamepad event callback function.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Instance::SetGamepadEventCallback()
	/// @see		PFN_GamepadConnectionEventCallback()
	/// 
	/// @return		Function pointer to the event callback function that's being called when gamepad event happened.
	VK2D_API PFN_GamepadConnectionEventCallback		VK2D_APIENTRY						GetGamepadEventCallback() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set gamepad event callback function.
	///
	///				the callback gets called if a gamepad gets added or removed from the system.
	///
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		PFN_GamepadConnectionEventCallback
	/// @see		Instance::IsGamepadPresent()
	/// @see		Instance::GetGamepadName()
	/// @see		Instance::QueryGamepadState()
	/// 
	/// @param[in]	gamepad_event_callback_function
	///				PFN_GamepadConnectionEventCallback function that will be called if a gamepad gets added or removed from
	///				the system.
	VK2D_API void									VK2D_APIENTRY						SetGamepadEventCallback(
		PFN_GamepadConnectionEventCallback			gamepad_event_callback_function );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if a specific gamepad is currently attached to the system.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	gamepad
	///				Specific gamepad to check if it's present.
	/// 
	/// @return		true if gamepad is connected to the system, false if not.
	VK2D_API bool									VK2D_APIENTRY						IsGamepadPresent(
		Gamepad										gamepad
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the name of a gamepad if it's attached to the system.
	///
	///				This name is reported by the OS/drivers/gamepad itself and might not be unique.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	gamepad
	///				Specific gamepad to check the name for.
	/// 
	/// @return		Name of the specific gamepad.
	VK2D_API std::string							VK2D_APIENTRY						GetGamepadName(
		Gamepad										gamepad );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the current state of the gamepad.
	///
	///				Tells which buttons are held down and the value of the axis.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		GamepadState
	/// 
	/// @param[in]	gamepad
	///				Specific gamepad to check the state for.
	/// 
	/// @return		Object which tells which buttons were pressed and state of the axis.
	VK2D_API GamepadState							VK2D_APIENTRY						QueryGamepadState(
		Gamepad										gamepad );
	 
	// TODO: gamepad mapping
	//VK2D_API void									VK2D_APIENTRY						SetGamepadMapping();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a new window.
	/// 
	/// @note		CreateWindow is defined as a macro in one of Microsoft Windows APIs so I couldn't easily use that name without
	///				risk of naming conflicts.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Window
	/// 
	/// @param[in]	window_create_info
	///				Window creation parameters struct object. This is handy as creating some objects can require a lot of
	///				parameters.
	/// 
	/// @return		Handle to a newly created window.
	VK2D_API Window								*	VK2D_APIENTRY						CreateOutputWindow(
		const WindowCreateInfo					&	window_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy a window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	window
	///				Handle to Window to destroy. Note that this window handle cannot be used for anything afterwards, if you
	///				try, you'll crash your application. If nullptr, then this function does nothing.
	VK2D_API void									VK2D_APIENTRY						DestroyOutputWindow(
		Window									*	window );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create render target texture.
	///
	///				Render target textures are textures which can be rendered to.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		RenderTargetTexture
	/// 
	/// @param[in]	render_target_texture_create_info
	///				Render target texture creation parameters struct object. This is handy as creating some objects can require a
	///				lot of parameters.
	/// 
	/// @return		Handle to a newly create render target texture.
	VK2D_API RenderTargetTexture				*	VK2D_APIENTRY						CreateRenderTargetTexture(
		const RenderTargetTextureCreateInfo		&	render_target_texture_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy a render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	render_target_texture
	///				Handle to RenderTargetTexture to destroy. Note that this handle cannot be used for anything afterwards, if
	///				you try, you'll crash your application. If nullptr, then this function does nothing.
	VK2D_API void									VK2D_APIENTRY						DestroyRenderTargetTexture(
		RenderTargetTexture						*	render_target_texture );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create a sampler.
	///
	///				Samplers tell how the texture should be read.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	///	@see		Sampler
	/// 
	/// @param[in]	sampler_create_info
	///				Sampler creation parameters struct object. This is handy as creating some objects can require a lot of
	///				parameters.
	/// 
	/// @return		Handle to newly created sampler.
	VK2D_API Sampler							*	VK2D_APIENTRY						CreateSampler(
		const SamplerCreateInfo					&	sampler_create_info );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy sampler.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	sampler
	///				Handle to Sampler to destroy. Note that this handle cannob be used for anything afterwards, if you try,
	///				you'll crash your application. If nullptr, then this function does nothing.
	VK2D_API void									VK2D_APIENTRY						DestroySampler(
		Sampler									*	sampler );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get GPU's maximum supported multisampling.
	///
	///				For example: If the GPU maximum supported samples is 8 samples then only Multisamples::SAMPLE_COUNT_8
	///				is returned.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Maximum supported samples.
	VK2D_API Multisamples							VK2D_APIENTRY						GetMaximumSupportedMultisampling();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get GPU's every supported multisampling setting.
	/// 
	///				In this case return value is considered a bit field, eg. If all 1 through 8 samples are supported then returned
	///				value is a combination of 1 + 2 + 4 + 8 samples. This is useful when checking if the GPU supports a specific
	///				sample count which can be done like:
	///				<br>
	/// @code
	///				if( samples & Multisamples::SAMPLE_COUNT_4 ) {
	///					// Multisample count 4 is supported...
	///				}
	/// @endcode
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		All supported multisamples.
	VK2D_API Multisamples							VK2D_APIENTRY						GetAllSupportedMultisampling();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool									VK2D_APIENTRY						IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::InstanceImpl>	impl;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Factory function for Instance.
///
///				This is the only way to create the VK2D instance in your application.
/// 
///	@note		Multithreading: Any thread originally, that thread will then be considered as the main thread for all VK2D
///				objects created from this instance. See note about multisampling for each function, if the description says
///				"Main thread only", it means that the function may only be called from the same thread as which created this
///				instance.
/// 
/// @see		Instance
///
/// @param[in]	instance_create_info
///				Instance creation parameters struct object. This is handy as creating some objects can require a lot of
///				parameters.
/// 
/// @return		Newly created instance.
VK2D_API std::unique_ptr<Instance>						VK2D_APIENTRY						CreateInstance(
	const InstanceCreateInfo						&	instance_create_info );



}
