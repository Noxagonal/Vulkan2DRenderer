#pragma once

#include "SourceCommon.h"

#include "ThreadPool.h"

namespace vk2d {

namespace _internal {

class RendererImpl;
class DescriptorAutoPool;
class DeviceMemoryPool;



class ThreadLoaderResource : public ThreadPrivateResource {
public:
	ThreadLoaderResource(
		vk2d::_internal::RendererImpl * parent
	);

	~ThreadLoaderResource()
	{}

	vk2d::_internal::RendererImpl							*	GetRenderer() const;
	VkDevice													GetVulkanDevice() const;
	vk2d::_internal::DeviceMemoryPool						*	GetDeviceMemoryPool() const;
	vk2d::_internal::DescriptorAutoPool						*	GetDescriptorAutoPool() const;
	VkCommandPool												GetPrimaryRenderCommandPool() const;
	VkCommandPool												GetSecondaryRenderCommandPool() const;
	VkCommandPool												GetPrimaryTransferCommandPool() const;

protected:
	bool														ThreadBegin();
	void														ThreadEnd();

private:
	vk2d::_internal::RendererImpl							*	renderer							= {};
	VkDevice													device								= {};
	std::unique_ptr<vk2d::_internal::DescriptorAutoPool>		descriptor_auto_pool				= {};
	std::unique_ptr<vk2d::_internal::DeviceMemoryPool>			device_memory_pool					= {};

	VkCommandPool												primary_render_command_pool			= {};
	VkCommandPool												secondary_render_command_pool		= {};
	VkCommandPool												primary_transfer_command_pool		= {};
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


} // _internal

} // vk2d
