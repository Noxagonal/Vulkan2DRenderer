#pragma once

#include "../Core/Common.h"

#include "../Types/Vector2.hpp"
#include "../Types/Rect2.hpp"
#include "../Types/Matrix4.hpp"
#include "../Types/Transform.h"
#include "../Types/Color.hpp"
#include "../Types/MeshPrimitives.hpp"
#include "../Types/Multisamples.h"
#include "../Types/RenderCoordinateSpace.hpp"

#include <memory>
#include <string>
#include <vector>
#include <filesystem>



namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class CursorImpl;
class MonitorImpl;

void UpdateMonitorLists( bool globals_locked );
} // _internal



class Instance;
class Texture;
class Mesh;
class WindowEventHandler;
class Window;
class Cursor;
class Monitor;
class Sampler;



enum class ButtonAction : int32_t {
	RELEASE				= 0,
	PRESS				= 1,
	REPEAT				= 2,
};

enum class MouseButton : int32_t {
	BUTTON_1			= 0,
	BUTTON_2			= 1,
	BUTTON_3			= 2,
	BUTTON_4			= 3,
	BUTTON_5			= 4,
	BUTTON_6			= 5,
	BUTTON_7			= 6,
	BUTTON_8			= 7,
	BUTTON_LAST			= BUTTON_8,
	BUTTON_LEFT			= BUTTON_1,
	BUTTON_RIGHT		= BUTTON_2,
	BUTTON_MIDDLE		= BUTTON_3,
};

enum class ModifierKeyFlags : int32_t {
	SHIFT				= 0x0001,
	CONTROL				= 0x0002,
	ALT					= 0x0004,
	SUPER				= 0x0008
};
inline ModifierKeyFlags operator|( ModifierKeyFlags f1, ModifierKeyFlags f2 )
{
	return ModifierKeyFlags( int32_t( f1 ) | int32_t( f2 ) );
}
inline ModifierKeyFlags operator&( ModifierKeyFlags f1, ModifierKeyFlags f2 )
{
	return ModifierKeyFlags( int32_t( f1 ) & int32_t( f2 ) );
}

enum class CursorState : int32_t {
	NORMAL,		// Normal cursor, allowed to leave the window area and is visible at all times.
	HIDDEN,		// Hidden cursor on window area, cursor is allowed to leave the window area and becomes visible when it does.
	LOCKED		// Cursor is locked to the window, it's not visible and it's now allowed to leave the window area.
};

enum class KeyboardButton : int32_t {
	KEY_UNKNOWN			= -1,

	KEY_SPACE			= 32,
	KEY_APOSTROPHE		= 39,	/* ' */
	KEY_COMMA			= 44,	/* , */
	KEY_MINUS			= 45,	/* - */
	KEY_PERIOD			= 46,	/* . */
	KEY_SLASH			= 47,	/* / */
	KEY_0				= 48,
	KEY_1				= 49,
	KEY_2				= 50,
	KEY_3				= 51,
	KEY_4				= 52,
	KEY_5				= 53,
	KEY_6				= 54,
	KEY_7				= 55,
	KEY_8				= 56,
	KEY_9				= 57,
	KEY_SEMICOLON		= 59,	/* ; */
	KEY_EQUAL			= 61,	/* = */
	KEY_A				= 65,
	KEY_B				= 66,
	KEY_C				= 67,
	KEY_D				= 68,
	KEY_E				= 69,
	KEY_F				= 70,
	KEY_G				= 71,
	KEY_H				= 72,
	KEY_I				= 73,
	KEY_J				= 74,
	KEY_K				= 75,
	KEY_L				= 76,
	KEY_M				= 77,
	KEY_N				= 78,
	KEY_O				= 79,
	KEY_P				= 80,
	KEY_Q				= 81,
	KEY_R				= 82,
	KEY_S				= 83,
	KEY_T				= 84,
	KEY_U				= 85,
	KEY_V				= 86,
	KEY_W				= 87,
	KEY_X				= 88,
	KEY_Y				= 89,
	KEY_Z				= 90,
	KEY_LEFT_BRACKET	= 91,	/* [ */
	KEY_BACKSLASH		= 92,	/* \ */
	KEY_RIGHT_BRACKET	= 93,	/* ] */
	KEY_GRAVE_ACCENT	= 96,	/* ` */
	KEY_WORLD_1			= 161,	/* non-US #1 */
	KEY_WORLD_2			= 162,	/* non-US #2 */

