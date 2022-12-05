#pragma once

#include <core/Common.hpp>

#include <chrono>
#include <filesystem>



namespace vk2d {
namespace vk2d_internal {

class ResourceManagerImpl;
class ResourceImplBase;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;

} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Status of vk2d resource in multithreaded loader.
///
///				Since VK2D resources are loaded on the background, they will not be immediately available to be used, in this
///				case ResourceStatus will tell if the resource has been loaded, failed to load or still undetermined.
enum class ResourceStatus
{
	/// @brief		Resource is still being loaded by the background thread or not yet started to load.
	UNDETERMINED	= 0,

	/// @brief		Resource has been fully loaded and is ready to be used.
	LOADED,

	/// @brief		Attempt to load the resource has been made but something went wrong.
	FAILED_TO_LOAD,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		VK2D resource is an object that has background loading capability.
/// 
///				Resource is meant to be used in situations where somewhat significant amount of time is required to
///				prepare some data for later use, such as textures. Resource is loaded and unloaded in a separate thread
///				so that the main application can keep running without significant delays. For example you can run the main
///				application while preparing data for the next section of the map or load commonly used textures in the game
///				while user is still in the main menu.
/// 
/// @see		ResourceManager
class ResourceBase {
	friend class vk2d_internal::ResourceManagerImpl;
	friend class vk2d_internal::ResourceImplBase;
	friend class vk2d_internal::ResourceThreadLoadTask;
	friend class vk2d_internal::ResourceThreadUnloadTask;

public:
	// TODO: Figure out how to get rid of Resource virtual destructor to improve ABI compatibility further.

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API virtual										~ResourceBase()						= default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the resource has been loaded, failed to load or is yet to be loaded / determined.
	///
	///				This function will not wait but returns immediately with the result.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus									GetStatus();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block. As soon
	///				as the resource state becomes determined this function will return and code execution can continue.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined. Default value is std::chrono::nanoseconds::max() which
	///				makes this function wait indefinitely.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	///				Resource status can only be undetermined if timeout was given.
	VK2D_API ResourceStatus									WaitUntilLoaded(
		std::chrono::nanoseconds							timeout								= std::chrono::nanoseconds::max()
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block.	As soon
	///				as the resource state becomes determined this function will return and code execution can continue.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined.
	/// 
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus									WaitUntilLoaded(
		std::chrono::steady_clock::time_point				timeout
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Get parent resource.
	/// 
	///				Resources can have parent and child resources, this is just used to keep track of resources that are used by
	///				other resources and should not be removed unless the parent resources are removed.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Parent resource that owns this resource.
	VK2D_API ResourceBase								*	GetParentResource_DEPRECATED();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the resource was loaded from a file or from data.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if resource origin is a file, eg. a .png image, false if resource origin is data, eg. a list of texels
	///				given to the resource manager to create a texture from.
	VK2D_API bool											IsFromFile() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// brief		If resource origin is a file then this returns all the file paths where to load the resource from.
	///
	///				Some resources can use multiple files to create a single resource object, eg. Texture can use multiple
	///				files where each file will be loaded as a texture layer. Wether a resource can use multiple file paths depends
	///				on the capabilities of the specific resource.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @see		Resource::IsFromFile()
	/// 
	/// @return		List of file paths.
	VK2D_API const std::vector<std::filesystem::path>	&	GetFilePaths() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool											IsGood() const;

protected:

	vk2d_internal::ResourceImplBase						*	resource_impl;
};



}
