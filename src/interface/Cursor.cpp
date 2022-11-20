
#include <core/SourceCommon.h>

#include "CursorImpl.hpp"

#include "InstanceImpl.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor::Cursor(
	vk2d_internal::InstanceImpl		&	instance,
	const std::filesystem::path		&	image_path,
	glm::ivec2							hot_spot
)
{
	impl = std::make_unique<vk2d_internal::CursorImpl>(
		instance,
		image_path,
		hot_spot
		);
	if( !impl || !impl->IsGood() )
	{
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		impl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor::Cursor(
	vk2d_internal::InstanceImpl		&	instance,
	glm::uvec2							image_size,
	const std::vector<Color8>		&	image_data,
	glm::ivec2							hot_spot
)
{
	impl = std::make_unique<vk2d_internal::CursorImpl>(
		instance,
		image_size,
		image_data,
		hot_spot
		);
	if( !impl || !impl->IsGood() )
	{
		instance.Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		impl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor::Cursor(
	Cursor	&	other
)
{
	impl = std::make_unique<vk2d_internal::CursorImpl>(
		other.impl->GetInstance(),
		other.impl->GetSize(),
		other.impl->GetTexelData(),
		other.impl->GetHotSpot()
		);
	if( !impl || !impl->IsGood() )
	{
		other.impl->GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		impl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor::Cursor(
	Cursor	&&	other
)
{
	std::swap( impl, other.impl );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor::~Cursor()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor & vk2d::Cursor::operator=(
	Cursor	&	other )
{
	impl = std::make_unique<vk2d_internal::CursorImpl>(
		other.impl->GetInstance(),
		other.impl->GetSize(),
		other.impl->GetTexelData(),
		other.impl->GetHotSpot()
		);
	if( !impl || !impl->IsGood() )
	{
		other.impl->GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create cursor implementation!" );
		impl = nullptr;
	}

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Cursor & vk2d::Cursor::operator=(
	Cursor	&&	other
	)
{
	std::swap( impl, other.impl );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API glm::uvec2 vk2d::Cursor::GetSize()
{
	return impl->GetSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API glm::ivec2 vk2d::Cursor::GetHotSpot()
{
	return impl->GetHotSpot();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API std::vector<vk2d::Color8> vk2d::Cursor::GetTexelData()
{
	return impl->GetTexelData();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::Cursor::IsGood() const
{
	return !!impl;
}
