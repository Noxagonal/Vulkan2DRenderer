#pragma once

#include "SourceCommon.h"
#include "RendererImpl.h"

namespace vk2d {

namespace _internal {



class ResourceManagerImpl {
public:
	ResourceManagerImpl(
		_internal::RendererImpl			*	parent_renderer
	);
	~ResourceManagerImpl();

	bool									IsGood();

private:
	bool									is_good						= {};
};



} // _internal



}

