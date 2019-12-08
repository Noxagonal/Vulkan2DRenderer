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
} // _internal



class Renderer;
class TextureResource;
class Mesh;
class Cursor;



enum class Multisamples : uint32_t {
	SAMPLE_COUNT_1		= 1,
	SAMPLE_COUNT_2		= 2,
	SAMPLE_COUNT_4		= 4,
	SAMPLE_COUNT_8		= 8,
	SAMPLE_COUNT_16		= 16,
	SAMPLE_COUNT_32		= 32,
	SAMPLE_COUNT_64		= 64
};

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

enum class Joystick : int32_t {
	JOYSTICK_1			= 0,
	JOYSTICK_2			= 1,
	JOYSTICK_3			= 2,
	JOYSTICK_4			= 3,
	JOYSTICK_5			= 4,
	JOYSTICK_6			= 5,
	JOYSTICK_7			= 6,
	JOYSTICK_8			= 7,
	JOYSTICK_9			= 8,
	JOYSTICK_10			= 9,
	JOYSTICK_11			= 10,
	JOYSTICK_12			= 11,
	JOYSTICK_13			= 12,
	JOYSTICK_14			= 13,
	JOYSTICK_15			= 14,
	JOYSTICK_16			= 15,
	JOYSTICK_LAST		= JOYSTICK_16,
};

enum class JoystickEvent : int32_t {
	CONNECTED			= 0x00040001,
	DISCONNECTED		= 0x00040002,
};

enum JoystickButton : int32_t {
	JOYSTICK_BUTTON_1			= 0,
	JOYSTICK_BUTTON_2			= 1,
	JOYSTICK_BUTTON_3			= 2,
	JOYSTICK_BUTTON_4			= 3,
	JOYSTICK_BUTTON_5			= 4,
	JOYSTICK_BUTTON_6			= 5,
	JOYSTICK_BUTTON_7			= 6,
	JOYSTICK_BUTTON_8			= 7,
	JOYSTICK_BUTTON_9			= 8,
	JOYSTICK_BUTTON_10			= 9,
	JOYSTICK_BUTTON_11			= 10,
	JOYSTICK_BUTTON_12			= 11,
	JOYSTICK_BUTTON_13			= 12,
	JOYSTICK_BUTTON_14			= 13,
	JOYSTICK_BUTTON_15			= 14,
	JOYSTICK_BUTTON_16			= 15,
	JOYSTICK_BUTTON_17			= 16,
	JOYSTICK_BUTTON_18			= 17,
	JOYSTICK_BUTTON_19			= 18,
	JOYSTICK_BUTTON_20			= 19,
	JOYSTICK_BUTTON_21			= 20,
	JOYSTICK_BUTTON_22			= 21,
	JOYSTICK_BUTTON_23			= 22,
	JOYSTICK_BUTTON_24			= 23,
	JOYSTICK_BUTTON_25			= 24,
	JOYSTICK_BUTTON_26			= 25,
	JOYSTICK_BUTTON_27			= 26,
	JOYSTICK_BUTTON_28			= 27,
	JOYSTICK_BUTTON_29			= 28,
	JOYSTICK_BUTTON_30			= 29,
	JOYSTICK_BUTTON_31			= 30,
	JOYSTICK_BUTTON_32			= 31,

	JOYSTICK_BUTTON_XBOX360_A				= JOYSTICK_BUTTON_1,
	JOYSTICK_BUTTON_XBOX360_B				= JOYSTICK_BUTTON_2,
	JOYSTICK_BUTTON_XBOX360_X				= JOYSTICK_BUTTON_3,
	JOYSTICK_BUTTON_XBOX360_Y				= JOYSTICK_BUTTON_4,
	JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER		= JOYSTICK_BUTTON_5,
	JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER	= JOYSTICK_BUTTON_6,
	JOYSTICK_BUTTON_XBOX360_BACK			= JOYSTICK_BUTTON_7,
	JOYSTICK_BUTTON_XBOX360_START			= JOYSTICK_BUTTON_8,
	JOYSTICK_BUTTON_XBOX360_LEFT_ANALOG		= JOYSTICK_BUTTON_9,
	JOYSTICK_BUTTON_XBOX360_RIGHT_ANALOG	= JOYSTICK_BUTTON_10,
	JOYSTICK_BUTTON_XBOX360_DPAD_UP			= JOYSTICK_BUTTON_11,
	JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT		= JOYSTICK_BUTTON_12,
	JOYSTICK_BUTTON_XBOX360_DPAD_DOWN		= JOYSTICK_BUTTON_13,
	JOYSTICK_BUTTON_XBOX360_DPAD_LEFT		= JOYSTICK_BUTTON_14,
};

