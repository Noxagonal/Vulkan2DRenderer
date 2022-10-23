#pragma once

#include "core/Common.h"

#include <initializer_list>
#include <array>
#include <assert.h>

namespace vk2d {

/// @brief		Class for managing version information.
class Version {
public:
	inline					Version()
	{};

	/// @brief		Construct with version information.
	/// 
	/// @param[in]	major
	///				Major version number.
	///
	/// @param[in]	minor
	///				Minor version number.
	/// 
	/// @param[in]	patch
	///				Patch version number.
	inline					Version(
		uint32_t			major,
		uint32_t			minor,
		uint32_t			patch
	) :
		major( major ),
		minor( minor ),
		patch( patch )
	{};

	/// @brief		Convert version into Vulkan version format.
	///
	///				Vulkan version is a packed 32 bit integer where the first 10 bits are major, the next 10 bits are minor
	///				and the last 12 bits are the patch number.
	/// 
	/// @return		Vulkan formatted version number.
	inline uint32_t			ToVulkanVersion()
	{
		return ( ( ( major ) << 22 ) | ( ( minor ) << 12 ) | ( patch ) );
	}

	uint32_t				major			= 0;
	uint32_t				minor			= 0;
	uint32_t				patch			= 0;
};

}
