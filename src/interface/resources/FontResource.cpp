
#include <core/SourceCommon.h>

#include <interface/resources/FontResource.h>
#include "FontResourceImpl.h"

#include "ResourceManagerImpl.h"

#include <interface/InstanceImpl.h>



VK2D_API vk2d::FontResource::FontResource(
	vk2d_internal::ResourceManagerImpl		&	resource_manager,
	uint32_t									loader_thread_index,
	ResourceBase							*	parent_resource,
	const std::filesystem::path				&	file_path,
	uint32_t									glyph_texel_size,
	bool										use_alpha,
	uint32_t									fallback_character,
	uint32_t									glyph_atlas_padding
)
{
	impl = std::make_unique<vk2d_internal::FontResourceImpl>(
		*this,
		resource_manager,
		loader_thread_index,
		parent_resource,
		file_path,
		glyph_texel_size,
		use_alpha,
		fallback_character,
		glyph_atlas_padding
	);
	if( !impl || !impl->IsGood() ) {
		impl = nullptr;
		resource_manager.GetInstance().Report( ReportSeverity::NON_CRITICAL_ERROR, "Internal error: Cannot create font resource implementation!" );
		return;
	}

	resource_impl = impl.get();
}

VK2D_API vk2d::FontResource::~FontResource()
{}

VK2D_API vk2d::ResourceStatus vk2d::FontResource::GetStatus()
{
	return impl->GetStatus();
}

VK2D_API vk2d::ResourceStatus vk2d::FontResource::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::ResourceStatus vk2d::FontResource::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

VK2D_API vk2d::Rect2f vk2d::FontResource::CalculateRenderedSize(
	std::string_view	text,
	float				kerning,
	glm::vec2			scale,
	bool				vertical,
	uint32_t			font_face,
	bool				wait_for_resource_load
)
{
	return impl->CalculateRenderedSize(
		text,
		kerning,
		scale,
		vertical,
		font_face,
		wait_for_resource_load
	);
}

VK2D_API bool vk2d::FontResource::IsGood() const
{
	return !!impl;
}
