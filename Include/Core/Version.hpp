#pragma once

#include "Core/Common.h"

#include <initializer_list>
#include <array>
#include <assert.h>

namespace vk2d {

class Version {
public:
	inline					Version()
	{};

	inline					Version(
		uint32_t			major,
		uint32_t			minor,
		uint32_t			patch
	) :
		major( major ),
		minor( minor ),
		patch( patch )
	{};

	inline					Version(
		std::array<uint32_t, 3>		init_list
	)
	{
		major				= init_list[ 0 ];
		minor				= init_list[ 1 ];
		patch				= init_list[ 2 ];
	}

	inline uint32_t			ToVulkanVersion()
	{
		return ( ( ( major ) << 22 ) | ( ( minor ) << 12 ) | ( patch ) );
	}

	uint32_t				major			= 0;
	uint32_t				minor			= 0;
	uint32_t				patch			= 0;
};

}
