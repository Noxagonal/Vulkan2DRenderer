#pragma once

#include "core/SourceCommon.h"

#include "system/ThreadPool.h"
#include "system/DescriptorSet.h"
#include "system/VulkanMemoryManagement.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {

namespace vk2d_internal {

class InstanceImpl;
class DescriptorAutoPool;
class DeviceMemoryPool;



class ThreadLoaderResource : public ThreadPrivateResource {
public:
	ThreadLoaderResource(
		InstanceImpl						*	instance );

	~ThreadLoaderResource()
	{}

	InstanceImpl							*	GetInstance() const;
	VkDevice													GetVulkanDevice() const;
	DeviceMemoryPool						*	GetDeviceMemoryPool() const;
	DescriptorAutoPool						*	GetDescriptorAutoPool() const;
	VkCommandPool								GetPrimaryRenderCommandPool() const;
	VkCommandPool								GetSecondaryRenderCommandPool() const;
	VkCommandPool								GetPrimaryTransferCommandPool() const;
	FT_Library									GetFreeTypeInstance() const;

protected:
	bool										ThreadBegin();
	void										ThreadEnd();

private:
	InstanceImpl							*	instance						= {};
	VkDevice									device								= {};
	std::unique_ptr<DescriptorAutoPool>			descriptor_auto_pool				= {};
	std::unique_ptr<DeviceMemoryPool>			device_memory_pool					= {};

	VkCommandPool								primary_render_command_pool			= {};
	VkCommandPool								secondary_render_command_pool		= {};
	VkCommandPool								primary_transfer_command_pool		= {};

	FT_Library									freetype_instance					= {};
};



class ThreadGeneralResource : public ThreadPrivateResource {
protected:
	bool			ThreadBegin()
	{
		return true;
	};
	void			ThreadEnd()
	{};
};


} // vk2d_internal

} // vk2d
