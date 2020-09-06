
#include "../Header/Core/SourceCommon.h"
#include "../Header/Impl/TextureImpl.h"

vk2d::_internal::TextureImpl::TextureImpl(
	vk2d::_internal::InstanceImpl		*	instance
)
{
	is_good = true;
}

vk2d::_internal::TextureImpl::~TextureImpl()
{}

bool vk2d::_internal::TextureImpl::IsGood()
{
	return is_good;
}
