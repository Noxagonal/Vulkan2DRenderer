#pragma once

#include <core/Common.hpp>
#include <interface/resources/resource_base/ResourceHandleBase.hpp>
#include "FontResource.hpp"



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FontResourceHandle :
	public ResourceHandleBase<FontResource>
{
	using Base = ResourceHandleBase;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline FontResourceHandle() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline FontResourceHandle(
		FontResource				*	resource_ptr
	) :
		Base(
			resource_ptr
		)
	{}
};



} // vk2d