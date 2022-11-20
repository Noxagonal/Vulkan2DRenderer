
#include <core/SourceCommon.hpp>

#include <interface/resources/FontResource.hpp>
#include "FontResourceImpl.hpp"

#include <interface/resource_manager/ResourceManagerImpl.hpp>

#include <interface/instance/InstanceImpl.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::FontResource::~FontResource()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::FontResource::GetStatus()
{
	return impl->GetStatus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::FontResource::WaitUntilLoaded(
	std::chrono::nanoseconds				timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::ResourceStatus vk2d::FontResource::WaitUntilLoaded(
	std::chrono::steady_clock::time_point	timeout
)
{
	return impl->WaitUntilLoaded( timeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::Rect2f vk2d::FontResource::CalculateRenderedSize(
	std::string_view	text,
	size_t				font_face,
	float				kerning,
	glm::vec2			scale,
	bool				vertical,
	bool				wait_for_resource_load
)
{
	return impl->CalculateRenderedSize(
		text,
		font_face,
		kerning,
		scale,
		vertical,
		wait_for_resource_load
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::FontResource::GetFontFaceCount()
{
	return impl->GetFontFaceCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::FontResource::FaceExists(
	size_t font_face
) const
{
	return impl->FaceExists( font_face );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::TextureResource * vk2d::FontResource::GetTextureResource()
{
	return impl->GetTextureResource();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API const vk2d::GlyphInfo & vk2d::FontResource::GetGlyphInfo(
	char32_t	character,
	size_t		font_face
) const
{
	return impl->GetGlyphInfo(
		character,
		font_face
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::FontResource::IsGood() const
{
	return !!impl;
}
