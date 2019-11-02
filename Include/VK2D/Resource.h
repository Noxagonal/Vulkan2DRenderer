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

	VK2D_API bool							VK2D_APIENTRY	IsLoaded() const;
	VK2D_API bool											FailedToLoad() const;

	VK2D_API bool											IsFromFile() const;
	VK2D_API const std::filesystem::path	VK2D_APIENTRY	GetFilePath() const;

protected:
	virtual bool					MTLoad()			= 0;
	virtual bool					MTUnload()			= 0;

	std::atomic_bool				is_loaded			= {};
	std::atomic_bool				failed_to_load		= {};

private:
	uint32_t						loader_thread		= {};
	std::filesystem::path			path				= {};
	bool							is_from_file		= {};
};



}
