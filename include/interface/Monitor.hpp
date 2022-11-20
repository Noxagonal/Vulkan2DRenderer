#pragma once

#include <core/Common.hpp>

#include <vector>
#include <memory>
#include <span>



namespace vk2d {
namespace vk2d_internal {

class MonitorImpl;
class WindowImpl;

void UpdateMonitorLists( bool globals_locked );

} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Video mode the monitor can natively work in.
struct MonitorVideoMode
{
	glm::uvec2			resolution;
	uint32_t			red_bit_count;
	uint32_t			green_bit_count;
	uint32_t			blue_bit_count;
	uint32_t			refresh_rate;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Gamma ramp for manual gamma adjustment on the monitor at different intensity levels per color.
/// 
///				Ramp is made out of nodes that are evenly spaced from lowest to highest value. Input must have at least 2 nodes,
///				values are linearly interpolated inbetween nodes to fill the entire range. This gamma ramp is applied in
///				addition to the hardware or OS gamma correction (usually approximation of sRGB gamma) so setting a linear gamma
///				ramp will result in already gamma corrected image.
struct GammaRampNode
{
	float	red;
	float	green;
	float	blue;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Holds information about a physical monitor.
class Monitor
{
	friend class Window;
	friend class vk2d_internal::WindowImpl;
	friend void vk2d_internal::UpdateMonitorLists( bool globals_locked );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Constructor. This object should not be directly constructed.
	///
	///				This object should not be directly constructed, it is created and destroyed automatically by VK2D whenever a new
	///				instance is created or if monitor is connected or disconnected while the application is running.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	preconstructed_impl
	///				This is the actual implementation details given to this interface by VK2D.
	VK2D_API Monitor(
		std::unique_ptr<vk2d_internal::MonitorImpl>	&&	preconstructed_impl
	);

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Monitor constructor for a null monitor.
	///
	///				needed for default initialization.
	/// 
	///				This object should not be directly constructed, it is created and destroyed automatically by VK2D whenever a new
	///				instance is created or if monitor is connected or disconnected while the application is running.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Monitor();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Copy constructor.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Monitor(
		const Monitor									&	other
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Move constructor.
	/// 
	/// @note		Multithreading: Main thread only.
	VK2D_API Monitor(
		Monitor											&&	other
	);

	/// @note		Multithreading: Main thread only.
	VK2D_API ~Monitor();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get current video mode of this monitor.
	///
	///				Get resolution, bits per color and refresh rate.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Current monitor video mode.
	VK2D_API MonitorVideoMode							GetCurrentVideoMode() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get all video modes supported by the monitor.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		A list of video modes supported by the monitor.
	VK2D_API std::vector<MonitorVideoMode>				GetVideoModes() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set monitor gamma.
	///
	///				Automatically generates a gamma ramp from this value and uses it to set the gamma. This value is in addition to
	///				the hardware or OS gamma correction value so 1.0 (linear) is considered already gamma corrected.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	gamma
	///				Value greater than 0.0. Default/original gamma value is 1.0 which produces linear gamma.
	VK2D_API void										SetGamma(
		float											gamma
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get monitor gamma ramp.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		GammaRampNode
	/// 
	/// @return		A list of gamma ramp nodes at equal spacing where first node is minimum brightness and last node is maximum
	///				brightness.
	VK2D_API std::vector<GammaRampNode>					GetGammaRamp();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set monitor gamma manually with gamma ramp.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		GammaRampNode
	/// 
	/// @param[in]	ramp
	///				A list of gamma nodes where all nodes are considered evenly spaced. First node is minimum brightness, last
	///				node is maximum brightness. Number of gamma ramp nodes must be 2 or more. Values inbetween nodes are
	///				automatically linearly interpolated so number of nodes only effects quality of the gamma ramp.
	VK2D_API void										SetGammaRamp(
		std::span<const GammaRampNode>					ramp
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API Monitor								&	operator=(
		const Monitor								&	other
		);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API Monitor								&	operator=(
		Monitor										&&	other
		);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
		/// 
		/// @note		Multithreading: Any thread.
		/// 
		/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool										IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::MonitorImpl>			impl;
};



} // vk2d
