#pragma once

#include <core/SourceCommon.hpp>

#include <system/ThreadPool.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>

#include <vulkan/utils/VulkanMemoryManagement.hpp>
#include <vulkan/descriptor_set/DescriptorSet.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vk2d {
namespace vulkan {
class DescriptorAutoPool;
class DeviceMemoryPool;
} // vulkan

namespace vk2d_internal {

class InstanceImpl;



class ThreadLoaderResource : public ThreadPrivateResource {
public:
	ThreadLoaderResource(
		InstanceImpl						&	instance );

	~ThreadLoaderResource()
	{}

	InstanceImpl							&	GetInstance();
	VkDevice									GetVulkanDevice() const;
	vulkan::DeviceMemoryPool				*	GetDeviceMemoryPool();
	vulkan::DescriptorAutoPool				*	GetDescriptorAutoPool();
	VkCommandPool								GetPrimaryRenderCommandPool() const;
	VkCommandPool								GetSecondaryRenderCommandPool() const;
	VkCommandPool								GetPrimaryTransferCommandPool() const;
	FT_Library									GetFreeTypeInstance() const;

protected:
	bool										ThreadBegin();
	void										ThreadEnd();

private:
	InstanceImpl							&	instance;
	VkDevice									device								= {};
	std::optional<vulkan::DescriptorAutoPool>	descriptor_auto_pool				= {};
	std::optional<vulkan::DeviceMemoryPool>		device_memory_pool					= {};

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
