#pragma once

#include "../Core/Common.h"

#include <filesystem>
#include <atomic>

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
	VK2D_API virtual																	~Resource()
	{};

	// Checks if the resource is ready to be used.
	// Returns true if resource is loaded, false otherwise.
	VK2D_API virtual bool							VK2D_APIENTRY						IsLoaded()						= 0;

	// Checks if the resource loading failed.
	// Returns true if failed to load, false otherwise.
	VK2D_API bool									VK2D_APIENTRY						FailedToLoad() const;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns true if loading was successful, false otherwise.
	VK2D_API virtual bool							VK2D_APIENTRY						WaitUntilLoaded()				= 0;

	// Gets the thread index that was responsible for loading this resource.
	VK2D_API uint32_t								VK2D_APIENTRY						GetLoaderThread();

	// Checks if the resource was loaded from a file.
	// Returns true if the resource origin is in a file, for example an image, false otherwise.
	VK2D_API bool									VK2D_APIENTRY						IsFromFile() const;

	// Returns the file path where the resource was loaded from,
	// if the was not loaded from a file, returns "".
	VK2D_API const std::filesystem::path			VK2D_APIENTRY						GetFilePath() const;

protected:
	// Internal use only.
	// Return true if loading was successful.
	VK2D_API virtual bool							VK2D_APIENTRY						MTLoad(
		vk2d::_internal::ThreadPrivateResource	*	thread_resource )					= 0;

	// Internal use only.
	VK2D_API virtual void							VK2D_APIENTRY						MTUnload(
		vk2d::_internal::ThreadPrivateResource	*	thread_resource )					= 0;

	std::atomic_bool								load_function_ran					= {};
	std::atomic_bool								failed_to_load						= {};

private:
	uint32_t										loader_thread						= {};
	std::filesystem::path							path								= {};
	bool											is_from_file						= {};
};



}