	KEY_ESCAPE			= 256,
	KEY_ENTER			= 257,
	KEY_TAB				= 258,
	KEY_BACKSPACE		= 259,
	KEY_INSERT			= 260,
	KEY_DELETE			= 261,
	KEY_RIGHT			= 262,
	KEY_LEFT			= 263,
	KEY_DOWN			= 264,
	KEY_UP				= 265,
	KEY_PAGE_UP			= 266,
	KEY_PAGE_DOWN		= 267,
	KEY_HOME			= 268,
	KEY_END				= 269,
	KEY_CAPS_LOCK		= 280,
	KEY_SCROLL_LOCK		= 281,
	KEY_NUM_LOCK		= 282,
	KEY_PRINT_SCREEN	= 283,
	KEY_PAUSE			= 284,
	KEY_F1				= 290,
	KEY_F2				= 291,
	KEY_F3				= 292,
	KEY_F4				= 293,
	KEY_F5				= 294,
	KEY_F6				= 295,
	KEY_F7				= 296,
	KEY_F8				= 297,
	KEY_F9				= 298,
	KEY_F10				= 299,
	KEY_F11				= 300,
	KEY_F12				= 301,
	KEY_F13				= 302,
	KEY_F14				= 303,
	KEY_F15				= 304,
	KEY_F16				= 305,
	KEY_F17				= 306,
	KEY_F18				= 307,
	KEY_F19				= 308,
	KEY_F20				= 309,
	KEY_F21				= 310,
	KEY_F22				= 311,
	KEY_F23				= 312,
	KEY_F24				= 313,
	KEY_F25				= 314,
	KEY_KP_0			= 320,
	KEY_KP_1			= 321,
	KEY_KP_2			= 322,
	KEY_KP_3			= 323,
	KEY_KP_4			= 324,
	KEY_KP_5			= 325,
	KEY_KP_6			= 326,
	KEY_KP_7			= 327,
	KEY_KP_8			= 328,
	KEY_KP_9			= 329,
	KEY_KP_DECIMAL		= 330,
	KEY_KP_DIVIDE		= 331,
	KEY_KP_MULTIPLY		= 332,
	KEY_KP_SUBTRACT		= 333,
	KEY_KP_ADD			= 334,
	KEY_KP_ENTER		= 335,
	KEY_KP_EQUAL		= 336,
	KEY_LEFT_SHIFT		= 340,
	KEY_LEFT_CONTROL	= 341,
	KEY_LEFT_ALT		= 342,
	KEY_LEFT_SUPER		= 343,
	KEY_RIGHT_SHIFT		= 344,
	KEY_RIGHT_CONTROL	= 345,
	KEY_RIGHT_ALT		= 346,
	KEY_RIGHT_SUPER		= 347,
	KEY_MENU			= 348,

	KEY_LAST			= KEY_MENU,
};

struct WindowCreateInfo {
	bool								resizeable					= true;											///< Can we use the cursor to resize the window.
	bool								visible						= true;											///< Is the window visible when created.
	bool								decorated					= true;											///< Does the window have default OS borders and buttons.
	bool								focused						= true;											///< Is the window focused and brought forth when created.
	bool								maximized					= false;										///< Is the window maximized to fill the screen when created.
	bool								transparent_framebuffer		= false;										///< Is the alpha value of the render interpreted as a transparent window background.
	vk2d::RenderCoordinateSpace			coordinate_space			= vk2d::RenderCoordinateSpace::TEXEL_SPACE;		///< Window coordinate system to be used, see RenderCoordinateSpace.
	vk2d::Vector2u						size						= { 800, 600 };									///< Window framebuffer initial size
	vk2d::Vector2u						min_size					= { 32, 32 };									///< Minimum size of the window, will be adjusted to suit the hardware.
	vk2d::Vector2u						max_size					= { UINT32_MAX, UINT32_MAX };					///< Maximum size of the window, will be adjusted to suit the hardware.
	vk2d::Monitor					*	fullscreen_monitor			= {};											///< Fullscreen monitor pointer, nullptr is windowed, use Instance::GetPrimaryMonitor() to use primary monitor for fullscreen.
	uint32_t							fullscreen_refresh_rate		= UINT32_MAX;									///< Refresh rate in fullscreen mode, UINT32_MAX uses maximum refresh rate available.
	bool								vsync						= true;											///< Vertical synchronization, works in both windowed and fullscreen modes, usually best left on for 2d graphics.
	vk2d::Multisamples					samples						= vk2d::Multisamples::SAMPLE_COUNT_1;			///< Multisampling, must be a single value in vk2d::Multisamples enum.
	std::string							title						= "";											///< Window title.
	vk2d::WindowEventHandler		*	event_handler				= nullptr;										///< Pointer to a custom event handler that will be used with this window. See vk2d::WindowEventHandler.
};







// Monitor video mode tells you one of the supported modes the monitor can natively work in.
struct MonitorVideoMode {
	vk2d::Vector2u		size;
	uint32_t			redBits;
	uint32_t			greenBits;
	uint32_t			blueBits;
	uint32_t			refreshRate;
};

// Gamma ramp for manual gamma adjustment per color
class GammaRamp {
	friend class vk2d::_internal::MonitorImpl;

public:
	GammaRamp()										= default;
	~GammaRamp()									= default;
	GammaRamp( const vk2d::GammaRamp & other )		= default;
	GammaRamp( vk2d::GammaRamp && other )			= default;

	inline void AddNode(
		uint16_t		r,
		uint16_t		g,
		uint16_t		b
	)
	{
		red.push_back( r );
		green.push_back( g );
		blue.push_back( b );
		++count;
	}

private:
	std::vector<uint16_t>		red;
	std::vector<uint16_t>		green;
	std::vector<uint16_t>		blue;
	uint32_t					count;
};







