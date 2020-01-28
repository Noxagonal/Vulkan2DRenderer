#pragma once

#include "SourceCommon.h"

#include "ThreadPool.h"
#include "DescriptorSet.h"
#include "VulkanMemoryManagement.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {

namespace _internal {

class RendererImpl;
class DescriptorAutoPool;
class DeviceMemoryPool;



class ThreadLoaderResource : public vk2d::_internal::ThreadPrivateResource {
public:
	ThreadLoaderResource(
		vk2d::_internal::RendererImpl						*	renderer );

	~ThreadLoaderResource()
	{}

	vk2d::_internal::RendererImpl							*	GetRenderer() const;
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
	vk2d::_internal::RendererImpl							*	renderer_parent						= {};
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
