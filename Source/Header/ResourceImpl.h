#pragma once

namespace vk2d {

namespace _internal {



class ResourceImpl {
public:
	virtual bool Load()		= 0;
	virtual bool Unload()	= 0;
};



} // _internal

} // vk2d
