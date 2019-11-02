#pragma once

#include "Common.h"

#include <filesystem>
#include <atomic>

namespace vk2d {

namespace _internal {
class ResourceManagerImpl;
class LoadTask;
class UnloadTask;
}


class Resource {
	friend class _internal::ResourceManagerImpl;
	friend class _internal::LoadTask;
	friend class _internal::UnloadTask;

public:
	VK2D_API virtual										~Resource()
	{};

	// Checks if the resource is ready to be used.
	// Returns true if resource is loaded, false otherwise.
	// TODO: MAKE THIS PURE VIRTUAL, this should invoke some more complicated checks.
	VK2D_API bool							VK2D_APIENTRY	IsLoaded() const;

	// Checks if the resource loading failed.
	// Returns true if failed to load, false otherwise.
	VK2D_API bool							VK2D_APIENTRY	FailedToLoad() const;

	// Blocks until the resource is ready to be used or an error happened.
	// Returns true if loading was successful, false otherwise.
	VK2D_API bool							VK2D_APIENTRY	WaitUntilLoaded() const;

	// Checks if the resource was loaded from a file.
	// Returns true if the resource origin is in a file, for example an image, false otherwise.
	VK2D_API bool							VK2D_APIENTRY	IsFromFile() const;

	// Returns the file path where the resource was loaded from,
	// if the was not loaded from a file, returns "".
	VK2D_API const std::filesystem::path	VK2D_APIENTRY	GetFilePath() const;

protected:
	// Internal use only.
	// Return true if loading was successful.
	VK2D_API virtual bool					VK2D_APIENTRY	MTLoad()			= 0;

	// Internal use only.
	VK2D_API virtual void					VK2D_APIENTRY	MTUnload()			= 0;

	std::atomic_bool						is_loaded							= {};
	std::atomic_bool						failed_to_load						= {};

private:
	uint32_t								loader_thread						= {};
	std::filesystem::path					path								= {};
	bool									is_from_file						= {};
};



}
