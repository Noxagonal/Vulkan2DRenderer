#pragma once

#include "Core/Common.h"



namespace vk2d {

namespace _internal {
class ResourceManagerImpl;
class ResourceImplBase;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
}



/// @brief		Since VK2D resources are loaded on the background, they will not be
///				immediately available to be used, in this case vk2d::ResourceStatus
///				will tell if the resource has been loaded, failed to load or still
///				undetermined.
enum class ResourceStatus
{
	UNDETERMINED		= 0,	///< Loading is still ongoing or not yet attempted.
	LOADED,						///< Resource has been fully loaded and is ready to be used.
	FAILED_TO_LOAD,				///< Attempt to load the resource has been made but something went wrong.
};



/// @brief		VK2D resource is an object that has background loading capability.
///				vk2d::Resource is meant to be used in situations where somewhat significant
///				amount of time is required to prepare some data for later use, such as
///				textures. vk2d::Resource is loaded and unloaded in a separate thread
///				so that the main application can keep running without significant delays.
///				For example you can run the main application while preparing data for
///				the next section of the map or load commonly used textures in the game
///				while user is still in the main menu.
/// @see		vk2d::ResourceManager
class ResourceBase {
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::ResourceImplBase;
	friend class vk2d::_internal::ResourceThreadLoadTask;
	friend class vk2d::_internal::ResourceThreadUnloadTask;

public:
	// TODO: Figure out how to get rid of Resource virtual destructor to improve ABI compatibility further.
	VK2D_API virtual																			~ResourceBase()						= default;

	/// @brief		Checks if the resource has been loaded, failed to load or is yet
	///				to be loaded / determined. This function will not wait but returns
	///				immediately with the result.
	/// @note		Multithreading: Any thread.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						GetStatus();

	/// @brief		Waits for the resource to load on the calling thread before
	///				continuing execution. For as long as the resource status is
	///				undetermined or timeout hasn't been met this function will block.
	///				As soon as the resource state becomes determined this function
	///				will return and code execution can continue.
	/// @note		Multithreading: Any thread.
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state
	///				at timeout it will return anyways and the result
	///				will tell that the resource is still undetermined.
	///				Default value is std::chrono::nanoseconds::max() which makes
	///				this function wait indefinitely.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	///				Resource status can only be undetermined if timeout was given.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max() );

	/// @brief		Waits for the resource to load on the calling thread before
	///				continuing execution. For as long as the resource status is
	///				undetermined or timeout hasn't been met this function will block.
	///				As soon as the resource state becomes determined this function
	///				will return and code execution can continue.
	/// @note		Multithreading: Any thread.
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state
	///				at timeout it will return anyways and the result
	///				will tell that the resource is still undetermined.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	VK2D_API vk2d::ResourceStatus							VK2D_APIENTRY						WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout );

	/// @brief		Resources can have parent and child resources, this is just used
	///				to keep track of resources that are used by other resources and
	///				should not be removed unless the parent resources are removed.
	/// @note		Multithreading: Any thread.
	/// @return		Parent resource that owns this resource.
	VK2D_API vk2d::ResourceBase							*	VK2D_APIENTRY						GetParentResource();

	/// @brief		Checks if the resource was loaded from a file or from data.
	/// @note		Multithreading: Any thread.
	/// @return		true if resource origin is a file, eg. a .png image, false if
	///				resource origin is data, eg. a list of texels given to the
	///				resource manager to create a texture from.
	VK2D_API bool											VK2D_APIENTRY						IsFromFile() const;

	/// @brief		If resource origin is a file then this returns all the file paths
	///				where to load the resource from. Some resources can use multiple
	///				files to create a single resource object, eg. vk2d::Texture can
	///				use multiple files where each file will be loaded as a texture layer.
	///				Wether a resource can use multiple file paths depends on the
	///				capabilities of the specific resource.
	/// @note		Multithreading: Any thread.
	/// @see		vk2d::Resource::IsFromFile()
	/// @return		List of file paths.
	VK2D_API const std::vector<std::filesystem::path>	&	VK2D_APIENTRY						GetFilePaths() const;

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool											VK2D_APIENTRY						IsGood() const;

protected:

	vk2d::_internal::ResourceImplBase					*	resource_impl						= {};
};



}
