
#include "../Header/Core/SourceCommon.h"
#include "../../Include/Interface/Texture.h"
#include "../Header/Impl/TextureImpl.h"
#include "../../Include/Interface/Instance.h"
#include "../Header/Impl/InstanceImpl.h"

VK2D_API vk2d::Texture::Texture(
	vk2d::_internal::InstanceImpl		*	instance
)
{
	impl		= std::unique_ptr<vk2d::_internal::TextureImpl>();
	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		is_good		= false;
		impl		= nullptr;
		instance->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create texture implementation!" );
	}
}

VK2D_API vk2d::Texture::~Texture()
{}
