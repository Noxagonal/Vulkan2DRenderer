#pragma once

#include <core/Common.h>

#include <containers/Color.hpp>

#include <vector>
#include <memory>



namespace vk2d {

namespace vk2d_internal {
class InstanceImpl;
class WindowImpl;
class CursorImpl;

void UpdateMonitorLists( bool globals_locked );
} // vk2d_internal



class Window;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		OS cursor image representation.
///
///				Mouse cursor is nothing more than an image that represents the location of the mouse on window, just like on the
///				desktop environment.
/// 
///				This cursor object is used to swap out the OS cursor image to another while the cursor is hovering over a
///				window. This is sometimes called "hardware" cursor in many applications. On "software" cursor implementations
///				your application would hide this type of cursor and implement it's own using a texture that's drawn at cursor
///				location inside the game loop. As "hardware" cursor is updated separately from your application it's usually a
///				good idea to use it in case your application has framerate issues or it locks up completely.
class Cursor
{
	friend class vk2d_internal::InstanceImpl;
	friend class Window;
	friend class vk2d_internal::WindowImpl;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Constructor. This object should not be directly constructed
	///
	///				Cursor is created by Instance::CreateCursor().
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	instance
	///				A reference to instance that owns this cursor.
	/// 
	/// @param[in]	image_path
	///				Path to an image file that will be used as a cursor.
	/// 
	/// @param[in]	hot_spot
	///				Hot spot is the texel offset from the top left corner of the image to where the actual pointing tip of the cursor
	///				is located. For example if the cursor is a 64*64 texel image of a circle where the circle is exactly centered and
	///				the center of the circle is the "tip" of the cursor, then the cursor's hot spot is 32*32.
	VK2D_API Cursor(
		vk2d_internal::InstanceImpl			&	instance,
		const std::filesystem::path			&	image_path,
		glm::ivec2								hot_spot
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Constructor. This object should not be directly constructed
	///
	///				Cursor is created with Instance::CreateCursor().
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	instance
	///				A reference to instance that owns this cursor.
	/// 
	/// @param[in]	image_size
	///				Size of the image in texels.
	/// 
	/// @param[in]	image_data
	///				Image texel data, from left to right, top to bottom order. Input size must be large enough to contain all
	///				texels at given texel size.
	/// 
	/// @param[in]	hot_spot
	///				Hot spot is the texel offset from the top left corner of the image to where the actual pointing tip of the
	///				cursor is located. For example if the cursor is a 64*64 texel image of a circle where the circle is exactly
	///				centered and the center of the circle is the "tip" of the cursor, then the cursor's hot spot is 32*32.
	VK2D_API Cursor(
		vk2d_internal::InstanceImpl			&	instance,
		glm::uvec2								image_size,
		const std::vector<Color8>			&	image_data,
		glm::ivec2								hot_spot
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Copy constructor.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Cursor(
		Cursor								&	other
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Move constructor.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Cursor(
		Cursor								&&	other
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @note		Multithreading: Main thread only.
	VK2D_API ~Cursor();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Copy operator.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Cursor							&	operator=(
		Cursor								&	other
		);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// @brief		Move operator.
		/// 
		/// @note		Multithreading: Main thread only.
	VK2D_API Cursor							&	operator=(
		Cursor								&&	other
		);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// @brief		Get cursor image texel size.
		/// 
		/// @note		Multithreading: Main thread only.
		/// 
		/// @return		Size of the cursor image in texels.
	VK2D_API glm::uvec2							GetSize();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get hot spot location in texels.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		The hot spot of the cursor image. Hot spot is the offset of the image to the "tip" of the cursor starting from
	///				top left of the image.
	VK2D_API glm::ivec2							GetHotSpot();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get texel data of the cursor image.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		A list of texels in left to right, top to bottom order. You will also need to use Cursor::GetSize() in
	///				order to correctly interpret the texels.
	VK2D_API std::vector<Color8>				GetTexelData();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool								IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::CursorImpl>	impl;
};



} // vk2d