// Monitor object holds information about the physical monitor
class Monitor {
	friend class vk2d::Window;
	friend class vk2d::_internal::WindowImpl;
	friend void vk2d::_internal::UpdateMonitorLists( bool globals_locked );

	// Monitor constructor from implementation directly, used internally.
	VK2D_API																				Monitor(
		std::unique_ptr<vk2d::_internal::MonitorImpl>	&&	preconstructed_impl );

public:
	// Monitor constructor for an empty monitor.
	VK2D_API																				Monitor();

	// Monitor copy constructor.
	VK2D_API																				Monitor(
		const vk2d::Monitor								&	other );

	// Monitor move constructor.
	VK2D_API																				Monitor(
		vk2d::Monitor									&&	other )							noexcept;

	// Monitor destructor.
	VK2D_API																				~Monitor();

	// Get current video mode, resolution, bits per color and refresh rate.
	// Returns:
	// A const reference to MonitorVideoMode object.
	VK2D_API vk2d::MonitorVideoMode							VK2D_APIENTRY					GetCurrentVideoMode() const;

	// Get all video modes, resolutions, bits per color and refresh rates.
	// Returns:
	// A vector of MonitorVideoMode objects.
	VK2D_API std::vector<vk2d::MonitorVideoMode>			VK2D_APIENTRY					GetVideoModes() const;

	// Set monitor gamma.
	// Parameters:
	// [in] gamma: gamma value between 0 and 1.
	VK2D_API void											VK2D_APIENTRY					SetGamma(
		float												gamma );

	// Get monitor gamma ramp.
	// Gamma ramps work similarly to gamma but per color and per color value.
	// Returns:
	// GammaRamp object which will tell you gamma per color.
	VK2D_API vk2d::GammaRamp								VK2D_APIENTRY					GetGammaRamp();

	// Set monitor gamma ramp.
	// Gamma ramps work similarly to gamma but per color and per color value.
	// It is recommended that the amount of gamma ramp values is set to 255
	// as that is supported by all monitors on all platforms.
	// Parameters:
	// [in] ramp: GammaRamp object into which we should change the gamma to.
	VK2D_API void											VK2D_APIENTRY					SetGammaRamp(
		const vk2d::GammaRamp							&	ramp );

	// Copy operator.
	VK2D_API vk2d::Monitor								&	VK2D_APIENTRY					operator=(
		const vk2d::Monitor								&	other );

	// Move operator.
	VK2D_API vk2d::Monitor								&	VK2D_APIENTRY					operator=(
		vk2d::Monitor									&&	other )							noexcept;

	VK2D_API bool											VK2D_APIENTRY					IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::MonitorImpl>			impl;
};







// Cursor objects hold a cursor image and the cursor image hot spot.
class Cursor {
	friend class vk2d::_internal::InstanceImpl;
	friend class vk2d::Window;
	friend class vk2d::_internal::WindowImpl;

	// Create cursor from image file.
	// [in] instance: instance parent.
	// [in] imagePath: path to an image.
	// [in] hot_spot_x: where the active location of the cursor is.
	VK2D_API																		Cursor(
		vk2d::_internal::InstanceImpl		*	instance,
		const std::filesystem::path			&	image_path,
		vk2d::Vector2i							hot_spot );

	// Create cursor from raw texel data.
	// Texel order is left to right, top to bottom.
	// [in] instance: instance parent.
	// [in] image_size: size of the image in pixels.
	// [in] image_data: raw image data.
	// [in] hot_spot: where the active location of the cursor is.
	VK2D_API																		Cursor(
		vk2d::_internal::InstanceImpl		*	instance,
		vk2d::Vector2u							image_size,
		const std::vector<vk2d::Color8>		&	image_data,
		vk2d::Vector2i							hot_spot );

public:
	// Copy constructor from another cursor.
	VK2D_API																		Cursor(
		vk2d::Cursor						&	other );

	// Move constructor from another cursor.
	VK2D_API																		Cursor(
		vk2d::Cursor						&&	other )								noexcept;

	VK2D_API									VK2D_APIENTRY						~Cursor();

	// Copy operator from another cursor.
	VK2D_API vk2d::Cursor					&	VK2D_APIENTRY						operator=(
		vk2d::Cursor						&	other );

	// Move operator from another cursor.
	VK2D_API vk2d::Cursor					&	VK2D_APIENTRY						operator=(
		vk2d::Cursor						&&	other )								noexcept;

	VK2D_API vk2d::Vector2u						VK2D_APIENTRY						GetSize();
	VK2D_API vk2d::Vector2i						VK2D_APIENTRY						GetHotSpot();
	VK2D_API std::vector<vk2d::Color8>			VK2D_APIENTRY						GetPixelData();

	VK2D_API bool								VK2D_APIENTRY						IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::CursorImpl>	impl;
};







