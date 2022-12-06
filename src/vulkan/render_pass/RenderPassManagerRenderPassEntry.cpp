
#include <core/SourceCommon.hpp>
#include "RenderPassManagerRenderPassEntry.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManagerRenderPassEntry::RenderPassManagerRenderPassEntry(
	VkRenderPass		vulkan_render_pass,
	size_t				hash
) :
	vulkan_render_pass( vulkan_render_pass ),
	hash( hash )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManagerRenderPassEntry::RenderPassManagerRenderPassEntry(
	RenderPassManagerRenderPassEntry && other
) noexcept
{
	MoveOther( std::move( other ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManagerRenderPassEntry::~RenderPassManagerRenderPassEntry()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::RenderPassManagerRenderPassEntry & vk2d::vulkan::RenderPassManagerRenderPassEntry::operator=(
	RenderPassManagerRenderPassEntry && other
	) noexcept
{
	MoveOther( std::move( other ) );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vulkan::RenderPassManagerRenderPassEntry::operator==(
	const RenderPassManagerRenderPassEntry & other
	)
{
	return vulkan_render_pass == other.vulkan_render_pass;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkRenderPass vk2d::vulkan::RenderPassManagerRenderPassEntry::GetVulkanRenderPass()
{
	return vulkan_render_pass;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::vulkan::RenderPassManagerRenderPassEntry::GetHash()
{
	return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::RenderPassManagerRenderPassEntry::MoveOther(
	RenderPassManagerRenderPassEntry && other
) noexcept
{
	if( other == *this ) return;

	// Mutex cannot be moved so we only allow moving if the reference count is 0 for both,
	// which means that both entries aren't in use.
	#if VK2D_DEBUG_ENABLE
	reference_count(
		[]( size_t & count )
		{
			assert( count == 0 );
		}
	);
	other.reference_count(
		[]( size_t & count )
		{
			assert( count == 0 );
		}
	);
	#endif

	std::swap( vulkan_render_pass, other.vulkan_render_pass );
	std::swap( hash, other.hash );
}
