
#include "../Header/SourceCommon.h"

#include "../Header/ResourceManagerImpl.h"



namespace vk2d {

namespace _internal {





ResourceManagerImpl::ResourceManagerImpl( _internal::RendererImpl * parent_renderer )
{
	is_good		= true;
}

ResourceManagerImpl::~ResourceManagerImpl()
{}

bool ResourceManagerImpl::IsGood()
{
	return is_good;
}

} // _internal

} // vk2d
