#pragma once

#include <core/SourceCommon.h>

#include <system/DescriptorSet.h>

#include <types/Multisamples.h>



namespace vk2d {
namespace vk2d_internal {



class InstanceImpl;
class WindowImpl;
class RenderTargetTextureImpl;



struct TimedDescriptorPoolData
{
	PoolDescriptorSet										descriptor_set								= {};
	std::chrono::time_point<std::chrono::steady_clock>		previous_access_time						= {};	// For cleanup
};



struct RenderTargetTextureDependencyInfo
{
	RenderTargetTextureImpl								*	render_target								= {};
	uint32_t												swap_buffer_index							= {};
};



/// @brief		Used to collect render information from render target texture when render is going to happen.
/// @see		WindowImpl::EndRender().
class RenderTargetTextureRenderCollector
{
	friend WindowImpl;

public:
	struct Collection
	{
		VkSubmitInfo		*	vk_transfer_submit_info			= {};
		VkSubmitInfo		*	vk_render_submit_info			= {};
	};

	void						Append(
		VkSubmitInfo		*	transfer_submit_info,
		VkSubmitInfo		*	render_submit_info
	);

	RenderTargetTextureRenderCollector::Collection				&	operator[]( size_t index );
	RenderTargetTextureRenderCollector::Collection				*	begin();
	RenderTargetTextureRenderCollector::Collection				*	end();
	size_t																			size();

private:
	std::vector<RenderTargetTextureRenderCollector::Collection>	collection;
};



Multisamples					CheckSupportedMultisampleCount(
	InstanceImpl			&	instance,
	Multisamples				samples
);



} // vk2d_internal
} // vk2d
