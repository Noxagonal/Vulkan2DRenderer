#pragma once

#include "core/SourceCommon.h"

#include "interface/resources/ResourceBase.h"

#include "types/Synchronization.hpp"



namespace vk2d {

class ResourceBase;

namespace vk2d_internal {



class ResourceManagerImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
class ThreadPrivateResource;



class ResourceImplBase
{
	friend class ResourceManagerImpl;
	friend class ResourceThreadLoadTask;
	friend class ResourceThreadUnloadTask;

public:
															ResourceImplBase()					= delete;

															ResourceImplBase(
		ResourceBase									*	my_interface,
		uint32_t											loader_thread,
		ResourceManagerImpl								*	resource_manager,
		ResourceBase									*	parent_resource );

															ResourceImplBase(
		ResourceBase									*	my_interface,
		uint32_t											loader_thread,
		ResourceManagerImpl								*	resource_manager,
		ResourceBase									*	parent_resource,
		const std::vector<std::filesystem::path>		&	paths );

	virtual													~ResourceImplBase()					= default;

	// Checks the status of the resource.
	virtual ResourceStatus									GetStatus()						= 0;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual ResourceStatus									WaitUntilLoaded(
		std::chrono::nanoseconds							timeout							= std::chrono::nanoseconds::max() ) = 0;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual ResourceStatus									WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout )						= 0;

protected:
	// Multithreaded load function, runs when the thread pool has time to process this resource.
	// Return true if loading was successful.
	virtual bool											MTLoad(
		ThreadPrivateResource							*	thread_resource )				= 0;

	// Multithreaded unload function, runs when the thread pool has time to process this resource.
	// Internal use only.
	virtual void											MTUnload(
		ThreadPrivateResource							*	thread_resource )				= 0;

private:
	// Internal use only.
	// If the resource creates any subresources, they must NOT be manually destroyed.
	// If subresources are deleted anywhere else it can lead to race conditions within resource manager.
	// In case any resource uses sub-sub-resources, the resource manager will handle all cleanup.
	void													DestroySubresources();

	// Internal use only.
	// Subresources can be created either in the resource constructor or MTLoad(). To create a subresource,
	// we can create them just like regular resources, just add parent information.
	void													AddSubresource(
		ResourceBase									*	subresource );

public:
	ResourceBase										*	GetParentResource();

	// Gets the thread index that was responsible for loading this resource.
	uint32_t												GetLoaderThread();

	// Checks if the resource was loaded from a file.
	// Returns true if the resource origin is in a file, for example an image, false otherwise.
	bool													IsFromFile() const;

	// Returns the file path where the resource was loaded from,
	// if the was not loaded from a file, returns "".
	const std::vector<std::filesystem::path>			&	GetFilePaths() const;

	virtual bool											IsGood() const						= 0;

protected:
	// Internal use only, this tells if this resource should be managed and
	// deleted by the resource manager or another resource. If this returns
	// true then resource manager should not delete this resource directly.
	bool													IsSubResource() const;

	Fence													load_function_run_fence;
	std::atomic<ResourceStatus>								status								= {};
	ResourceBase										*	my_interface						= {};

private:
	ResourceManagerImpl									*	resource_manager					= {};
	uint32_t												loader_thread						= {};
	std::vector<std::filesystem::path>						file_paths							= {};
	std::mutex												subresources_mutex;
	std::vector<ResourceBase*>								subresources						= {};
	ResourceBase										*	parent_resource						= {};
	bool													is_from_file						= {};
};


} // vk2d_internal

} // vk2d
