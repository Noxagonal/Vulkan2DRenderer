
#include <core/SourceCommon.hpp>
#include "PipelineManagerPipelineEntry.hpp"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PipelineManagerPipelineEntry::PipelineManagerPipelineEntry(
	VkPipeline vulkan_pipeline,
	size_t hash
) :
	vulkan_pipeline( vulkan_pipeline ),
	hash( hash )
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PipelineManagerPipelineEntry::PipelineManagerPipelineEntry(
	PipelineManagerPipelineEntry && other
) noexcept
{
	MoveOther( std::move( other ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PipelineManagerPipelineEntry::~PipelineManagerPipelineEntry()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vk2d::vulkan::PipelineManagerPipelineEntry & vk2d::vulkan::PipelineManagerPipelineEntry::operator=(
	PipelineManagerPipelineEntry && other
) noexcept
{
	MoveOther( std::move( other ) );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool vk2d::vulkan::PipelineManagerPipelineEntry::operator==(
	const PipelineManagerPipelineEntry & other
)
{
	return vulkan_pipeline == other.vulkan_pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPipeline vk2d::vulkan::PipelineManagerPipelineEntry::GetVulkanPipeline()
{
	return vulkan_pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t vk2d::vulkan::PipelineManagerPipelineEntry::GetHash()
{
	return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void vk2d::vulkan::PipelineManagerPipelineEntry::MoveOther(
	PipelineManagerPipelineEntry && other
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

	std::swap( vulkan_pipeline, other.vulkan_pipeline );
	std::swap( hash, other.hash );
}
