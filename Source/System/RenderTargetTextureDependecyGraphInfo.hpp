#pragma once

#include "../Core/SourceCommon.h"

#include "DescriptorSet.h"

#include "../../Include/Types/Multisamples.h"



namespace vk2d {
namespace _internal {



class InstanceImpl;
class WindowImpl;
class RenderTargetTextureImpl;



struct TimedDescriptorPoolData
{
	vk2d::_internal::PoolDescriptorSet						descriptor_set								= {};
	std::chrono::time_point<std::chrono::steady_clock>		previous_access_time						= {};	// For cleanup
};



struct RenderTargetTextureDependencyInfo
{
	vk2d::_internal::RenderTargetTextureImpl			*	render_target								= {};
	uint32_t												swap_buffer_index							= {};
};



/// @brief		Used to collect render information from render target texture when render is going to happen.
/// @see		vk2d::_internal::WindowImpl::EndRender().
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

	vk2d::_internal::RenderTargetTextureRenderCollector::Collection				&	operator[]( size_t index );
	vk2d::_internal::RenderTargetTextureRenderCollector::Collection				*	begin();
	vk2d::_internal::RenderTargetTextureRenderCollector::Collection				*	end();
	size_t																			size();

private:
	std::vector<vk2d::_internal::RenderTargetTextureRenderCollector::Collection>	collection;
};



vk2d::Multisamples						CheckSupportedMultisampleCount(
	vk2d::_internal::InstanceImpl	*	instance,
	vk2d::Multisamples					samples
);



} // _internal
} // vk2d