enum JoystickAxes : int32_t {
	JOYSTICK_AXEL_1				= 0,
	JOYSTICK_AXEL_2				= 1,
	JOYSTICK_AXEL_3				= 2,
	JOYSTICK_AXEL_4				= 3,
	JOYSTICK_AXEL_5				= 4,
	JOYSTICK_AXEL_6				= 5,
	JOYSTICK_AXEL_7				= 6,
	JOYSTICK_AXEL_8				= 7,
	JOYSTICK_AXEL_9				= 8,
	JOYSTICK_AXEL_10			= 9,
	JOYSTICK_AXEL_11			= 10,
	JOYSTICK_AXEL_12			= 11,
	JOYSTICK_AXEL_13			= 12,
	JOYSTICK_AXEL_14			= 13,
	JOYSTICK_AXEL_15			= 14,
	JOYSTICK_AXEL_16			= 15,
	JOYSTICK_AXEL_17			= 16,
	JOYSTICK_AXEL_18			= 17,
	JOYSTICK_AXEL_19			= 18,
	JOYSTICK_AXEL_20			= 19,
	JOYSTICK_AXEL_21			= 20,
	JOYSTICK_AXEL_22			= 21,
	JOYSTICK_AXEL_23			= 22,
	JOYSTICK_AXEL_24			= 23,
	JOYSTICK_AXEL_25			= 24,
	JOYSTICK_AXEL_26			= 25,
	JOYSTICK_AXEL_27			= 26,
	JOYSTICK_AXEL_28			= 27,
	JOYSTICK_AXEL_29			= 28,
	JOYSTICK_AXEL_30			= 29,
	JOYSTICK_AXEL_31			= 30,
	JOYSTICK_AXEL_32			= 31,

	JOYSTICK_AXEL_XBOX360_LEFT_ANALOG_X		= JOYSTICK_AXEL_1,
	JOYSTICK_AXEL_XBOX360_LEFT_ANALOG_Y		= JOYSTICK_AXEL_2,
	JOYSTICK_AXEL_XBOX360_RIGHT_ANALOG_X	= JOYSTICK_AXEL_3,
	JOYSTICK_AXEL_XBOX360_RIGHT_ANALOG_Y	= JOYSTICK_AXEL_4,
	JOYSTICK_AXEL_XBOX360_LEFT_TRIGGER		= JOYSTICK_AXEL_5,
	JOYSTICK_AXEL_XBOX360_RIGHT_TRIGGER		= JOYSTICK_AXEL_6,
};

