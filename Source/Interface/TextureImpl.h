#pragma once

#include "../Core/SourceCommon.h"
#include "../../Include/Types/Vector2.hpp"



namespace vk2d {
namespace _internal {



class InstanceImpl;



class TextureImpl
{
public:
	virtual vk2d::Vector2u							GetSize() const					= 0;
	virtual uint32_t								GetLayerCount() const			= 0;
	virtual VkImage									GetVulkanImage() const			= 0;
	virtual VkImageView								GetVulkanImageView() const		= 0;
	virtual VkImageLayout							GetVulkanImageLayout() const	= 0;

	virtual bool									IsTextureDataReady()			= 0;

	virtual bool									IsGood() const					= 0;

private:
};



} // _internal
} // vk2d