class Window {
	friend class vk2d::_internal::InstanceImpl;

private:
	// Only accessible through Instance::CreateOutputWindow();
	VK2D_API																		Window(
		vk2d::_internal::InstanceImpl				*	instance,
		const vk2d::WindowCreateInfo				&	window_create_info );

public:
	VK2D_API																		~Window();

	// Signal that the window should now close. This function does not actually close the window
	// but rather just sets a flag that it should close, the main program will have to manually
	// remove the window from Instance, this function will however hide the window.
	VK2D_API void										VK2D_APIENTRY				CloseWindow();

	// Checks if the window wants to close.
	// Returns:
	// true if the window requested to be removed and closed, false if the window is good to stay open.
	VK2D_API bool										VK2D_APIENTRY				ShouldClose();

	// Takes a screenshot of the next image that will be rendered.
	// Parameters:
	// [in] save_path: path where the screenshot will be saved to.
	// [in] include_alpha: true if you want background to be transparent, false othervise
	VK2D_API void										VK2D_APIENTRY				TakeScreenshotToFile(
		const std::filesystem::path					&	save_path,
		bool											include_alpha				= false );

	VK2D_API void										VK2D_APIENTRY				TakeScreenshotToData(
		bool											include_alpha);

	// Sets focus to this window, should be called before
	// entering fullscreen from windowed mode.
	VK2D_API void										VK2D_APIENTRY				Focus();

	// Sets fullscreen opacity.
	// Parameters:
	// [in] opacity: a value between 0 and 1 where 1 is completely opaque and 0 is completely transparent.
	VK2D_API void										VK2D_APIENTRY				SetOpacity(
		float											opacity );

	// Gets the fullscreen opacity.
	// Returns: Opacity of this window in range of 0 to 1 where 1 is completely opaque and 0 is completely transparent.
	VK2D_API float										VK2D_APIENTRY				GetOpacity();

	// Hides or unhides the window.
	// Parameters:
	// [in] hidden: false will unhide the window, true will hide the window, user input is uneffected.
	VK2D_API void										VK2D_APIENTRY				Hide(
		bool											hidden );

	// Gets the hidden status of the window.
	// Returns:
	// true if hidden, false if visible.
	VK2D_API bool										VK2D_APIENTRY				IsHidden();

	// Sets the events to be disabled or enabled.
	// Parameters:
	// [in] disable_events: true will disable all events, false will enable all events, including closing the window.
	VK2D_API void										VK2D_APIENTRY				DisableEvents(
		bool											disable_events );

	// Gets the status of are events disabled or not. If window has no event receiver this will always return true.
	// Returns:
	// true if events are disabled, false if they're enabled.
	VK2D_API bool										VK2D_APIENTRY				AreEventsDisabled();

	// Sets window to fullscreen to a specific window.
	// Parameters:
	// [in] monitor: pointer to monitor object. ( See Instance::GetMonitors() and Instance::GetPrimaryMonitor() )
	// [in] frequency: new refresh rate.
	VK2D_API void										VK2D_APIENTRY				SetFullscreen(
		vk2d::Monitor								*	monitor,
		uint32_t										frequency );

	// Checks if the window is fullscreen or not.
	// Returns:
	// true if fullscreen, false if windowed.
	VK2D_API bool										VK2D_APIENTRY				IsFullscreen();

	// Get cursor position.
	// Returns:
	// Vector returning the X and Y coordinates of the mouse cursor position, respectively.
	VK2D_API vk2d::Vector2d								VK2D_APIENTRY				GetCursorPosition();

	// Set cursor position.
	// Parameters:
	// [in] x: new x location of the cursor
	// [in] y: new y location of the cursor
	VK2D_API void										VK2D_APIENTRY				SetCursorPosition(
		vk2d::Vector2d									new_position );

	// Set cursor.
	// Sets the cursor image for the window, hardware cursor.
	// Parameters:
	// [in] cursor: pointer to a Cursor object that we want to use from now on
	VK2D_API void										VK2D_APIENTRY				SetCursor(
		vk2d::Cursor								*	cursor );

	// Get clipboard string.
	// Gets whatever was copied to the system clipboard last, if convertible to string.
	// Returns:
	// contents of the clipboard in string format.
	VK2D_API std::string								VK2D_APIENTRY				GetClipboardString();

	// Set clipboard string.
	// Copies a string to the system clipboard.
	// Parameters:
	// [in] str: string to copy to system clipboard.
	VK2D_API void										VK2D_APIENTRY				SetClipboardString(
		const std::string							&	str );

	// Set window title.
	// Parameters:
	// [in] title: new title of the window as utf-8 string
	VK2D_API void										VK2D_APIENTRY				SetTitle(
		const std::string							&	title );

	// Get window title.
	// Returns:
	// Current title of the window.
	VK2D_API std::string								VK2D_APIENTRY				GetTitle();

	// Set window icon, from image paths.
	// A set of images are provided so that the window can automatically choose the right size for the task.
	// Parameters:
	// [in] imagePaths: paths to new images to be used as the window icon.
	VK2D_API void										VK2D_APIENTRY				SetIcon(
		const std::vector<std::filesystem::path>	&	image_paths );

