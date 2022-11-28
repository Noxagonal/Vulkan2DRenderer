#pragma once

#include <core/SourceCommon.hpp>

#include <interface/monitor/Monitor.hpp>



#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace vk2d {



class Window;
class Monitor;



namespace vk2d_internal {



class WindowImpl;



// Monitor object holds information about the physical monitor
class MonitorImpl
{
	friend class Window;
	friend class WindowImpl;
	friend class Monitor;

public:
	MonitorImpl(
		GLFWmonitor									*	monitor,
		VkOffset2D										position,
		VkExtent2D										physical_size,
		std::string										name,
		MonitorVideoMode								current_video_mode,
		const std::vector<MonitorVideoMode>			&	video_modes
	);

	MonitorImpl() = delete;

	MonitorImpl(
		const MonitorImpl							&	other
	) = default;

	MonitorImpl(
		MonitorImpl									&&	other
	) = default;

	~MonitorImpl() = default;

	const MonitorVideoMode							&	GetCurrentVideoMode() const;

	const std::vector<MonitorVideoMode>				&	GetVideoModes() const;

	void												SetGamma(
		float											gamma
	);

	std::vector<GammaRampNode>							GetGammaRamp();

	void												SetGammaRamp(
		std::span<const GammaRampNode>					ramp
	);

	MonitorImpl										&	operator=(
		const MonitorImpl							&	other
		) = default;

	MonitorImpl										&	operator=(
		MonitorImpl									&&	other
		) = default;

	bool												IsGood();

private:
	GLFWmonitor										*	monitor = {};
	VkOffset2D											position = {};
	VkExtent2D											physical_size = {};
	std::string											name = {};
	MonitorVideoMode									current_video_mode = {};
	std::vector<MonitorVideoMode>						video_modes;

	bool												is_good = {};
};



} // vk2d_internal
} // vk2d
