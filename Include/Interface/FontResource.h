#pragma once

#include "../Core/Common.h"
#include "Resource.h"

#include <filesystem>

namespace vk2d {

class TextureResource;

namespace _internal {

class ResourceManagerImpl;
class FontResourceImpl;

} // _internal



class FontResource
	: public vk2d::Resource
{
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::FontResourceImpl;

public:
	VK2D_API																					FontResource(
		vk2d::Resource									*	parent_resource,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		uint32_t											loader_thread,
		std::filesystem::path								file_path,
		uint32_t											glyph_texel_size,
		bool												use_alpha,
		uint32_t											glyph_atlas_padding );

	VK2D_API																					~FontResource();

	VK2D_API bool											VK2D_APIENTRY						IsLoaded();

	VK2D_API bool											VK2D_APIENTRY						WaitUntilLoaded();

#pragma VK2D_WARNING( "REMOVE WHEN DONE TESTING!" )
	VK2D_API vk2d::TextureResource						*	VK2D_APIENTRY						GetTextureResource();

protected:
	VK2D_API bool											VK2D_APIENTRY						MTLoad(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	VK2D_API void											VK2D_APIENTRY						MTUnload(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

private:

	std::unique_ptr<vk2d::_internal::FontResourceImpl>		impl								= {};
};



} // vk2d
