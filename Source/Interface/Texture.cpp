
#include "Core/SourceCommon.h"

#include "Interface/Texture.h"
#include "Interface/TextureImpl.h"



VK2D_API bool VK2D_APIENTRY vk2d::Texture::IsTextureDataReady()
{
	assert( texture_impl );
	return texture_impl->IsTextureDataReady();
}

VK2D_API glm::uvec2 VK2D_APIENTRY vk2d::Texture::GetSize() const
{
	assert( texture_impl );
	return texture_impl->GetSize();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::Texture::GetLayerCount() const
{
	assert( texture_impl );
	return texture_impl->GetLayerCount();
}

VK2D_API bool VK2D_APIENTRY vk2d::Texture::IsGood() const
{
	if( !texture_impl ) return false;
	return texture_impl->IsGood();
}
