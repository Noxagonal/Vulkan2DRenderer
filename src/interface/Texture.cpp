
#include "core/SourceCommon.h"

#include "interface/Texture.h"
#include "interface/TextureImpl.h"



VK2D_API bool vk2d::Texture::IsTextureDataReady()
{
	assert( texture_impl );
	return texture_impl->IsTextureDataReady();
}

VK2D_API glm::uvec2 vk2d::Texture::GetSize() const
{
	assert( texture_impl );
	return texture_impl->GetSize();
}

VK2D_API uint32_t vk2d::Texture::GetLayerCount() const
{
	assert( texture_impl );
	return texture_impl->GetLayerCount();
}

VK2D_API bool vk2d::Texture::IsGood() const
{
	if( !texture_impl ) return false;
	return texture_impl->IsGood();
}
