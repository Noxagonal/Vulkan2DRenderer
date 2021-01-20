#pragma once

#include "Core/SourceCommon.h"

#include "Interface/Resources/ResourceBase.h"

#include "Types/Synchronization.hpp"



namespace vk2d {

class ResourceBase;

namespace _internal {



class ResourceManagerImpl;
class ResourceThreadLoadTask;
class ResourceThreadLoadMoreTask;
class ResourceThreadUnloadTask;
class ThreadPrivateResource;



/// @brief		Some VK2D resources can continue loading stuff after initial loading has completed.
enum class ResourceLoadMoreStatus : uint32_t
{
	IDLE					= 0,
	QUEUED,
	LOADING,
};



class ResourceImplBase
{
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::ResourceThreadLoadTask;
	friend class vk2d::_internal::ResourceThreadLoadMoreTask;
	friend class vk2d::_internal::ResourceThreadUnloadTask;

public:
															ResourceImplBase()					= delete;

															ResourceImplBase(
		vk2d::ResourceBase								*	my_interface,
		uint32_t											loader_thread,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::ResourceBase								*	parent_resource );

															ResourceImplBase(
		vk2d::ResourceBase								*	my_interface,
		uint32_t											loader_thread,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::ResourceBase								*	parent_resource,
		const std::vector<std::filesystem::path>		&	paths );

	virtual													~ResourceImplBase()					= default;

	// Checks the status of the resource.
	virtual vk2d::ResourceStatus							GetStatus()							= 0;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual vk2d::ResourceStatus							WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max() ) = 0;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual vk2d::ResourceStatus							WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout )							= 0;

protected:
	// Multithreaded load function, runs when the thread pool has time to process this resource.
	// Return true if loading was successful.
	virtual bool											MTLoad(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource )					= 0;

	// Optional multithreaded load function to load more afterwards, runs when the thread pool has
	// time to process this resource further. Returns true if loading was successful.
	virtual bool											MTLoadMore(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource );

	// Multithreaded unload function, runs when the thread pool has time to process this resource.
	// Internal use only.
	virtual void											MTUnload(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource )					= 0;

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
		vk2d::ResourceBase								*	subresource );

public:
	vk2d::ResourceBase									*	GetParentResource();

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

	vk2d::_internal::Fence									load_function_run_fence;
	std::atomic<vk2d::ResourceStatus>						status								= {};
	std::mutex												load_more_status_mutex;
	vk2d::_internal::ResourceLoadMoreStatus					load_more_status					= {};
	vk2d::ResourceBase									*	my_interface						= {};

private:
	vk2d::_internal::ResourceManagerImpl				*	resource_manager					= {};
	uint32_t												loader_thread						= {};
	std::vector<std::filesystem::path>						file_paths							= {};
	std::mutex												subresources_mutex;
	std::vector<vk2d::ResourceBase*>						subresources						= {};
	vk2d::ResourceBase									*	parent_resource						= {};
	bool													is_from_file						= {};
};


} // _internal

} // vk2d
