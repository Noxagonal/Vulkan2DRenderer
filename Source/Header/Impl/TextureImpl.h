#pragma once

#include "../Core/SourceCommon.h"



namespace vk2d {
namespace _internal {



class InstanceImpl;



class TextureImpl
{
public:
	TextureImpl(
		vk2d::_internal::InstanceImpl		*	instance );
	~TextureImpl();

	bool																IsGood();

private:

	bool										is_good					= {};
};



} // _internal
} // vk2d