	// Set window position on screen.
	// Parameters:
	// [in] x: new window position x coordinate.
	// [in] y: new window position y coordinate.
	VK2D_API void										VK2D_APIENTRY				SetPosition(
		vk2d::Vector2i									new_position );

	// Get window position on screen.
	// Returns:
	// An vector containing x and y locations.
	VK2D_API vk2d::Vector2i								VK2D_APIENTRY				GetPosition();

	VK2D_API void										VK2D_APIENTRY				SetSize(
		vk2d::Vector2u									new_size );

	VK2D_API vk2d::Vector2u								VK2D_APIENTRY				GetSize();

	// Sets the window to be minimized or normal.
	// Parameters:
	// [in] minimized: true will minimize the window to the taskbar, false if you want to clear it.
	VK2D_API void										VK2D_APIENTRY				Iconify(
		bool											minimized );

	// Gets the minimized status of the window.
	// Returns:
	// true if the window is minimized, false if normal.
	VK2D_API bool										VK2D_APIENTRY				IsIconified();

	// Sets the window to be maximized or normal.
	// Parameters:
	// [in] maximized: true will maximize the window to fill the screen, false if you want to clear it.
	VK2D_API void										VK2D_APIENTRY				SetMaximized(
		bool											maximized );

	// Gets the maximized status of the window.
	// Returns:
	// true if the window is maximized, false if normal.
	VK2D_API bool										VK2D_APIENTRY				GetMaximized();

	// Sets the cursor to be normal, hidden or locked.
	// Parameters:
	// [in] new_state: see vk2d::CursorState for more information.
	VK2D_API void										VK2D_APIENTRY				SetCursorState(
		vk2d::CursorState								new_state );

	// Returns the state of the cursor. If hidden, the cursor will become
	// visible outside the window boundaries. If locked the cursor is not
	// allowed to leave the window boundaries.
	// Returns:
	// current state of the cursor. See vk2d::CursorState for more information.
	VK2D_API vk2d::CursorState							VK2D_APIENTRY				GetCursorState();

	/// @brief		Begins the render operations. You must call this before using any drawing commands.
	///				For best performance you should calculate game logic first, when you're ready to draw
	///				call this function just before your first draw command. Every draw call must be
	///				between this and vk2d::Window::EndRender().
	/// @see		vk2d::Window::EndRender()
	/// @note		Multithreading: Main thread only.
	/// @return		true if operation was successful, false on error and if you should quit.
	VK2D_API bool										VK2D_APIENTRY				BeginRender();

	/// @brief		Ends the rendering operations. You must call this after you're done drawing
	///				everything in order to display the results on the window surface.
	/// @see		vk2d::Window::BeginRender()
	/// @note		Multithreading: Main thread only.
	/// @return		true if operation was successful, false on error and if you should quit.
	VK2D_API bool										VK2D_APIENTRY				EndRender();

