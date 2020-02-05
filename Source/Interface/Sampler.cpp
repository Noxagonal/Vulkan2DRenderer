
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/Sampler.h"
#include "../Header/Impl/SamplerImpl.h"
#include "../Header/Impl/InstanceImpl.h"

VK2D_API vk2d::Sampler::Sampler(
	vk2d::_internal::InstanceImpl			*	instance_parent,
	const vk2d::SamplerCreateInfo			&	create_info
)
{
	impl			= std::make_unique<vk2d::_internal::SamplerImpl>(
		this,
		instance_parent,
		create_info
	);

	if( impl && impl->IsGood() ) {
		is_good		= true;
	} else {
		impl		= nullptr;
		is_good		= false;
		instance_parent->Report( vk2d::ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create sampler implementation!" );
	}
}

VK2D_API vk2d::Sampler::~Sampler()
{}

VK2D_API bool VK2D_APIENTRY vk2d::Sampler::IsGood()
{
	return is_good;
}