// Window event handler
// Responsible in signalling back events from the window object
class WindowEventHandler {
public:
	// Window position changed.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowPosition(
		vk2d::Window				*	window,
		int32_t							position_x,
		int32_t							position_y )
	{};

	// Window size changed.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowSize(
		vk2d::Window				*	window,
		uint32_t						size_x,
		uint32_t						size_y )
	{};

	// Window was closed, either via the "X" or someone called Window::Close().
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowClose(
		vk2d::Window				*	window )
	{};

	// Window refreshed itself, not as useful nowadays.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowRefresh(
		vk2d::Window				*	window )
	{};

	// Window gained or lost focus.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowFocus(
		vk2d::Window				*	window,
		bool							focused )
	{};

	// Window was iconified or recovered from iconified state.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowIconify(
		vk2d::Window				*	window,
		bool							iconified )
	{};

	// Window was maximized or recovered from maximized state.
	virtual VK2D_API void				VK2D_APIENTRY		EventWindowMaximize(
		vk2d::Window				*	window,
		bool							maximized )
	{};

	// Framebuffer size changed, you should listen to this event if you want to change your rendering output size to match the window's.
	virtual VK2D_API void				VK2D_APIENTRY		EventFramebufferSize(
		vk2d::Window				*	window,
		uint32_t						size_x,
		uint32_t						size_y )
	{};


	// Mouse button pressed or released.
	virtual VK2D_API void				VK2D_APIENTRY		EventMouseButton(
		vk2d::Window				*	window,
		vk2d::MouseButton				button,
		vk2d::ButtonAction				action,
		vk2d::ModifierKeyFlags			modifierKeys )
	{};

	// Cursor position on window changed.
	virtual VK2D_API void				VK2D_APIENTRY		EventCursorPosition(
		vk2d::Window				*	window,
		double							x,
		double							y )
	{};

	// Cursor entered or left window client area.
	virtual VK2D_API void				VK2D_APIENTRY		EventCursorEnter(
		vk2d::Window				*	window,
		bool							entered )
	{};

	// Scrolling happened, y for vertical scrolling, x for horisontal.
	virtual VK2D_API void				VK2D_APIENTRY		EventScroll(
		vk2d::Window				*	window,
		double							x,
		double							y )
	{};

	// Keyboard button was pressed, released or kept down ( repeating ).
	virtual VK2D_API void				VK2D_APIENTRY		EventKeyboard(
		vk2d::Window				*	window,
		vk2d::KeyboardButton			button,
		int								scancode,
		vk2d::ButtonAction				action,
		vk2d::ModifierKeyFlags			modifierKeys )
	{};

	// Character input, use this if you want to know the character that was received from combination of keyboard presses, character is in UTF-32 format.
	virtual VK2D_API void				VK2D_APIENTRY		EventCharacter(
		vk2d::Window				*	window,
		uint32_t						character,
		vk2d::ModifierKeyFlags			modifierKeys )
	{};


	// File or files were dropped on window.
	virtual VK2D_API void				VK2D_APIENTRY		EventFileDrop(
		vk2d::Window				*	window,
		std::vector<std::string>		files )
	{};


	// Joystick events, connected or disconnected, called for everyone that's listening
	virtual VK2D_API void				VK2D_APIENTRY		EventJoystic(
		vk2d::Joystick					joystick,
		vk2d::JoystickEvent				event,
		const std::string			&	joystickName )
	{};

	// Joystick query of buttons, called every time you update the window and reports the button states rather than events, called for everyone that's listening.
	virtual VK2D_API void				VK2D_APIENTRY		QueryJoysticButtons(
		vk2d::Joystick					joystick,
		const std::vector<bool>		&	buttons )
	{};

	// Joystick query of axes, called every time you update the window and reports the axel states rather than events, called for everyone that's listening.
	virtual VK2D_API void				VK2D_APIENTRY		QueryJoysticAxes(
		vk2d::Joystick					joystick,
		const std::vector<float>	&	axes )
	{};

	// Screenshot events, called when screenshot save was successfully saved on disk or if there was an error, if error, error message is also given.
	virtual VK2D_API void				VK2D_APIENTRY		EventScreenshot(
		vk2d::Window				*	window,
		const std::string			&	path,
		bool							success,
		const std::string			&	errorMessage )
	{};
};

struct WindowCreateInfo {
	bool								resizeable					= true;			// Can we use the cursor to resize the window.
	bool								visible						= true;			// Is the window visible when created.
	bool								decorated					= true;			// Does the window have default OS borders and buttons.
	bool								focused						= true;			// Is the window focused and brought forth when created.
	bool								maximized					= false;		// Is the window maximized to fill the screen when created.
	bool								transparent_framebuffer		= false;		// Is the alpha value of the render interpreted as a transparent window background.
	vk2d::WindowCoordinateSpace			coordinate_space			= WindowCoordinateSpace::TEXEL_SPACE; // Window coordinate system to be used, see WindowCoordinateSpace.
	uint32_t							width						= 0;			// Window framebuffer initial width.
	uint32_t							height						= 0;			// Window framebuffer initial height.
	uint32_t							min_width					= 32;			// Minimum width of the window, will be adjusted to suit the hardware.
	uint32_t							min_height					= 32;			// Minimum height of the window, will be adjusted to suit the hardware.
	uint32_t							max_width					= UINT32_MAX;	// Maximum width of the window, will be adjusted to suit the hardware.
	uint32_t							max_height					= UINT32_MAX;	// Maximum height of the window, will be adjusted to suit the hardware.
	uint32_t							fullscreen_monitor			= 0;			// Fullscreen monitor index, 0 means windowed, 1 is primary, any value larger than amount of monitors goes to primary monitor.
	uint32_t							fullscreen_refresh_rate		= UINT32_MAX;	// Refresh rate in fullscreen mode, UINT32_MAX uses maximum refresh rate available.
	bool								vsync						= true;			// Vertical synchronization, works in both windowed and fullscreen modes, usually best left on for 2d graphics.
	Multisamples						samples						= Multisamples::SAMPLE_COUNT_1;	// Multisampling, must be a value in Multisamples enum.
	std::string							title						= "";			// Window title.
	std::filesystem::path				window_icon					= "";
	vk2d::WindowEventHandler		*	event_handler				= nullptr;
};



class Window {
	friend class _internal::RendererImpl;

private:
	// Only accessible through Renderer::CreateOutputWindow();
	VK2D_API																		Window(
		_internal::RendererImpl						*	renderer_parent,
		WindowCreateInfo							&	window_create_info );

public:
	VK2D_API																		~Window();

