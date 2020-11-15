#pragma once

#include "../../Core/Common.h"

namespace vk2d {

namespace _internal {
class ResourceManagerImpl;
class ResourceImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
}

class Resource {
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::ResourceImpl;
	friend class vk2d::_internal::ResourceThreadLoadTask;
	friend class vk2d::_internal::ResourceThreadUnloadTask;

public:
	// TODO: Figure out how to get rid of Resource virtual destructor to improve ABI compatibility further.
	VK2D_API virtual																			~Resource()						= default;

	// Checks if the resource is ready to be used.
	// Returns true if resource is loaded, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						IsLoaded();

	// Blocks until the resource is ready to be used or an error happened.
	// Returns true if loading was successful, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						WaitUntilLoaded();

	VK2D_API vk2d::Resource								*	VK2D_APIENTRY						GetParentResource();

	// Checks if the resource loading failed.
	// Returns true if failed to load, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						FailedToLoad() const;

	// Checks if the resource was loaded from a file.
	// Returns true if the resource origin is in a file, for example an image, false otherwise.
	VK2D_API bool											VK2D_APIENTRY						IsFromFile() const;

	// Returns the file path where the resource was loaded from,
	// if the was not loaded from a file, returns "".
	VK2D_API const std::vector<std::filesystem::path>	&	VK2D_APIENTRY						GetFilePaths() const;

	VK2D_API bool											VK2D_APIENTRY						IsGood() const;

protected:
	vk2d::_internal::ResourceImpl						*	resource_impl						= {};
};



}
