#pragma once

#include "Core/SourceCommon.h"

#include "Interface/Resources/ResourceImplBase.h"



namespace vk2d {

class GraphicsPipelineResource;

namespace _internal {



class GraphicsPipelineResourceImpl :
	public vk2d::_internal::ResourceImplBase
{
public:
	GraphicsPipelineResourceImpl(
		vk2d::GraphicsPipelineResource						*	my_interface,
		vk2d::_internal::ResourceManagerImpl				*	resource_manager,
		uint32_t												loader_thread,
		vk2d::ResourceBase									*	parent_resource,
		const std::filesystem::path							&	file_path );

	~GraphicsPipelineResourceImpl();

	// Checks the status of the resource.
	vk2d::ResourceStatus										GetStatus() override;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	vk2d::ResourceStatus										WaitUntilLoaded(
		std::chrono::nanoseconds								timeout								= std::chrono::nanoseconds::max() ) override;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	vk2d::ResourceStatus										WaitUntilLoaded(
		std::chrono::steady_clock::time_point					timeout ) override;

protected:
	// Multithreaded load function, runs when the thread pool has time to process this resource.
	// Return true if loading was successful.
	bool														MTLoad(
		vk2d::_internal::ThreadPrivateResource				*	thread_resource ) override;

	// Multithreaded load function, runs when the thread pool has time to process this resource further.
	// Return true if loading was successful.
	bool														MTLoadMore(
		vk2d::_internal::ThreadPrivateResource				*	thread_resource ) override;

	// Multithreaded unload function, runs when the thread pool has time to process this resource.
	// Internal use only.
	void														MTUnload(
		vk2d::_internal::ThreadPrivateResource				*	thread_resource ) override;

public:
	bool														IsGood() const override;

private:
	bool														is_good									= {};
};



} // _internal

} // vk2d
