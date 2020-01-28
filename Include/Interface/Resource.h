#pragma once

#include "../Core/Common.h"

#include <filesystem>
#include <atomic>
#include <mutex>

namespace vk2d {

namespace _internal {
class ResourceManagerImpl;
class LoadTask;
class UnloadTask;
class ThreadPrivateResource;
}


class Resource {
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::LoadTask;
	friend class vk2d::_internal::UnloadTask;

public:
	VK2D_API																					Resource(
		uint32_t											thread,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::Resource									*	parent_resource );

	VK2D_API																					Resource(
		uint32_t											thread,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::Resource									*	parent_resource,
		std::filesystem::path								path );

	VK2D_API																					Resource(
		uint32_t											thread,
		vk2d::_internal::ResourceManagerImpl			*	resource_manager,
		vk2d::Resource									*	parent_resource,
		const std::vector<std::filesystem::path>		&	paths );

	VK2D_API virtual																			~Resource()
	{};

	// Checks if the resource is ready to be used.
	// Returns true if resource is loaded, false otherwise.
	VK2D_API virtual bool									VK2D_APIENTRY						IsLoaded()						= 0;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns true if loading was successful, false otherwise.
	VK2D_API virtual bool									VK2D_APIENTRY						WaitUntilLoaded()				= 0;

protected:
	// Internal use only.
	// Return true if loading was successful.
	VK2D_API virtual bool									VK2D_APIENTRY						MTLoad(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource )													= 0;

	// Internal use only.
	VK2D_API virtual void									VK2D_APIENTRY						MTUnload(
		vk2d::_internal::ThreadPrivateResource			*	thread_resource )													= 0;

private:
	// Internal use only.
	// If the resource creates any subresources, they must NOT be manually destroyed.
	// If subresources are deleted anywhere else it can lead to race conditions within resource manager.
	// In case any resource uses sub-sub-resources, the resource manager will handle all cleanup.
	VK2D_API void											VK2D_APIENTRY						DestroySubresources();

	// Internal use only.
	// Subresources can be created either in the resource constructor or MTLoad(). To create a subresource,
	// we can create them just like regular resources, just add parent information.
	VK2D_API void											VK2D_APIENTRY						AddSubresource(
		vk2d::Resource									*	subresource );

public:
	VK2D_API vk2d::Resource								*	VK2D_APIENTRY						GetParentResource();

	// Checks if the resource loading failed.
	// Returns true if failed to load, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						FailedToLoad() const;

	// Gets the thread index that was responsible for loading this resource.
	VK2D_API uint32_t										VK2D_APIENTRY						GetLoaderThread();

	// Checks if the resource was loaded from a file.
	// Returns true if the resource origin is in a file, for example an image, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						IsFromFile() const;

	// Returns the file path where the resource was loaded from,
	// if the was not loaded from a file, returns "".
	VK2D_API const std::vector<std::filesystem::path>	&	VK2D_APIENTRY						GetFilePaths() const;

	VK2D_API bool											VK2D_APIENTRY						IsGood() const;

protected:
	// Internal use only, this tells if this resource should be managed and
	// deleted by the resource manager or another resource. If this returns
	// true then resource manager should not delete this resource directly.
	VK2D_API bool											VK2D_APIENTRY						IsSubResource() const;

	std::atomic_bool										load_function_ran					= {};
	std::atomic_bool										failed_to_load						= {};
	bool													is_good								= {};

private:
	std::mutex												resource_mutex;
	vk2d::_internal::ResourceManagerImpl				*	resource_manager_parent				= {};
	uint32_t												loader_thread						= {};
	std::vector<std::filesystem::path>						file_paths							= {};
	std::vector<vk2d::Resource*>							subresources						= {};
	vk2d::Resource										*	parent_resource						= {};
	bool													is_from_file						= {};
};



}
