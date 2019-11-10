#pragma once

#include "SourceCommon.h"

#include "ThreadPool.h"

namespace vk2d {

namespace _internal {

class RendererImpl;
class DescriptorAutoPool;



class ThreadLoaderResource : public ThreadPrivateResource {
public:
	ThreadLoaderResource(
		RendererImpl * parent
	);

	~ThreadLoaderResource()
	{}

	RendererImpl							*	GetRenderer() const;
	VkDevice									GetVulkanDevice() const;
	DescriptorAutoPool						*	GetDescriptorAutoPool() const;
	VkCommandPool								GetPrimaryRenderCommandPool() const;
	VkCommandPool								GetSecondaryRenderCommandPool() const;
	VkCommandPool								GetPrimaryTransferCommandPool() const;

protected:
	bool										ThreadBegin();
	void										ThreadEnd();

private:
	RendererImpl							*	parent								= {};
	VkDevice									device								= {};
	std::unique_ptr<DescriptorAutoPool>			descriptor_auto_pool				= {};

	VkCommandPool								primary_render_command_pool			= {};
	VkCommandPool								secondary_render_command_pool		= {};
	VkCommandPool								primary_transfer_command_pool		= {};
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