	/// @brief		Draw triangles directly.
	///				Best used if you want to manipulate and draw vertices directly.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	indices
	///				List of indices telling how to form triangles between vertices.
	/// @param[in]	vertices
	///				List of vertices that define the shape.
	/// @param[in]	texture_channel_weights
	///				Only has effect if provided texture has more than 1 layer.
	///				This tell how much weight each texture layer has on each vertex.
	///				TODO: Need to check formatting... Yank Niko, he forgot...
	/// @param[in]	solid
	///				If true, renders solid polygons, if false renders as wireframe.
	/// @param[in]	texture
	///				Pointer to texture, see vk2d::Vertex for UV mapping details.
	///				Can be nullptr in which case a white texture is used (vertex colors only).
	/// @param[in]	sampler
	///				Pointer to sampler which determines how the texture is drawn.
	///				Can be nullptr in which case the default sampler is used.
	VK2D_API void										VK2D_APIENTRY				DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>			&	transformations,
		bool											solid						= true,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	/// @brief		Draws lines directly.
	///				Best used if you want to manipulate and draw vertices directly.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	indices
	///				List of indices telling how to form lines between vertices.
	/// @param[in]	vertices 
	///				List of vertices that define the shape.
	/// @param[in]	texture_channel_weights 
	///				Only has effect if provided texture has more than 1 layer.
	///				This tell how much weight each texture layer has on each vertex.
	///				TODO: Need to check formatting... Yank Niko, he forgot...
	/// @param[in]	texture 
	///				Pointer to texture, see vk2d::Vertex for UV mapping details.
	///				Can be nullptr in which case a white texture is used (vertex colors only).
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn.
	///				Can be nullptr in which case the default sampler is used.
	VK2D_API void										VK2D_APIENTRY				DrawLineList(
		const std::vector<vk2d::VertexIndex_2>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>			&	transformations,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	/// @brief		Draws points directly.
	///				Best used if you want to manipulate and draw vertices directly.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	vertices 
	///				List of vertices that define where and how points are drawn.
	/// @param[in]	texture_channel_weights 
	///				Only has effect if provided texture has more than 1 layer.
	///				This tell how much weight each texture layer has on each vertex.
	///				TODO: Need to check formatting... Yank Niko, he forgot...
	/// @param[in]	texture 
	///				Pointer to texture, see vk2d::Vertex for UV mapping details.
	///				Can be nullptr in which case a white texture is used (vertex colors only).
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn.
	///				Can be nullptr in which case the default sampler is used.
	VK2D_API void										VK2D_APIENTRY				DrawPointList(
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>			&	transformations,
		vk2d::Texture								*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	/// @brief		Draws an individual point.
	///				Inefficient, for when you really just need a single point drawn without extra information.
	///				As soon as you need to draw 2 or more points, use vk2d::Window::DrawPointList() instead.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	location
	///				Where to draw the point to, depends on the coordinate system. See vk2d::RenderCoordinateSpace for more info.
	/// @param[in]	color
	///				Color of the point to be drawn.
	/// @param[in]	size
	///				Size of the point to be drawn, sizes larger than 1.0f will appear as a rectangle.
	VK2D_API void										VK2D_APIENTRY				DrawPoint(
		vk2d::Vector2f									location,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float											size						= 1.0f );

	/// @brief		Draws an individual line.
	///				Inefficient, for when you really just need a single line drawn without extra information.
	///				As soon as you need to draw 2 or more lines, use vk2d::Window::DrawLineList() instead.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	point_1
	///				Coordinates of the starting point of the line, depends on the coordinate system. See vk2d::RenderCoordinateSpace for more info.
	/// @param[in]	point_2
	///				Coordinates of the ending point of the line, depends on the coordinate system. See vk2d::RenderCoordinateSpace for more info.
	/// @param[in]	color 
	///				Color of the line to be drawn.
	VK2D_API void										VK2D_APIENTRY				DrawLine(
		vk2d::Vector2f									point_1,
		vk2d::Vector2f									point_2,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		Draws a rectangle.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	area
	///				Area of the rectangle that will be covered, depends on the coordinate system. See
	///				vk2d::RenderCoordinateSpace for more info about what values should be used.
	/// @param[in]	solid
	///				true if the inside of the rectangle is drawn, false for the outline only.
	/// @param[in]	color
	///				Color of the rectangle to be drawn.
	VK2D_API void										VK2D_APIENTRY				DrawRectangle(
		vk2d::Rect2f									area,
		bool											solid						= true,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		Draws an ellipse or a circle.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	area
	///				Rectangle area in which the ellipse must fit. See vk2d::RenderCoordinateSpace for
	///				more info about what values should be used.
	/// @param[in]	solid
	///				true to draw the inside of the ellipse/circle, false to draw the outline only.
	/// @param[in]	edge_count
	///				How many corners this ellipse should have, or quality if you prefer. This is a float value for
	///				"smoother" transitions between amount of corners, in case this value is animated.
	/// @param[in]	color
	///				Color of the ellipse/circle to be drawn.
	VK2D_API void										VK2D_APIENTRY				DrawEllipse(
		vk2d::Rect2f									area,
		bool											solid						= true,
		float											edge_count					= 64.0f,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		Draws an ellipse or a circle that has a "slice" cut out, similar to usual pie graphs.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	area
	///				Rectangle area in which the ellipse must fit. See vk2d::RenderCoordinateSpace for
	///				more info about what values should be used.
	/// @param[in]	begin_angle_radians
	///				Angle (in radians) where the slice cut should start.
	/// @param[in]	coverage
	///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full ellipse.
	/// @param[in]	solid
	///				true to draw the inside of the pie, false to draw the outline only.
	/// @param[in]	edge_count 
	///				How many corners the complete ellipse should have, or quality if you prefer. This is a float value for
	///				"smoother" transitions between amount of corners, in case this value is animated.
	/// @param[in]	color 
	///				Color of the pie to be drawn.
	VK2D_API void										VK2D_APIENTRY				DrawEllipsePie(
		vk2d::Rect2f									area,
		float											begin_angle_radians,
		float											coverage,
		bool											solid						= true,
		float											edge_count					= 64.0f,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		Draw a rectangular pie, similar to drawing a rectangle but which has a pie slice cut out.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	area
	///				Area of the rectangle to be drawn. See vk2d::RenderCoordinateSpace for
	///				more info about what values should be used.
	/// @param[in]	begin_angle_radians
	///				Angle (in radians) where the slice cut should start.
	/// @param[in]	coverage 
	///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full rectangle.
	/// @param[in]	solid
	///				true to draw the inside of the pie rectangle, false to draw the outline only.
	/// @param[in]	color 
	///				Color of the pie rectangle to be drawn.
	VK2D_API void										VK2D_APIENTRY				DrawRectanglePie(
		vk2d::Rect2f									area,
		float											begin_angle_radians,
		float											coverage,
		bool											solid						= true,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		Draws a rectangle with texture and use the size of the texture to determine size of the rectangle.
	/// @warning	!! If window surface coordinate space is normalized, this will not work properly as bottom
	///				right coordinates are offsetted by the texture size !! See vk2d::RenderCoordinateSpace for more info.
	/// @param[in]	location
	///				Draw location of the texture, this is the top left corner of the texture,
	///				depends on the coordinate system. See vk2d::RenderCoordinateSpace for more info.
	/// @param[in]	texture
	///				Texture to draw.
	/// @param[in]	color 
	///				Color multiplier of the texture texel color, eg. If Red color is 0.0 then texture will
	///				lack all Red color, or if alpha channel of the color is 0.5 then texture appears half transparent.
	VK2D_API void										VK2D_APIENTRY				DrawTexture(
		vk2d::Vector2f									location,
		vk2d::Texture								*	texture,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	/// @brief		The most useful drawing method. Draws vk2d::Mesh which contains all information needed for the render.
	/// @note		Multithreading: Main thread only.
	/// @see		vk2d::Mesh
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// @param[in]	transformation
	///				Draw using transformation.
	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh,
		const vk2d::Transform						&	transformations				= {} );

	/// @brief		The most useful drawing method. Draws vk2d::Mesh which contains all information needed for the render.
	/// @note		Multithreading: Main thread only.
	/// @see		vk2d::Mesh
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw.
	///				Using multiple transformations results the mesh being drawn multiple times using
	///				different transformations. This is also called instanced drawing.
	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh,
		const std::vector<vk2d::Transform>			&	transformation );

	/// @brief		The most useful drawing method. Draws vk2d::Mesh which contains all information needed for the render.
	/// @note		Multithreading: Main thread only.
	/// @see		vk2d::Mesh
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw.
	///				Using multiple transformations results the mesh being drawn multiple times using
	///				different transformations. This is also called instanced drawing.
	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh,
		const std::vector<vk2d::Matrix4f>			&	transformations );

	VK2D_API bool										VK2D_APIENTRY				IsGood() const;


private:
	std::unique_ptr<vk2d::_internal::WindowImpl>		impl;
};







/// @brief		Window event handler base class. You can override member methods to receive keyboard, mouse, gamepad, other... events. <br>
///				Example event handler:
/// @code		
///				class MyEventHandler : public vk2d::WindowEventHandler
///				{
///				public:
///					// Keyboard button was pressed, released or kept down ( repeating ).
///					void										VK2D_APIENTRY		EventKeyboard(
///						vk2d::Window						*	window,
///						vk2d::KeyboardButton					button,
///						int32_t									scancode,
///						vk2d::ButtonAction						action,
///						vk2d::ModifierKeyFlags					modifierKeys
///					) override
///					{};
///				};
///	@endcode
///	@note		You must use VK2D_APIENTRY when overriding events.
class WindowEventHandler {
public:
	/// @brief		Window position changed.
	/// @param[in]	window
	///				Which window's position changed.
	/// @param[in]	position
	///				Where the window moved to.
	virtual void								VK2D_APIENTRY		EventWindowPosition(
		vk2d::Window						*	window,
		vk2d::Vector2i							position )
	{};

	/// @brief		Window size changed.
	/// @param[in]	window
	///				Which window's position changed.
	/// @param[in]	size
	///				what's the new size of the window.
	virtual void								VK2D_APIENTRY		EventWindowSize(
		vk2d::Window						*	window,
		vk2d::Vector2u							size )
	{};

	/// @brief		Window wants to close when this event runs, either the user pressed the "X", the OS
	///				wants to close the window, user pressed Alt+F4...
	///				This event is not called when the window is actually closed, only when the window should be. <br>
	///				Default behavior is: @code window->CloseWindow(); @endcode
	///				If you override this function, you'll have to handle closing the window yourself.
	/// @param[in]	window
	///				Window that should be closed.
	virtual void								VK2D_APIENTRY		EventWindowClose(
		vk2d::Window						*	window )
	{
		window->CloseWindow();
	};

	/// @brief		Window refreshed itself. <br>
	///				Not that useful nowadays as modern OSes don't use CPU to draw the windows anymore,
	///				this doesn't trigger often if at all. Might remove this event later.
	/// @param[in]	window
	///				Window that refreshed itself.
	virtual void								VK2D_APIENTRY		EventWindowRefresh(
		vk2d::Window						*	window )
	{};

	/// @brief		Window gained or lost focus. Ie. Became topmost window, or lost the topmost position.
	/// @param[in]	window
	///				Window that became topmost or lost it's position.
	/// @param[in]	focused
	///				true if the window became topmost, false if it lost the topmost position.
	virtual void								VK2D_APIENTRY		EventWindowFocus(
		vk2d::Window						*	window,
		bool									focused )
	{};

	/// @brief		Window was iconified to the taskbar or recovered from there.
	/// @param[in]	window
	///				Window that was iconified or recovered.
	/// @param[in]	iconified
	///				true if the window was iconified, false if recovered from taskbar.
	virtual void								VK2D_APIENTRY		EventWindowIconify(
		vk2d::Window						*	window,
		bool									iconified )
	{};

	/// @brief		Window was maximized or recovered from maximized state.
	/// @param[in]	window
	///				Window that was maximized or recovered from maximized state.
	/// @param[in]	maximized
	///				true if maximized or false if recevered from maximized state.
	virtual void								VK2D_APIENTRY		EventWindowMaximize(
		vk2d::Window						*	window,
		bool									maximized )
	{};


	/// @brief		Mouse button was pressed or released.
	/// @param[in]	window
	///				Window that the mouse click happened in.
	/// @param[in]	button
	///				Which mouse button was clicked or released.
	/// @param[in]	action
	///				Tells if the button was pressed or released.
	/// @param[in]	modifier_keys
	///				What modifier keys were also pressed down when the mouse button was clicked or released.
	virtual void								VK2D_APIENTRY		EventMouseButton(
		vk2d::Window						*	window,
		vk2d::MouseButton						button,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifier_keys )
	{};

	/// @brief		Mouse moved to a new position on the window.
	/// @param[in]	window
	///				Which window the mouse movement happened in.
	/// @param[in]	position
	///				Tells the new mouse position.
	virtual void								VK2D_APIENTRY		EventCursorPosition(
		vk2d::Window						*	window,
		vk2d::Vector2d							position )
	{};

	/// @brief		Mouse cursor moved on top of the window area, or left it.
	/// @param[in]	window
	///				Which window the mouse cursor entered.
	/// @param[in]	entered
	///				true if entered, false if cursor left the window area.
	virtual void								VK2D_APIENTRY		EventCursorEnter(
		vk2d::Window						*	window,
		bool									entered )
	{};

	/// @brief		Mouse wheel was scrolled.
	/// @param[in]	window
	///				Which window the scrolling happened in.
	/// @param[in]	scroll
	///				Scroll direction vector telling what changed since last event handling.
	///				This is a vector2 because some mice have sideways scrolling. Normal vertical
	///				scrolling is reported in the Y axis, sideways movement in the X axis.
	virtual void								VK2D_APIENTRY		EventScroll(
		vk2d::Window						*	window,
		vk2d::Vector2d							scroll )
	{};

	/// @brief		Keyboard button was pressed, released or kepth down (repeating).
	/// @param[in]	window
	///				Which window the keyboard event happened in.
	/// @param[in]	button
	///				Keyboard button that was pressed, released or is repeating.
	/// @param[in]	scancode
	///				Raw scancode from the keyboard, may change from platform to platform.
	/// @param[in]	action
	///				Tells if the button was pressed or released or repeating.
	/// @param[in]	modifier_keys
	///				What modifier keys were also kept down.
	virtual void								VK2D_APIENTRY		EventKeyboard(
		vk2d::Window						*	window,
		vk2d::KeyboardButton					button,
		int32_t									scancode,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifier_keys )
	{};

	/// @brief		Text input event, use this if you want to know the character that was received from
	///				combination of keyboard presses. Character is in UTF-32 format.
	/// @param[in]	window
	///				Window where we're directing text input to.
	/// @param[in]	character
	///				Resulting combined character from the OS. Eg. A = 'a', Shift+A = 'A'.
	///				This also takes into consideration the region and locale setting of the
	///				OS and keyboard.
	/// @param[in]	modifier_keys
	///				What modifier keys were pressed down when character event was generated.
	virtual void								VK2D_APIENTRY		EventCharacter(
		vk2d::Window						*	window,
		uint32_t								character,
		vk2d::ModifierKeyFlags					modifier_keys )
	{};


	/// @brief		File was drag-dropped onto the window.
	/// @param[in]	window
	///				Window where files were dragged and dropped onto.
	/// @param[in]	files
	///				List of file paths.
	virtual void								VK2D_APIENTRY		EventFileDrop(
		vk2d::Window						*	window,
		std::vector<std::filesystem::path>		files )
	{};

	/// @brief		Screenshot event, called when screenshot was successfully saved to disk
	///				or it's ready for manual manipulation or if there was an error somewhere.
	///				Screenshots can either be saved to disk directly or the image data can be returned here.
	///				Taking screenshots is multithreaded operation and will take a while to process,
	///				so we need to use this event to notify when it's ready.
	/// @param[in]	window
	///				Window where the screenshot was taken from.
	/// @param[in]	screenshot_path
	///				Screenshot save path if screenshot was saved to disk.
	/// @param[in]	screenshot_data
	///				Screenshot data if screenshot was not automatically saved to disk.
	///				WARNING! Returned data will be destroyed as soon as this event finishes,
	///				so if you want to further modify the data you'll have to copy it somewhere else first.
	/// @param[in]	success
	///				true if taking the screenshot was successful, either saved to file or returned here as data.
	///				false if there was an error.
	/// @param[in]	error_message
	///				if success was false then this tells what kind of error we encountered.
	virtual void								VK2D_APIENTRY		EventScreenshot(
		vk2d::Window						*	window,
		const std::filesystem::path			&	screenshot_path,
		const vk2d::ImageData				&	screenshot_data,
		bool									success,
		const std::string					&	error_message )
	{};
};





}
