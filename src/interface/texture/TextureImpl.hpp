#pragma once

#include <core/SourceCommon.hpp>

#include <interface/texture/Texture.hpp>



namespace vk2d {
namespace vk2d_internal {



class InstanceImpl;



class TextureImpl
{
public:
	virtual glm::uvec2								GetSize() const					= 0;
	virtual uint32_t								GetLayerCount() const			= 0;
	virtual VkImage									GetVulkanImage() const			= 0;
	virtual VkImageView								GetVulkanImageView() const		= 0;
	virtual VkImageLayout							GetVulkanImageLayout() const	= 0;

	virtual bool									IsTextureDataReady()			= 0;

	virtual bool									IsGood() const					= 0;

private:
};



} // vk2d_internal
} // vk2d