	// Update window events. Need to be called once a frame.
	VK2D_API void										VK2D_APIENTRY				UpdateEvents();

	// Takes a screenshot of the next image that will be rendered.
	// Parameters:
	// [in] save_path: path where the screenshot will be saved to.
	VK2D_API void										VK2D_APIENTRY				TakeScreenshot(
		std::filesystem::path							save_path );

	VK2D_API bool										VK2D_APIENTRY				IsFullscreen();
	VK2D_API bool										VK2D_APIENTRY				SetFullscreen(
		uint32_t										monitor,
		uint32_t										frequency );

	// Get cursor position.
	// Returns:
	// Array of 2 doubles returning the X and Y coordinates of the mouse cursor position.
	VK2D_API std::array<double, 2>						VK2D_APIENTRY				GetCursorPosition();

	// Set cursor position.
	// Parameters:
	// [in] x: new x location of the cursor
	// [in] y: new y location of the cursor
	VK2D_API void										VK2D_APIENTRY				SetCursorPosition(
		double											x,
		double											y );

	// Set cursor.
	// Sets the cursor image for the window, hardware cursor.
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

	// Set window icon, from image paths.
	// A set of images are provided so that the window can automatically choose the right size for the task.
	// Parameters:
	// [in] imagePaths: paths to new images to be used as the window icon.
	VK2D_API void										VK2D_APIENTRY				SetIcon(
		const std::vector<std::filesystem::path>	&	image_paths );

	VK2D_API bool										VK2D_APIENTRY				BeginRender();
	VK2D_API bool										VK2D_APIENTRY				EndRender();

	VK2D_API void										VK2D_APIENTRY				DrawTriangleList(
		const std::vector<Vertex>					&	vertices,
		const std::vector<VertexIndex_3>			&	indices,
		bool											filled						= true,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLineList(
		const std::vector<Vertex>					&	vertices,
		const std::vector<VertexIndex_2>			&	indices,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawPointList(
		const std::vector<Vertex>					&	vertices,
		TextureResource								*	texture						= nullptr );

	VK2D_API void										VK2D_APIENTRY				DrawLine(
		Vector2d										point_1,
		Vector2d										point_2,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawBox(
		Vector2d										top_left,
		Vector2d										bottom_right,
		bool											filled						= true,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawCircle(
		Vector2d										top_left,
		Vector2d										bottom_right,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPie(
		Vector2d										top_left,
		Vector2d										bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		float											edge_count					= 64.0f,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawPieBox(
		Vector2d										top_left,
		Vector2d										bottom_right,
		float											begin_angle_radians,
		float											coverage,
		bool											filled						= true,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawTexture(
		Vector2d										top_left,
		Vector2d										bottom_right,
		vk2d::TextureResource						*	texture,
		Color											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void										VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh							&	mesh );


private:
	std::unique_ptr<_internal::WindowImpl>				impl				= {};

	bool												is_good				= {};
};



// Cursor objects hold a cursor image and the cursor image hot spot.
class Cursor {
	friend class vk2d::Window;

public:
	// Cursor constructor, image path version.
	// [in] imagePath: path to an image.
	// [in] hot_spot_x: where the active location of the cursor is, x location.
	// [in] hot_spot_y: where the active location of the cursor is, y location.
	VK2D_API																		Cursor(
		const std::filesystem::path			&	image_path,
		int32_t									hot_spot_x,
		int32_t									hot_spot_y );

	// Cursor constructor, raw data version.
	// Image data needs to be in format RGBA, 8 bits per channel, 32 bits per pixel,
	// in order left to right - top to bottom.
	// [in] image_size: size of the image in pixels, x dimension.
	// [in] image_size: size of the image in pixels, y dimension.
	// [in] image_data: raw image data.
	// [in] hot_spot_x: where the active location of the cursor is, x location.
	// [in] hot_spot_y: where the active location of the cursor is, y location.
	VK2D_API																		Cursor(
		uint32_t								image_size_x,
		uint32_t								image_size_y,
		const std::vector<vk2d::Color>		&	image_data,
		int32_t									hot_spot_x,
		int32_t									hot_spot_y );

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

	VK2D_API std::array<uint32_t, 2>			VK2D_APIENTRY						GetExtent();
	VK2D_API std::array<int32_t, 2>				VK2D_APIENTRY						GetHotSpot();
	VK2D_API const std::vector<vk2d::Color>	&	VK2D_APIENTRY						GetPixelData();

private:
	std::unique_ptr<vk2d::_internal::CursorImpl>	impl							= nullptr;

	bool										is_good								= {};
};




}
