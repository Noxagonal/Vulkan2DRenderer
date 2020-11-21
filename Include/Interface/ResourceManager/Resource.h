#pragma once

#include "../../Core/Common.h"

namespace vk2d {

namespace _internal {
class ResourceManagerImpl;
class ResourceImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
}



enum class ResourceStatus
{
	UNDETERMINED		= 0,
	LOADED,
	FAILED_TO_LOAD,
};



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
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						GetStatus();

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max() );

	// Blocks until the resource is ready to be used or an error happened.
	// Returns the new status of the resource, it's guaranteed to not be undetermined.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	VK2D_API vk2d::Resource								*	VK2D_APIENTRY						GetParentResource();

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
