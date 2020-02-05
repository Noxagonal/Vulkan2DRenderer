#pragma once

#include "../Core/Common.h"

#include "RenderPrimitives.h"

#include <memory>
#include <string>
#include <vector>
#include <filesystem>



namespace vk2d {

namespace _internal {
class RendererImpl;
class WindowImpl;
class CursorImpl;
class MonitorImpl;

void UpdateMonitorLists();
} // _internal



class Renderer;
class TextureResource;
class Mesh;
class WindowEventHandler;
class Window;
class Cursor;
class Monitor;
class Sampler;



enum class Multisamples : uint32_t {
	SAMPLE_COUNT_1		= 1,
	SAMPLE_COUNT_2		= 2,
	SAMPLE_COUNT_4		= 4,
	SAMPLE_COUNT_8		= 8,
	SAMPLE_COUNT_16		= 16,
	SAMPLE_COUNT_32		= 32,
	SAMPLE_COUNT_64		= 64
};
inline vk2d::Multisamples operator&( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) & uint32_t( m2 ) );
}
inline vk2d::Multisamples operator|( vk2d::Multisamples m1, vk2d::Multisamples m2 )
{
	return vk2d::Multisamples( uint32_t( m1 ) | uint32_t( m2 ) );
}

enum class WindowCoordinateSpace : uint32_t {
	TEXEL_SPACE,					// Default, ( 0, 0 ) at top left corner of the screen, bottom right is window extent.
	TEXEL_SPACE_CENTERED,			// Same as TEXEL_SPACE but ( 0, 0 ) coordinates are at screen center.
	NORMALIZED_SPACE,				// Window always contains 1x1 coordinate space, larger side is extended to keep window contents from stretching.
	NORMALIZED_SPACE_CENTERED,		// Same as NORMALIZED SPACE but window always contains 2x2 coordinate space, ( 0, 0 ) is window center
	NORMALIZED_VULKAN,				// ( -1, -1 ) top left, ( 1, 1 ) bottom right.
};

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
	bool								resizeable					= true;			// Can we use the cursor to resize the window.
	bool								visible						= true;			// Is the window visible when created.
	bool								decorated					= true;			// Does the window have default OS borders and buttons.
	bool								focused						= true;			// Is the window focused and brought forth when created.
	bool								maximized					= false;		// Is the window maximized to fill the screen when created.
	bool								transparent_framebuffer		= false;		// Is the alpha value of the render interpreted as a transparent window background.
	vk2d::WindowCoordinateSpace			coordinate_space			= vk2d::WindowCoordinateSpace::TEXEL_SPACE; // Window coordinate system to be used, see WindowCoordinateSpace.
	vk2d::Vector2u						size						= { 800, 600 };	// Window framebuffer initial size
	vk2d::Vector2u						min_size					= { 32, 32 };	// Minimum size of the window, will be adjusted to suit the hardware.
	vk2d::Vector2u						max_size					= { UINT32_MAX, UINT32_MAX };	// Maximum size of the window, will be adjusted to suit the hardware.
	vk2d::Monitor					*	fullscreen_monitor			= {};			// Fullscreen monitor pointer, nullptr is windowed, use Renderer::GetPrimaryMonitor() to use primary monitor for fullscreen.
	uint32_t							fullscreen_refresh_rate		= UINT32_MAX;	// Refresh rate in fullscreen mode, UINT32_MAX uses maximum refresh rate available.
	bool								vsync						= true;			// Vertical synchronization, works in both windowed and fullscreen modes, usually best left on for 2d graphics.
	vk2d::Multisamples					samples						= vk2d::Multisamples::SAMPLE_COUNT_1;	// Multisampling, must be a single value in vk2d::Multisamples enum.
	std::string							title						= "";			// Window title.
	vk2d::WindowEventHandler		*	event_handler				= nullptr;
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
	friend void vk2d::_internal::UpdateMonitorLists();

	// Monitor constructor from implementation directly, used internally.
	VK2D_API																				Monitor(
		std::unique_ptr<vk2d::_internal::MonitorImpl>	&&	preconstructed_impl );

public:
	// Monitor constructor for an empty monitor.
	VK2D_API																				Monitor()			= default;

	// Monitor copy constructor.
	VK2D_API																				Monitor(
		const vk2d::Monitor								&	other );

	// Monitor move constructor.
	VK2D_API																				Monitor(
		vk2d::Monitor									&&	other )							= default;

	// Monitor destructor.
	VK2D_API																				~Monitor()			= default;

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
		vk2d::Monitor									&&	other )							= default;

	VK2D_API bool											VK2D_APIENTRY					IsGood();

