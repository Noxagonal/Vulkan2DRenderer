#pragma once

#include <core/SourceCommon.hpp>

#include "GraphicsPipelineCreateInfo.hpp"
#include "ComputePipelineCreateInfo.hpp"

#include "PipelineHandle.hpp"



namespace vk2d {
namespace vk2d_internal {

class InstanceImpl;

} // vk2d_internal

namespace vulkan {

class Device;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineManager
{
	friend class PipelineHandle;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct PipelineEntry
	{
		VkPipeline									vulkan_pipeline					= {};
		size_t										reference_count					= {};
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManager(
		vk2d_internal::InstanceImpl					&	instance,
		Device										&	vulkan_device
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManager(
		const PipelineManager						&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PipelineManager(
		PipelineManager								&&	other
	) = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~PipelineManager();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tries to find a graphics pipeline and return a handle to it if it exists.
	///
	/// @param		graphics_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to find.
	///
	/// @return		Handle to existing pipeline or empty handle if pipeline was not found.
	PipelineHandle									FindGraphicsPipeline(
		const GraphicsPipelineCreateInfo		&	graphics_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tries to find a compute pipeline and return a handle to it if it exists.
	///
	/// @param		compute_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to find.
	///
	/// @return		Handle to existing pipeline or empty handle if pipeline was not found.
	PipelineHandle									FindComputePipeline(
		const ComputePipelineCreateInfo			&	compute_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tries to find a graphics or compute pipeline and return a handle to it if it exists.
	///
	/// @param		pipeline_hash
	///				Hash of the pipeline we wish to find.
	///
	/// @return		Handle to existing pipeline or empty handle if pipeline was not found.
	PipelineHandle									FindPipeline(
		size_t										pipeline_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets a graphics pipeline if it exists or creates a new graphics pipeline if it does not exist.
	///
	/// @param[in]	graphics_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to find or create.
	///
	/// @return		Handle to pipeline.
	PipelineHandle									GetGraphicsPipeline(
		const GraphicsPipelineCreateInfo		&	graphics_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets a compute pipeline if it exists or creates a new compute pipeline if it does not exist.
	///
	/// @param[in]	compute_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to find or create.
	///
	/// @return		Handle to pipeline.
	PipelineHandle									GetComputePipeline(
		const ComputePipelineCreateInfo			&	compute_pipeline_create_info
	);

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Creates a new graphics pipeline.
	///
	/// @param[in]	graphics_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to create.
	///
	/// @return		Handle to pipeline.
	PipelineHandle									CreateGraphicsPipeline(
		const GraphicsPipelineCreateInfo		&	graphics_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Creates a new compute pipeline.
	///
	/// @param[in]	compute_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to create.
	///
	/// @return		Handle to pipeline.
	PipelineHandle									CreateComputePipeline(
		const ComputePipelineCreateInfo			&	compute_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											IncrementReferenceCount(
		size_t										pipeline_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DecrementReferenceCount(
		size_t										pipeline_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyPipeline(
		size_t										pipeline_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyPipeline(
		std::map<size_t, PipelineEntry>::iterator	pipeline_list_iterator
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void											DestroyAllPipelines();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache									GetGraphicsPipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	/// 
	/// @note		Multithreading: Any thread.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache									GetComputePipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool											IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool											CreateGraphicsPipelineCache();
	bool											CreateComputePipelineCache();

	void											DestroyGraphicsPipelineCache();
	void											DestroyComputePipelineCache();

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl					&	instance;
	Device										&	vulkan_device;

	std::map<size_t, PipelineEntry>					pipeline_list;

	VkPipelineCache									vulkan_graphics_pipeline_cache				= {};
	VkPipelineCache									vulkan_compute_pipeline_cache				= {};

	bool											is_good										= {};
};



} // vulkan
} // vk2d
