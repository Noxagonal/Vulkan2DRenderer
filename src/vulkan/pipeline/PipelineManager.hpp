#pragma once

#include <core/SourceCommon.hpp>

#include "GraphicsPipelineCreateInfo.hpp"
#include "ComputePipelineCreateInfo.hpp"

#include "PipelineHandle.hpp"
#include "PipelineManagerPipelineEntry.hpp"

#include <types/Synchronization.hpp>



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
	// TODO: Find better ways to implement a hash map. eg. std::flat_map. Mind pointer invalidation though.
	using PipelineList = std::map<size_t, PipelineManagerPipelineEntry>;

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
	/// @note		Multithreading: Internally synchronized.
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
	/// @note		Multithreading: Internally synchronized.
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
	/// @note		Multithreading: Internally synchronized.
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
	/// @note		Multithreading: Internally synchronized.
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
	/// @note		Multithreading: Internally synchronized.
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
	/// @warning	Existance of a similar pipeline is not checked here, doing multiple similar pipelines is an error.
	///
	/// @note		Multithreading: Internally synchronized.
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
	/// @warning	Existance of a similar pipeline is not checked here, doing multiple similar pipelines is an error.
	///
	/// @note		Multithreading: Internally synchronized.
	///
	/// @param[in]	compute_pipeline_create_info
	///				Pipeline create info structure describing the pipeline we want to create.
	///
	/// @return		Handle to pipeline.
	PipelineHandle									CreateComputePipeline(
		const ComputePipelineCreateInfo			&	compute_pipeline_create_info
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Increment reference count to a pipeline entry.
	///
	/// @note		Multithreading: Internally synchronized.
	///
	/// @param		pipeline_entry
	///				Pipeline entry to increment.
	void											IncrementReferenceCount(
		PipelineManagerPipelineEntry			*	pipeline_entry
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Decrement reference count to a pipeline entry.
	///
	///				If reference count drops to 0, pipeline is destroyed.
	///
	/// @note		Multithreading: Internally synchronized.
	///
	/// @param		pipeline_entry
	///				Pipeline entry to decrement.
	void											DecrementReferenceCount(
		PipelineManagerPipelineEntry			*	pipeline_entry
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy a specific pipeline regardless of its reference count.
	///
	/// @note		Multithreading: Internally synchronized.
	///
	/// @param		pipeline_hash
	///				Hash of the pipeline to destroy.
	void											DestroyPipeline(
		size_t										pipeline_hash
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy all pipelines contained within this pipeline manager.
	/// 
	/// @note		Multithreading: Internally synchronized.
	void											DestroyAllPipelines();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get graphics pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	///  
	/// @note		Multithreading: Internally synchronized.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache									GetGraphicsPipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get compute pipeline cache.
	///
	///				Pipeline cache is used to speed up the creation of new pipelines.
	/// 
	/// @note		Multithreading: Internally synchronized.
	///
	/// @return		Graphics pipeline cache.
	VkPipelineCache									GetComputePipelineCache() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool											IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create graphics pipeline cache which is used to speed up pipeline creation.
	///
	/// @note		Multithreading: Main thread only.
	///
	/// @return		true on success, false if something went wrong.
	bool											CreateGraphicsPipelineCache();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Create compute pipeline cache which is used to speed up pipeline creation.
	///
	/// @note		Multithreading: Main thread only.
	///
	/// @return		true on success, false if something went wrong.
	bool											CreateComputePipelineCache();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy graphics pipeline cache.
	///
	/// @note		Multithreading: Main thread only.
	void											DestroyGraphicsPipelineCache();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy compute pipeline cache.
	///
	/// @note		Multithreading: Main thread only.
	void											DestroyComputePipelineCache();

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vk2d_internal::InstanceImpl					&	instance;
	Device										&	vulkan_device;

	MutexObject<PipelineList>						pipeline_list;

	VkPipelineCache									vulkan_graphics_pipeline_cache				= {};
	VkPipelineCache									vulkan_compute_pipeline_cache				= {};

	bool											is_good										= {};
};



} // vulkan
} // vk2d