private:
	std::unique_ptr<vk2d::_internal::MonitorImpl>			impl							= nullptr;

	bool													is_good							= {};
};







// Cursor objects hold a cursor image and the cursor image hot spot.
class Cursor {
	friend class vk2d::_internal::RendererImpl;
	friend class vk2d::Window;
	friend class vk2d::_internal::WindowImpl;

	// Create cursor from image file.
	// [in] renderer: renderer parent.
	// [in] imagePath: path to an image.
	// [in] hot_spot_x: where the active location of the cursor is.
	VK2D_API																		Cursor(
		vk2d::_internal::RendererImpl		*	renderer,
		const std::filesystem::path			&	image_path,
		vk2d::Vector2i							hot_spot );

	// Create cursor from raw texel data.
	// Texel order is left to right, top to bottom.
	// [in] renderer: renderer parent.
	// [in] image_size: size of the image in pixels.
	// [in] image_data: raw image data.
	// [in] hot_spot: where the active location of the cursor is.
	VK2D_API																		Cursor(
		vk2d::_internal::RendererImpl		*	renderer,
		vk2d::Vector2u							image_size,
		const std::vector<vk2d::Color8>		&	image_data,
		vk2d::Vector2i							hot_spot );

public:
	// Copy constructor from another cursor.
	VK2D_API																		Cursor(
		vk2d::Cursor						&	other );

	// Move constructor from another cursor.
	VK2D_API																		Cursor(
		vk2d::Cursor						&&	other )								= default;

	// Destructor for cursor.
	VK2D_API																		~Cursor();

	// Copy operator from another cursor.
	VK2D_API vk2d::Cursor					&	VK2D_APIENTRY						operator=(
		vk2d::Cursor						&	other );

	// Move operator from another cursor.
	VK2D_API vk2d::Cursor					&	VK2D_APIENTRY						operator=(
		vk2d::Cursor						&&	other )								= default;

	VK2D_API vk2d::Vector2u						VK2D_APIENTRY						GetSize();
	VK2D_API vk2d::Vector2i						VK2D_APIENTRY						GetHotSpot();
	VK2D_API std::vector<vk2d::Color8>			VK2D_APIENTRY						GetPixelData();

	VK2D_API bool								VK2D_APIENTRY						IsGood();

private:
	std::unique_ptr<vk2d::_internal::CursorImpl>	impl							= {};

	bool										is_good								= {};
};







class Window {
	friend class vk2d::_internal::RendererImpl;

private:
	// Only accessible through Renderer::CreateOutputWindow();
	VK2D_API																		Window(
		vk2d::_internal::RendererImpl				*	renderer_parent,
		vk2d::WindowCreateInfo						&	window_create_info );

public:
	VK2D_API																		~Window();

	// Signal that the window should now close. This function does not actually close the window
	// but rather just sets a flag that it should close, the main program will have to manually
	// remove the window from Renderer, this function will however hide the window.
	VK2D_API void										VK2D_APIENTRY				CloseWindow();

	// Checks if the window wants to close.
	// Returns:
	// true if the window requested to be removed and closed, false if the window is good to stay open.
	VK2D_API bool										VK2D_APIENTRY				ShouldClose();

	// Update window events. Need to be called once a frame.
	VK2D_API void										VK2D_APIENTRY				UpdateEvents();

	// Takes a screenshot of the next image that will be rendered.
	// Parameters:
	// [in] save_path: path where the screenshot will be saved to.
	// [in] include_alpha: true if you want background to be transparent, false othervise
	VK2D_API void										VK2D_APIENTRY				TakeScreenshot(
		const std::filesystem::path					&	save_path,
		bool											include_alpha				= false );

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
	// [in] monitor: pointer to monitor object. ( See Renderer::GetMonitors() and Renderer::GetPrimaryMonitor() )
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

	// Begins the render operations. You must call this before using any drawing commands.
	// For best performance you should calculate game logic first, when you're ready to draw
	// call this function just before your first draw command.
	VK2D_API bool										VK2D_APIENTRY				BeginRender();

	// Ends the rendering operations. You must call this after you're done drawing.
	// This will display the results on screen.
	VK2D_API bool										VK2D_APIENTRY				EndRender();

