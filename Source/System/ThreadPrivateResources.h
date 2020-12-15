#pragma once

#include "Core/SourceCommon.h"

#include "System/ThreadPool.h"
#include "System/DescriptorSet.h"
#include "System/VulkanMemoryManagement.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {

namespace _internal {

class InstanceImpl;
class DescriptorAutoPool;
class DeviceMemoryPool;



class ThreadLoaderResource : public vk2d::_internal::ThreadPrivateResource {
public:
	ThreadLoaderResource(
		vk2d::_internal::InstanceImpl						*	instance );

	~ThreadLoaderResource()
	{}

	vk2d::_internal::InstanceImpl							*	GetInstance() const;
	VkDevice													GetVulkanDevice() const;
	vk2d::_internal::DeviceMemoryPool						*	GetDeviceMemoryPool() const;
	vk2d::_internal::DescriptorAutoPool						*	GetDescriptorAutoPool() const;
	VkCommandPool												GetPrimaryRenderCommandPool() const;
	VkCommandPool												GetSecondaryRenderCommandPool() const;
	VkCommandPool												GetPrimaryTransferCommandPool() const;
	FT_Library													GetFreeTypeInstance() const;

protected:
	bool														ThreadBegin();
	void														ThreadEnd();

private:
	vk2d::_internal::InstanceImpl							*	instance						= {};
	VkDevice													device								= {};
	std::unique_ptr<vk2d::_internal::DescriptorAutoPool>		descriptor_auto_pool				= {};
	std::unique_ptr<vk2d::_internal::DeviceMemoryPool>			device_memory_pool					= {};

	VkCommandPool												primary_render_command_pool			= {};
	VkCommandPool												secondary_render_command_pool		= {};
	VkCommandPool												primary_transfer_command_pool		= {};

	FT_Library													freetype_instance					= {};
};



class ThreadGeneralResource : public vk2d::_internal::ThreadPrivateResource {
protected:
	bool			ThreadBegin()
	{
		return true;
	};
	void			ThreadEnd()
	{};
};


} // _internal

} // vk2d
