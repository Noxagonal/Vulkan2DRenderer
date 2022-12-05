#pragma once

#include <core/SourceCommon.hpp>

#include <types/Synchronization.hpp>



namespace vk2d {
namespace vulkan {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineManagerPipelineEntry
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManagerPipelineEntry(
		VkPipeline									vulkan_pipeline,
		size_t										hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManagerPipelineEntry(
		const PipelineManagerPipelineEntry		&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManagerPipelineEntry(
		PipelineManagerPipelineEntry			&&	other
	) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~PipelineManagerPipelineEntry();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManagerPipelineEntry				&	operator=(
		const PipelineManagerPipelineEntry		&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManagerPipelineEntry				&	operator=(
		PipelineManagerPipelineEntry			&&	other
	) noexcept;

	bool											operator==(
		const PipelineManagerPipelineEntry		&	other
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkPipeline										GetVulkanPipeline();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	size_t											GetHash();

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											MoveOther(
		PipelineManagerPipelineEntry			&&	other
	) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkPipeline										vulkan_pipeline			= {};
	size_t											hash					= {};

public:
	MutexObject<size_t>								reference_count;
};



} // vulkan
} // vk2d