	VK2D_API void										VK2D_APIENTRY				DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		bool											filled						= true,
		vk2d::TextureResource						*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLineList(
		const std::vector<vk2d::VertexIndex_2>		&	indices,
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		vk2d::TextureResource						*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawPointList(
		const std::vector<vk2d::Vertex>				&	vertices,
		const std::vector<float>					&	texture_channels,
		vk2d::TextureResource						*	texture						= nullptr,
		vk2d::Sampler								*	sampler						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLine(
		vk2d::Vector2f									point_1,
		vk2d::Vector2f									point_2,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawBox(
		vk2d::Vector2f									top_left,
		vk2d::Vector2f									bottom_right,
		bool											filled						= true,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawCircle(
		vk2d::Vector2f									top_left,
		vk2d::Vector2f									bottom_right,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPie(
		vk2d::Vector2f									top_left,
		vk2d::Vector2f									bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPieBox(
		vk2d::Vector2f									top_left,
		vk2d::Vector2f									bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawTexture(
		vk2d::Vector2f									top_left,
		vk2d::Vector2f									bottom_right,
		vk2d::TextureResource						*	texture,
		vk2d::Colorf									color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh );


private:
	std::unique_ptr<vk2d::_internal::WindowImpl>		impl				= {};

	bool												is_good				= {};
};







// Window event handler
// Responsible in signalling back events from the window object
class WindowEventHandler {
public:
	// Window position changed.
	virtual void								VK2D_APIENTRY		EventWindowPosition(
		vk2d::Window						*	window,
		vk2d::Vector2i							position )
	{};

	// Window size changed.
	virtual void								VK2D_APIENTRY		EventWindowSize(
		vk2d::Window						*	window,
		vk2d::Vector2u							size )
	{};

	// Window wants to close, either the window "X" was pressed or system wants to close the window.
	// This function will not be called when the window will actually close, only when window wants to be closed.
	// Window::CloseWindow() does not call this event.
	virtual void								VK2D_APIENTRY		EventWindowClose(
		vk2d::Window						*	window )
	{
		window->CloseWindow();
	};

	// Window refreshed itself, not as useful nowadays.
	virtual void								VK2D_APIENTRY		EventWindowRefresh(
		vk2d::Window						*	window )
	{};

	// Window gained or lost focus.
	virtual void								VK2D_APIENTRY		EventWindowFocus(
		vk2d::Window						*	window,
		bool									focused )
	{};

	// Window was iconified or recovered from iconified state.
	virtual void								VK2D_APIENTRY		EventWindowIconify(
		vk2d::Window						*	window,
		bool									iconified )
	{};

	// Window was maximized or recovered from maximized state.
	virtual void								VK2D_APIENTRY		EventWindowMaximize(
		vk2d::Window						*	window,
		bool									maximized )
	{};


	// Mouse button pressed or released.
	virtual void								VK2D_APIENTRY		EventMouseButton(
		vk2d::Window						*	window,
		vk2d::MouseButton						button,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifierKeys )
	{};

	// Cursor position on window changed.
	virtual void								VK2D_APIENTRY		EventCursorPosition(
		vk2d::Window						*	window,
		vk2d::Vector2d							position )
	{};

	// Cursor entered or left window client area.
	virtual void								VK2D_APIENTRY		EventCursorEnter(
		vk2d::Window						*	window,
		bool									entered )
	{};

	// Scrolling happened, y for vertical scrolling, x for horisontal.
	virtual void								VK2D_APIENTRY		EventScroll(
		vk2d::Window						*	window,
		vk2d::Vector2d							scroll )
	{};

	// Keyboard button was pressed, released or kept down ( repeating ).
	virtual void								VK2D_APIENTRY		EventKeyboard(
		vk2d::Window						*	window,
		vk2d::KeyboardButton					button,
		int32_t									scancode,
		vk2d::ButtonAction						action,
		vk2d::ModifierKeyFlags					modifierKeys )
	{};

	// Character input, use this if you want to know the character that was received from combination of keyboard presses, character is in UTF-32 format.
	virtual void								VK2D_APIENTRY		EventCharacter(
		vk2d::Window						*	window,
		uint32_t								character,
		vk2d::ModifierKeyFlags					modifierKeys )
	{};


	// File or files were dropped on window.
	virtual void								VK2D_APIENTRY		EventFileDrop(
		vk2d::Window						*	window,
		std::vector<std::filesystem::path>		files )
	{};

	// Screenshot events, called when screenshot save was successfully saved on disk or if there was an error, if error, error message is also given.
	virtual void								VK2D_APIENTRY		EventScreenshot(
		vk2d::Window						*	window,
		const std::filesystem::path			&	path,
		bool									success,
		const std::string					&	errorMessage )
	{};
};





}
