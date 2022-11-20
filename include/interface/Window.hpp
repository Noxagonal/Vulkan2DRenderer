#pragma once

#include <core/Common.hpp>

#include <containers/Rect2.hpp>
#include <containers/Transform.hpp>
#include <containers/Color.hpp>
#include <mesh/Mesh.hpp>
#include <containers/Multisamples.hpp>
#include <containers/RenderCoordinateSpace.hpp>
#include <mesh/vertex/RawVertexData.hpp>
#include <mesh/vertex/VertexBase.hpp>

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <span>



namespace vk2d {

namespace vk2d_internal {
class InstanceImpl;
class WindowImpl;
class CursorImpl;

void UpdateMonitorLists( bool globals_locked );
} // vk2d_internal



class Instance;
class Texture;
class MeshBase;
class WindowEventHandler;
class Window;
class Cursor;
class Monitor;
class Sampler;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Tells what action happened with a button.
enum class ButtonAction : int32_t {
	RELEASE				= 0,	///< Button was lift up.
	PRESS				= 1,	///< Button was pressed down.
	REPEAT				= 2,	///< Button was held down long and is being repeated by the OS, this is used in text input when user wants to insert same character multiple times.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Tells which mouse button was pressed or released.
enum class MouseButton : int32_t {
	BUTTON_1			= 0,		///< Left mouse button
	BUTTON_2			= 1,		///< Right mouse button
	BUTTON_3			= 2,		///< Middle mouse button
	BUTTON_4			= 3,		///< Forward side button
	BUTTON_5			= 4,		///< Backward side button
	BUTTON_6			= 5,		///< (Extra mouse button)
	BUTTON_7			= 6,		///< (Extra mouse button)
	BUTTON_8			= 7,		///< (Extra mouse button)
	BUTTON_LAST			= BUTTON_8,	///< (Extra mouse button)
	BUTTON_LEFT			= BUTTON_1,	///< Left mouse button
	BUTTON_RIGHT		= BUTTON_2,	///< Right mouse button
	BUTTON_MIDDLE		= BUTTON_3,	///< Middle mouse button
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Tells which modifier keys were held down during an action.
enum class ModifierKeyFlags : int32_t {
	SHIFT				= 0x0001,	///< Shift key, either left or right
	CONTROL				= 0x0002,	///< Ctrl key, either left or right
	ALT					= 0x0004,	///< Alt key, either left or right
	SUPER				= 0x0008	///< Windows key, either left or right
};
inline ModifierKeyFlags operator|( ModifierKeyFlags f1, ModifierKeyFlags f2 )
{
	return ModifierKeyFlags( int32_t( f1 ) | int32_t( f2 ) );
}
inline ModifierKeyFlags operator&( ModifierKeyFlags f1, ModifierKeyFlags f2 )
{
	return ModifierKeyFlags( int32_t( f1 ) & int32_t( f2 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Cursor state dictates the behavior with window.
enum class CursorState : int32_t {
	NORMAL,		///< Normal cursor, allowed to leave the window area and is visible at all times.
	HIDDEN,		///< Hidden cursor on window area, cursor is allowed to leave the window area and becomes visible when it does.
	LOCKED		///< Cursor is locked to the window, it's not visible and it's typically not allowed to leave the window area.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Key codes for each individual keyboard button.
enum class KeyboardButton : int32_t {
	KEY_UNKNOWN			= -1,	///< Unrecognized keyboard button

	KEY_SPACE			= 32,	///< Space bar
	KEY_APOSTROPHE		= 39,	///< <tt>'</tt>
	KEY_COMMA			= 44,	///< <tt>,</tt>
	KEY_MINUS			= 45,	///< <tt>-</tt>
	KEY_PERIOD			= 46,	///< <tt>.</tt>
	KEY_SLASH			= 47,	///< <tt>/</tt> forward slash
	KEY_0				= 48,	///< Top row <tt>0</tt>
	KEY_1				= 49,	///< Top row <tt>1</tt>
	KEY_2				= 50,	///< Top row <tt>2</tt>
	KEY_3				= 51,	///< Top row <tt>3</tt>
	KEY_4				= 52,	///< Top row <tt>4</tt>
	KEY_5				= 53,	///< Top row <tt>5</tt>
	KEY_6				= 54,	///< Top row <tt>6</tt>
	KEY_7				= 55,	///< Top row <tt>7</tt>
	KEY_8				= 56,	///< Top row <tt>8</tt>
	KEY_9				= 57,	///< Top row <tt>9</tt>
	KEY_SEMICOLON		= 59,	///< <tt>;</tt>
	KEY_EQUAL			= 61,	///< <tt>=</tt>
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
	KEY_LEFT_BRACKET	= 91,	///< <tt>[</tt>
	KEY_BACKSLASH		= 92,	///< <tt>\\</tt> back slash
	KEY_RIGHT_BRACKET	= 93,	///< <tt>]</tt>
	KEY_GRAVE_ACCENT	= 96,	///< <tt>`</tt>
	KEY_WORLD_1			= 161,	///< non-US #1
	KEY_WORLD_2			= 162,	///< non-US #2

	KEY_ESCAPE			= 256,	///< <tt>Esc</tt>
	KEY_ENTER			= 257,	///< <tt>Enter</tt>, Underneath backspace
	KEY_TAB				= 258,	///< <tt>Tab</tt>
	KEY_BACKSPACE		= 259,	///< <tt>Backspace</tt>
	KEY_INSERT			= 260,	///< <tt>Insert</tt>
	KEY_DELETE			= 261,	///< <tt>Delete</tt>
	KEY_RIGHT			= 262,	///< Right arrow key
	KEY_LEFT			= 263,	///< Left arrow key
	KEY_DOWN			= 264,	///< Down arrow key
	KEY_UP				= 265,	///< Up arrow key
	KEY_PAGE_UP			= 266,	///< <tt>Page up</tt>
	KEY_PAGE_DOWN		= 267,	///< <tt>Page down</tt>
	KEY_HOME			= 268,	///< <tt>Home</tt>
	KEY_END				= 269,	///< <tt>End</tt>
	KEY_CAPS_LOCK		= 280,	///< <tt>Caps lock</tt>
	KEY_SCROLL_LOCK		= 281,	///< <tt>Scroll lock</tt>
	KEY_NUM_LOCK		= 282,	///< <tt>Num lock</tt>
	KEY_PRINT_SCREEN	= 283,	///< <tt>Print screen</tt>
	KEY_PAUSE			= 284,	///< <tt>Pause</tt>
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
	KEY_NUMPAD_0		= 320,
	KEY_NUMPAD_1		= 321,
	KEY_NUMPAD_2		= 322,
	KEY_NUMPAD_3		= 323,
	KEY_NUMPAD_4		= 324,
	KEY_NUMPAD_5		= 325,
	KEY_NUMPAD_6		= 326,
	KEY_NUMPAD_7		= 327,
	KEY_NUMPAD_8		= 328,
	KEY_NUMPAD_9		= 329,
	KEY_NUMPAD_DECIMAL	= 330,	///< Numpad <tt>.</tt> or <tt>,</tt> depending on region
	KEY_NUMPAD_DIVIDE	= 331,	///< Numpad <tt>/</tt>
	KEY_NUMPAD_MULTIPLY	= 332,	///< Numpad <tt>*</tt>
	KEY_NUMPAD_SUBTRACT	= 333,	///< Numpad <tt>-</tt>
	KEY_NUMPAD_ADD		= 334,	///< Numpad <tt>+</tt>
	KEY_NUMPAD_ENTER	= 335,	///< Numpad <tt>Enter</tt>
	KEY_NUMPAD_EQUAL	= 336,	///< Numpad <tt>=</tt> (often missing)
	KEY_LEFT_SHIFT		= 340,	///< Left <tt>Shift</tt>
	KEY_LEFT_CONTROL	= 341,	///< Left <tt>Ctrl</tt>
	KEY_LEFT_ALT		= 342,	///< Left <tt>Alt</tt>
	KEY_LEFT_SUPER		= 343,	///< Left Super/Windows key 
	KEY_RIGHT_SHIFT		= 344,	///< Right <tt>Shift</tt>
	KEY_RIGHT_CONTROL	= 345,	///< Right <tt>Ctrl</tt>
	KEY_RIGHT_ALT		= 346,	///< Right <tt>Alt</tt>
	KEY_RIGHT_SUPER		= 347,	///< Right Super/Windows key
	KEY_MENU			= 348,	///< Menu

	KEY_LAST			= KEY_MENU,	///< Used to get the number of total key entries.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Parameters to construct a Window.
struct WindowCreateInfo {

	/// @brief		Is window resizeable.
	bool						resizeable					= true;

	/// @brief		Is the window visible after it's been created.
	bool						visible						= true;

	/// @brief		Does the window have default OS borders and buttons.
	///
	///				If set to false, only window contents are displayed, no borders, minimize/maximize/close buttons.
	bool						decorated					= true;

	/// @brief		Is the window focused and brought forth by the OS when created.
	bool						focused						= true;

	/// @brief		Is the window maximized to fill the screen when created.
	bool						maximized					= false;

	/// @brief		Is the alpha value of the render interpreted as a transparent window background.
	bool						transparent_framebuffer		= false;

	/// @brief		Coordinate system to be used.
	///
	/// @see		RenderCoordinateSpace
	RenderCoordinateSpace		coordinate_space			= RenderCoordinateSpace::TEXEL_SPACE;

	/// @brief		Window content initial size in pixels.
	glm::uvec2					size						= { 800, 600 };

	/// @brief		Minimum size of the window.
	///
	///				This also works when resizing with the mouse. Your OS may have an absolute minimum size which is used as a
	///				limit of how small of a window you can create.
	glm::uvec2					min_size					= { 32, 32 };

	/// @brief		Maximum size of the window.
	///
	///				This also works when resizing with the mouse. Your OS may have an absolute maximum size which is used as a
	///				limit of how big of a window you can create.
	glm::uvec2					max_size					= { UINT32_MAX, UINT32_MAX };

	/// @brief		Fullscreen monitor.
	///
	///				nullptr is windowed, use Instance::GetPrimaryMonitor() to use primary monitor for fullscreen, or use
	///				Instance::GetMonitors to choose which monitor you wish display this window in fullscreen mode.
	Monitor					*	fullscreen_monitor			= {};

	/// @brief		Refresh rate in fullscreen mode, UINT32_MAX is unlimited.
	uint32_t					fullscreen_refresh_rate		= UINT32_MAX;

	/// @brief		Vertical synchronization.
	///
	///				This works in both windowed and fullscreen modes, 2d graphics are rarely demanding so you should always
	///				leave this on.
	///
	/// @warning	Disabling vsync in 2D graphics has caused certain GPUs to overheat, damaging hardware. Drawing 2D is
	///				usually not too demanding, by not limiting the speed, you may stress the GPU a lot more than usual
	///				which in rare cases may cause physical damage.
	bool						vsync						= true;

	/// @brief		Multisampling.
	///
	///				Must be a single value from Multisamples. Uses more GPU resources if higher than 1 but gets rid of
	///				jagged edges of polygons, the higher the samples, the smoother the edge and higher the demand of the GPU.
	Multisamples				samples						= Multisamples::SAMPLE_COUNT_1;

	/// @brief		Window title text.
	std::string					title						= "";

	/// @brief		Pointer to an event handler.
	///
	///				Allows capturing keyboard and mouse events that happened to this window.
	///
	/// @see		WindowEventHandler
	WindowEventHandler		*	event_handler				= nullptr;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Represents an OS window and its render surface.
class Window {
	friend class vk2d_internal::InstanceImpl;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Constructor. This object should not be directly constructed
	///
	///				Window is created with Instance::CreateOutputWindow().
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	instance
	///				A pointer to instance that owns this window.
	/// 
	/// @param[in]	window_create_info
	///				Window creation parameters.
	VK2D_API Window(
		vk2d_internal::InstanceImpl				&	instance,
		const WindowCreateInfo					&	window_create_info
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @note		Multithreading: Main thread only.
	VK2D_API ~Window();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Signal that the window should now close.
	///
	///				This function does not actually close the window but rather just sets a flag that it should close, the main
	///				program will have to manually remove the window from Instance with Instance::DestroyOutputWindow().
	///
	/// @note		Multithreading: Main thread only.
	VK2D_API void									CloseWindow();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the window wants to close.
	///
	///				Used to check if window should be closed, if the user presses the close button of the window in OS, then every
	///				to this function afterwards will return true.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if window close is requested, false if window close has not been requestd.
	VK2D_API bool									ShouldClose();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Takes a screenshot of the next image that will be rendered and saves it into a file.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	save_path
	///				Path where the file will be saved. Extension will determine the file format used. Supported file formats are:
	///				<br>
	///				<table>
	///				<tr><th> Format												<th> Extension
	///				<tr><td> Portable network graphics (RGBA) or (RGB)			<td> <tt>.png</tt>
	///				<tr><td> Microsoft Windows Bitmap 24-bit (BGR) non-RLE		<td> <tt>.bmp</tt>
	///				<tr><td> Truevision Targa (RGBA) or (RGB) RLE compressed	<td> <tt>.tga</tt>
	///				<tr><td> JPEG (RGB)											<td> <tt>.jpg</tt> or <tt>.jpeg</tt>
	///				</table>
	///				<br>
	///				Unsupported or unknown extensions will be saved as PNG.
	/// 
	/// @param[in]	include_alpha
	///				Include transparency of the scene in the saved image. true if you want transparency channel included, false
	///				otherwise. Note that not all formats can save transparency, in which case this parameter is ignored.
	VK2D_API void									TakeScreenshotToFile(
		const std::filesystem::path				&	save_path,
		bool										include_alpha				= false
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Takes a screenshot of the next image that will be rendered and saves it into a memory.
	///
	///				Calls an event callback to give the data to the application.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	include_alpha
	///				Include transparency of the scene in the saved image. true if you want transparency channel included, false if
	///				you want transparency channel set to opaque.
	VK2D_API void									TakeScreenshotToData(
		bool										include_alpha
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets focus to this window.
	///
	/// @note		You should call this function before entering fullscreen mode from windowed mode.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API void									Focus();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set window opacity.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	opacity
	///				Value between 0.0 and 1.0 where 0.0 is completely transparent and 1.0 is completely opaque.
	VK2D_API void									SetOpacity(
		float										opacity
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the current opacity of this window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Current opacity value between 0.0 and 1.0 where 0.0 is completely transparent and 1.0 is completely opaque.
	VK2D_API float									GetOpacity();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Hides or un-hides the window.
	///
	/// @note		Hidden windows do not receive user input.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	hidden
	///				true to hide the window, false to un-hide.
	VK2D_API void									Hide(
		bool										hidden
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the hidden status of the window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if window is hidden, false if it's visible.
	VK2D_API bool									IsHidden();

	/// @brief		Disable or enable all events for a specific window.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	disable_events
	///				true to disable all events, false to enable all events.
	VK2D_API void									DisableEvents(
		bool										disable_events
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if events are enabled or disabled.
	/// 
	/// @see		Window::DisableEvents()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if events are disabled, false if events are enabled.
	VK2D_API bool									AreEventsDisabled();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Enter fullscreen or windowed mode.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	monitor
	///				A pointer to a monitor where you wish to display the contents of this window in fullscreen mode.
	///				<tt>nullptr</tt> if you wish to enter windowed mode.
	/// 
	/// @param[in]	frequency
	///				Target refresh rate of the monitor when entering fullscreen mode. Ignored when entering windowed mode.
	VK2D_API void									SetFullscreen(
		Monitor									*	monitor,
		uint32_t									frequency
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if we're in fullscreen or windowed mode.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if this window has entered fullscreen mode on any monitor, false if windowed.
	VK2D_API bool									IsFullscreen();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get cursor position on window surface without using events.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		The cursor position on window surface in texels where 0*0 coordinate is top left corner of the window.
	VK2D_API glm::dvec2								GetCursorPosition();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set cursor position in relation to this window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	new_position
	///				New mouse cursor position. This changes the position of the OS or the "hardware" cursor.
	VK2D_API void									SetCursorPosition(
		glm::dvec2									new_position
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the OS or "hardware" cursor image to something else.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	cursor
	///				A pointer to cursor object to use from now on while the OS cursor is located inside this window.
	VK2D_API void									SetCursor(
		Cursor									*	cursor
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get last contents of the OS clipboard if it's a string.
	///
	///				Allows you to copy from another application and paste it right inside yours.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Last clipboard contents if it was text.
	VK2D_API std::string							GetClipboardString();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set OS clipboard last entry to some string.
	///
	///				Allows you to copy text from your application paste it another application.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	str
	///				Text to send to the OS clipboard.
	VK2D_API void									SetClipboardString(
		const std::string						&	str
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set window title that shows up on the title bar of the window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	title
	///				New title text of the window.
	VK2D_API void									SetTitle(
		const std::string						&	title
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets title of the window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Current title of the window.
	VK2D_API std::string							GetTitle();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set window icon that shows up in OS taskbar/toolbar when the application is running.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param		image_paths
	///				Path to image to use. Supported formats are: <br>
	///				<table>
	///					<tr>
	///						<th>Format</th>	<th>Support level</th>
	///					</tr>
	///					<tr>
	///						<td>JPEG</td>	<td>baseline & progressive supported, 12 bits-per-channel/arithmetic not supported</td>
	///					</tr>
	///					<tr>
	///						<td>PNG</td>	<td>1/2/4/8/16 bit-per-channel supported</td>
	///					</tr>
	///					<tr>
	///						<td>TGA</td>	<td>not sure what subset, if a subset</td>
	///					</tr>
	///					<tr>
	///						<td>BMP</td>	<td>no support for 1bpp or RLE</td>
	///					</tr>
	///					<tr>
	///						<td>PSD</td>	<td>composited view only, no extra channels, 8/16 bits-per-channel</td>
	///					</tr>
	///					<tr>
	///						<td>PIC</td>	<td>Softimage PIC</td>
	///					</tr>
	///					<tr>
	///						<td>PNM</td>	<td>PPM and PGM binary only</td>
	///					</tr>
	///				</table>
	VK2D_API void									SetIcon(
		const std::vector<std::filesystem::path>&	image_paths
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets window position on the virtual screen space.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	new_position
	///				New position in the virtual monitor space. Virtual in this case means that if the user has 2 or more monitor
	///				setup and the desktop is set to be continuous from one monitor to the next, the window coordinates determine in
	///				which monitor the window will appear, this depends on the user's monitor setup however.
	VK2D_API void									SetPosition(
		glm::ivec2									new_position
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get window current position on the virtual screen space.
	/// 
	/// @note		Multithreading: Main thread only.
	///
	/// @see		Window::SetPosition()
	/// 
	/// @return		Position of the window in the virtual monitor space.
	VK2D_API glm::ivec2								GetPosition();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set size of the window.
	///
	///				More specifically this sets the framebuffer size or the content size of the window to a new size, window
	///				decorators are extended to fit the new content size.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	new_size
	///				New size of the window.
	VK2D_API void									SetSize(
		glm::uvec2									new_size
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get content/framebuffer size of the window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Size of the window.
	VK2D_API glm::uvec2								GetSize();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Iconifies the window to the taskbar or restores it back into a window.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	minimized
	///				true if window should be iconified, false if restored from iconified state.
	VK2D_API void									Iconify(
		bool										minimized
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the window is currently iconified.
	/// 
	/// @see		Window::Iconify()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if window is iconified to the task/tool-bar, false if window is fully visible.
	VK2D_API bool									IsIconified();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the window to be maximized or normal.
	///
	///				When maximized the window will be sized to fill the entire workable space of the desktop monitor so that the
	///				window edges, top bar and the OS task/tool-bar is not covered.
	///
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	maximized
	///				true if you wish to maximize the window, false if you want floating window.
	VK2D_API void									SetMaximized(
		bool										maximized
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the maximized status.
	/// 
	/// @see		Window::SetMaximized()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if the window is maximized, false if it's floating.
	VK2D_API bool									GetMaximized();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set the cursor to be visible, invisible or constrained inside the window.
	/// 
	/// @see		CursorState
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	new_state
	///				New state of the cursor to be used from now on.
	VK2D_API void									SetCursorState(
		CursorState									new_state
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Returns the current state of the cursor, either normal, hidden, or constrained.
	/// 
	/// @see		CursorState
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Current state of the cursor.
	VK2D_API CursorState							GetCursorState();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Begins the render operations.
	///
	///				This signals the beginning of the render block and you must call this before using any drawing commands. For
	///				best performance you should calculate game logic first, when you're ready to draw call this function just before
	///				your first draw command. Every draw call must be between this and Window::EndRender().
	///
	///				Calling this will wait for the previous frame to finish rendering, to maximize the asynchronous usage of the CPU
	///				and the GPU, you should call this function as late as possible in your application.
	/// 
	/// @see		Window::EndRender()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if operation was successful, false on error and if you should quit.
	VK2D_API bool									BeginRender();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Ends the rendering operations.
	///
	///				This signals the end of the render block and you must call this after you're done drawing everything in order to
	///				display the results on the window surface.
	///
	///				This will actually send everything to the GPU to be rendered. This function returns as soon as the work has been
	///				submitted to the GPU so rendering will be done by the GPU asynchronously to your application. The completed
	///				render will automatically be displayed on the window surface as soon as the GPU finishes with the frame.
	/// 
	/// @see		Window::BeginRender()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if operation was successful, false on error and if you should quit.
	VK2D_API bool									EndRender();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Switch render coordinate space.
	///
	///				Can be set at any time, but will not take effect until starting next frame.
	/// 
	/// @param[in]	coordinate_space
	///				Coordinate space to switch to.
	VK2D_API void									SetRenderCoordinateSpace(
		RenderCoordinateSpace						coordinate_space
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws points directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawPointList(
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations				= {},
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr
	)
	{
		DrawPointList(
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			texture,
			sampler
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws lines directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	indices
	///				List of indices telling how to form lines between vertices.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawLineList(
		std::span<const VertexIndex_2>				indices,
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations				= {},
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr,
		float										line_width					= 1.0f
	)
	{
		auto indices_span = std::span( reinterpret_cast<const uint32_t*>( indices.data() ), indices.size() * 2 );

		DrawLineList(
			indices_span,
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			texture,
			sampler
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw triangles directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	indices
	///				List of indices telling how to form lines between vertices.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawTriangleList(
		std::span<const VertexIndex_3>				indices,
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations				= {},
		bool										filled						= true,
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr
	)
	{
		auto indices_span = std::span( reinterpret_cast<const uint32_t*>( indices.data() ), indices.size() * 3 );

		DrawTriangleList(
			indices_span,
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			filled,
			texture,
			sampler
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws an individual point.
	/// 
	///				Inefficient, for when you really just need a single point drawn without extra information. As soon as you need
	///				to draw 2 or more points, use Window::DrawPointList() instead.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	location
	///				Where to draw the point to, depends on the coordinate system. See RenderCoordinateSpace for more info.
	/// 
	/// @param[in]	color
	///				Color of the point to be drawn.
	/// 
	/// @param[in]	size
	///				Size of the point to be drawn, sizes larger than 1.0f will appear as a rectangle.
	VK2D_API void									DrawPoint(
		glm::vec2									location,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float										size						= 1.0f
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws an individual line.
	/// 
	///				Inefficient, for when you really just need a single line drawn without extra information. As soon as you need to
	///				draw 2 or more lines, use Window::DrawLineList() instead.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	point_1
	///				Coordinates of the starting point of the line, depends on the coordinate system. See RenderCoordinateSpace for more info.
	/// 
	/// @param[in]	point_2
	///				Coordinates of the ending point of the line, depends on the coordinate system. See RenderCoordinateSpace for more info.
	/// 
	/// @param[in]	color
	///				Color of the line to be drawn.
	VK2D_API void									DrawLine(
		glm::vec2									point_1,
		glm::vec2									point_2,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float										line_width					= 1.0f
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws a rectangle.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Area of the rectangle that will be covered, depends on the coordinate system. See RenderCoordinateSpace
	///				for more info about what scale is used.
	/// 
	/// @param[in]	filled
	///				true if the inside of the rectangle is drawn, false for the outline only.
	/// 
	/// @param[in]	color
	///				Color of the rectangle to be drawn.
	VK2D_API void									DrawRectangle(
		Rect2f										area,
		bool										filled						= true,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws an ellipse or a circle.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
	///				used.
	/// 
	/// @param[in]	filled
	///				true to draw the inside of the ellipse/circle, false to draw the outline only.
	/// 
	/// @param[in]	edge_count
	///				How many corners this ellipse should have, or quality if you prefer. This is a float value for "smoother"
	///				transitions between amount of corners, in case this value is animated.
	/// 
	/// @param[in]	color
	///				Color of the ellipse/circle to be drawn.
	VK2D_API void									DrawEllipse(
		Rect2f										area,
		bool										filled						= true,
		float										edge_count					= 64.0f,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws an ellipse or a circle that has a "slice" cut out, similar to pie graphs.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
	///				be used.
	/// 
	/// @param[in]	begin_angle_radians
	///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
	/// 
	/// @param[in]	coverage
	///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full ellipse. Moving value
	///				from 0 to 1 makes "whole" pie visible in clockwise direction.
	/// 
	/// @param[in]	filled
	///				true to draw the inside of the pie, false to draw the outline only.
	/// 
	/// @param[in]	edge_count 
	///				How many corners the complete ellipse should have, or quality if you prefer. This is a float value for
	///				"smoother" transitions between amount of corners, in case this value is animated.
	/// 
	/// @param[in]	color 
	///				Color of the pie to be drawn.
	VK2D_API void									DrawEllipsePie(
		Rect2f										area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled						= true,
		float										edge_count					= 64.0f,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a rectangular pie, similar to drawing a rectangle but which has a pie slice cut out.
	/// 
	/// @note		Multithreading: Main thread only.
	///
	/// @param[in]	area
	///				Area of the rectangle to be drawn. See RenderCoordinateSpace for more info about what scale is used.
	/// 
	/// @param[in]	begin_angle_radians
	///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
	/// 
	/// @param[in]	coverage 
	///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full rectangle. Moving value
	///				from 0 to 1 makes "whole" pie visible in clockwise direction.
	/// 
	/// @param[in]	filled
	///				true to draw the inside of the pie rectangle, false to draw the outline only.
	/// 
	/// @param[in]	color 
	///				Color of the pie rectangle to be drawn.
	VK2D_API void									DrawRectanglePie(
		Rect2f										area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled						= true,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws a rectangle with texture and use the size of the texture to determine size of the rectangle.
	/// 
	/// @warning	!! If window surface coordinate space is normalized, this will not work properly as bottom right coordinates are
	///				offsetted by the texture size !! See RenderCoordinateSpace for more info.
	/// 
	/// @param[in]	location
	///				Draw location of the texture, this is the top left corner of the texture, depends on the coordinate system. See
	///				RenderCoordinateSpace for more info.
	/// 
	/// @param[in]	texture
	///				Texture to draw.
	/// 
	/// @param[in]	color 
	///				Color multiplier of the texture texel color, eg. If Red color is 0.0 then texture will lack all Red color, or if
	///				alpha channel of the color is 0.5 then texture appears half transparent.
	VK2D_API void									DrawTexture(
		glm::vec2									location,
		Texture									*	texture,
		Colorf										color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	/// 
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	///
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw. Using multiple
	///				transformations results the mesh being drawn multiple times using different transformations. This is also called
	///				instanced rendering.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		std::span<const glm::mat4>					transformations
	)
	{
		if( mesh.vertices.size() == 0 ) return;
		auto raw_vertices = vk2d_internal::RawVertexData( std::span<const VertexT>( mesh.vertices ) );

		switch( mesh.mesh_type ) {
		case MeshType::TRIANGLE_FILLED:
			DrawTriangleList(
				mesh.indices,
				raw_vertices,
				transformations,
				true,
				mesh.texture,
				mesh.sampler
			);
			break;
		case MeshType::TRIANGLE_WIREFRAME:
			DrawTriangleList(
				mesh.indices,
				raw_vertices,
				transformations,
				false,
				mesh.texture,
				mesh.sampler
			);
			break;
		case MeshType::LINE:
			DrawLineList(
				mesh.indices,
				raw_vertices,
				transformations,
				mesh.texture,
				mesh.sampler,
				mesh.line_width
			);
			break;
		case MeshType::POINT:
			DrawPointList(
				raw_vertices,
				transformations,
				mesh.texture,
				mesh.sampler
			);
			break;
		default:
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	///
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	///
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformation
	///				Draw using transformation.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		const Transform							&	transformation = {}
	)
	{
		auto transformation_matrix = transformation.CalculateTransformationMatrix();

		DrawMesh(
			mesh,
			std::span( &transformation_matrix, 1 )
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	/// 
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw. Using multiple
	///				transformations results the mesh being drawn multiple times using different transformations. This is also called
	///				instanced rendering.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		std::span<const Transform>					transformations
	)
	{
		std::vector<glm::mat4> transformation_matrices( std::size( transformations ) );
		for( size_t i = 0; i < std::size( transformations ); ++i ) {
			transformation_matrices[ i ] = transformations[ i ].CalculateTransformationMatrix();
		}

		DrawMesh(
			mesh,
			transformation_matrices
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool									IsGood() const;


private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawPointList(
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations				= {},
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawLineList(
		std::span<const uint32_t>					indices,
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations				= {},
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr,
		float										line_width					= 1.0f
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawTriangleList(
		std::span<const uint32_t>					indices,
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations				= {},
		bool										filled						= true,
		Texture									*	texture						= nullptr,
		Sampler									*	sampler						= nullptr
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::WindowImpl>		impl;
};







/// @brief		Window event handler base class. You can override member methods to receive keyboard, mouse, gamepad, other... events. <br>
///				Example event handler:
/// @code		
///				class MyEventHandler : public WindowEventHandler
///				{
///				public:
///					// Keyboard button was pressed, released or kept down ( repeating ).
///					void						EventKeyboard(
///						Window				&	window,
///						KeyboardButton			button,
///						int32_t					scancode,
///						ButtonAction			action,
///						ModifierKeyFlags		modifierKeys
///					) override
///					{};
///				};
///	@endcode
///	@note		You must use when overriding events.
class WindowEventHandler {
public:
	/// @brief		Window position changed.
	/// @param[in]	window
	///				Which window's position changed.
	/// @param[in]	position
	///				Where the window moved to.
	virtual void								EventWindowPosition(
		Window								&	window,
		glm::ivec2								position
	)
	{};

	/// @brief		Window size changed.
	/// @param[in]	window
	///				Which window's position changed.
	/// @param[in]	size
	///				what's the new size of the window.
	virtual void								EventWindowSize(
		Window								&	window,
		glm::uvec2								size
	)
	{};

	/// @brief		Window wants to close when this event runs, either the user pressed the "X", the OS
	///				wants to close the window, user pressed Alt+F4...
	///				This event is not called when the window is actually closed, only when the window should be. <br>
	///				Default behavior is: @code window->CloseWindow(); @endcode
	///				If you override this function, you'll have to handle closing the window yourself.
	/// @param[in]	window
	///				Window that should be closed.
	virtual void								EventWindowClose(
		Window								&	window
	)
	{
		window.CloseWindow();
	};

	/// @brief		Window refreshed itself. <br>
	///				Not that useful nowadays as modern OSes don't use CPU to draw the windows anymore,
	///				this doesn't trigger often if at all. Might remove this event later.
	/// @param[in]	window
	///				Window that refreshed itself.
	virtual void								EventWindowRefresh(
		Window								&	window
	)
	{};

	/// @brief		Window gained or lost focus. Ie. Became topmost window, or lost the topmost position.
	/// @param[in]	window
	///				Window that became topmost or lost it's position.
	/// @param[in]	focused
	///				true if the window became topmost, false if it lost the topmost position.
	virtual void								EventWindowFocus(
		Window								&	window,
		bool									focused
	)
	{};

	/// @brief		Window was iconified to the taskbar or recovered from there.
	/// @param[in]	window
	///				Window that was iconified or recovered.
	/// @param[in]	iconified
	///				true if the window was iconified, false if recovered from taskbar.
	virtual void								EventWindowIconify(
		Window								&	window,
		bool									iconified
	)
	{};

	/// @brief		Window was maximized or recovered from maximized state.
	/// @param[in]	window
	///				Window that was maximized or recovered from maximized state.
	/// @param[in]	maximized
	///				true if maximized or false if recevered from maximized state.
	virtual void								EventWindowMaximize(
		Window								&	window,
		bool									maximized
	)
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
	virtual void								EventMouseButton(
		Window								&	window,
		MouseButton								button,
		ButtonAction							action,
		ModifierKeyFlags						modifier_keys
	)
	{};

	/// @brief		Mouse moved to a new position on the window.
	/// @param[in]	window
	///				Which window the mouse movement happened in.
	/// @param[in]	position
	///				Tells the new mouse position.
	virtual void								EventCursorPosition(
		Window								&	window,
		glm::dvec2								position
	)
	{};

	/// @brief		Mouse cursor moved on top of the window area, or left it.
	/// @param[in]	window
	///				Which window the mouse cursor entered.
	/// @param[in]	entered
	///				true if entered, false if cursor left the window area.
	virtual void								EventCursorEnter(
		Window								&	window,
		bool									entered
	)
	{};

	/// @brief		Mouse wheel was scrolled.
	/// @param[in]	window
	///				Which window the scrolling happened in.
	/// @param[in]	scroll
	///				Scroll direction vector telling what changed since last event handling.
	///				This is a 2d vector because some mice have sideways scrolling. Normal vertical
	///				scrolling is reported in the Y axis, sideways movement in the X axis.
	virtual void								EventScroll(
		Window								&	window,
		glm::ivec2								scroll
	)
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
	virtual void								EventKeyboard(
		Window								&	window,
		KeyboardButton							button,
		int32_t									scancode,
		ButtonAction							action,
		ModifierKeyFlags						modifier_keys
	)
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
	virtual void								EventCharacter(
		Window								&	window,
		uint32_t								character,
		ModifierKeyFlags						modifier_keys
	)
	{};


	/// @brief		File was drag-dropped onto the window.
	/// @param[in]	window
	///				Window where files were dragged and dropped onto.
	/// @param[in]	files
	///				List of file paths.
	virtual void								EventFileDrop(
		Window								&	window,
		std::vector<std::filesystem::path>		files
	)
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
	virtual void								EventScreenshot(
		Window								&	window,
		const std::filesystem::path			&	screenshot_path,
		const ImageData						&	screenshot_data,
		bool									success,
		const std::string					&	error_message
	)
	{};
};





}
