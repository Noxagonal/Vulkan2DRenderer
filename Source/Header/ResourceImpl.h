#pragma once

#include "SourceCommon.h"

namespace vk2d {

namespace _internal {



class ResourceImpl {
public:
	virtual bool MTLoad()		= 0;
	virtual bool MTUnload()		= 0;
};



} // _internal

} // vk2d
