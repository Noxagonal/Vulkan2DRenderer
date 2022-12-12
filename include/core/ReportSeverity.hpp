#pragma once

#include <core/Common.hpp>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Nature and severity of the reported action.
enum class ReportSeverity : uint32_t
{

	/// @brief		Not valid severity value, used to detect invalid severity values.
	NONE			= 0,

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

	/// @brief		Similar to critical error, this means the GPU crashed and we need to terminate immediately.
	DEVICE_LOST,

	/// @brief		This has the highest print priority but will not raise an error, used only with debug printouts.
	DEBUG,
};



} // vk2d
