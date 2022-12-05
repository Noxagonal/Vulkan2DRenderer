#pragma once

#include <core/SourceCommon.hpp>

#include <interface/resources/resource_base/ResourceBase.hpp>

#include <types/Synchronization.hpp>



namespace vk2d {

class ResourceBase;

namespace vk2d_internal {

class ResourceManagerImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
class ThreadPrivateResource;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Result of the multithreaded load operation for a resource.
enum class ResourceMTLoadResult
{
	SUCCESS,			///< Usable, loading completed successfully.
	SUCCESS_CONTINUED,	///< Usable, was partially loaded but should be loaded further later.
	POSTPONED,			///< Unusable, loading was postponed till later.
	FAILED,				///< Unusable, failed to load and is unusable.
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ResourceImplBase
{
	friend class ResourceManagerImpl;
	friend class ResourceThreadLoadTask;
	friend class ResourceThreadUnloadTask;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceImplBase() = delete;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceImplBase(
		ResourceBase									&	my_base_interface,
		uint32_t											loader_thread,
		ResourceManagerImpl								&	resource_manager,
		ResourceBase									*	parent_resource
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceImplBase(
		ResourceBase									&	my_base_interface,
		uint32_t											loader_thread,
		ResourceManagerImpl								&	resource_manager,
		ResourceBase									*	parent_resource,
		const std::vector<std::filesystem::path>		&	paths
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual													~ResourceImplBase() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Check the status of the resource.
	/// 
	/// @return		Resource status.
	virtual ResourceStatus									GetStatus() = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual ResourceStatus									WaitUntilLoaded(
		std::chrono::nanoseconds							timeout							= std::chrono::nanoseconds::max()
	) = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	virtual ResourceStatus									WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout
	) = 0;

protected:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Multithreaded load function.
	///
	///				Runs whenever the thread pool has time to process this resource.
	///
	/// @param[in]	thread_resource
	///				Represents a thread, collection of objects, and data attached to that specific thread.
	///
	/// @return		Result of the load operation.
	virtual ResourceMTLoadResult							MTLoad(
		ThreadPrivateResource							*	thread_resource
	) = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Multithreaded unload function.
	///
	///				Runs whenever the thread pool has time to process this resource.
	///
	///				This function is called before the resource itself is destroyed and regardless of the result the MTLoad()
	///				function returned so this function can be used for cleanup.
	///
	/// @param[in]	thread_resource
	///				Represents a thread, collection of objects, and data attached to that specific thread.
	virtual void											MTUnload(
		ThreadPrivateResource							*	thread_resource
	) = 0;

private:

	// TODO: Remove resource subresources and move onto reference counting and using ResourceMTLoadResult::POSTPONED instead.

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Destroy all subresources.
	///
	///				If the resource creates any subresources, they must NOT be manually destroyed. If subresources are deleted
	///				anywhere else it can lead to race conditions within resource manager. In case any resource uses
	///				sub-sub-resources, the resource manager will handle all cleanup.
	void													DestroySubresources_DEPRECATED();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Add a subresource.
	///
	///				Subresources can be created either in the resource constructor or MTLoad(). To create a subresource, we can
	///				create them just like regular resources, just add parent information.
	///
	/// @param		subresource
	///				Resource to add as a subresource.
	void													AddSubresource_DEPRECATED(
		ResourceBase									&	subresource
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ResourceBase										*	GetParentResource_DEPRECATED();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get assigned loader thread.
	///
	/// @return		Thread index assigned to this resource.
	uint32_t												GetLoaderThread();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Check if this resource was loaded from a file.
	///
	/// @return		True if resource was loaded from a file, false otherwise.
	bool													IsFromFile() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get file paths where this resource was loaded from.
	///
	/// @note		If the resource was created instead of loaded from a file, then returned list will be empty.
	///
	/// @return		List of paths to files where this resource was loaded from.
	const std::vector<std::filesystem::path>			&	GetFilePaths() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool											IsGood() const						= 0;

protected:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Check if this resource is assigned as a subresource.
	///
	///				This tells if this resource should be managed and deleted by the resource manager or another resource.
	///
	/// @warning	If this returns true then resource manager should not delete this resource directly.
	///
	/// @return		True if this resource is a subresource to some other resource.
	bool													IsSubResource() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Fence													load_function_run_fence;
	std::atomic<ResourceStatus>								status								= {};
	ResourceBase										&	my_base_interface;
	ResourceManagerImpl									&	resource_manager;

private:
	uint32_t												loader_thread						= {};
	std::vector<std::filesystem::path>						file_paths							= {};
	std::mutex												subresources_mutex_DEPRECATED;
	std::vector<ResourceBase*>								subresources_DEPRECATED				= {};
	ResourceBase										*	parent_resource						= {};
	bool													is_from_file						= {};
};


} // vk2d_internal

} // vk2d
