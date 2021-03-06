#pragma once

#include "Core/Common.h"

#include "Types/Color.hpp"

#include "Interface/Resources/ResourceBase.h"
#include "Interface/Texture.h"

#include <memory>

namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class TextureResourceImpl;
class ResourceThreadLoadTask;
class ResourceThreadUnloadTask;
} // _internal



/// @brief		vk2d::TextureResource is a resource that can be loaded with vk2d::ResourceManager.
///				It is needed for rendering images on the window or render target texture.
///				To use a texture resource you'll first need to load the texture font from a disk
///				or directly from data, then either draw it directly using Window::DrawTexture()
///				or use it as a part of a vk2d::Mesh and finally draw it by using Window::DrawMesh().
///				All texture resources are multi-layer textures, meaning that a single texture can
///				contain multiple images that can be indexed at runtime.
class TextureResource :
	public vk2d::Texture,
	public vk2d::ResourceBase
{
	friend class vk2d::_internal::TextureResourceImpl;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::ResourceThreadLoadTask;
	friend class vk2d::_internal::ResourceThreadUnloadTask;
	friend class vk2d::_internal::WindowImpl;

	/// @brief		This constructor is meant for internal use only. All resources are
	///				created from vk2d::ResourceManager only.
	/// @note		Multithreading: Any thread, selected managed by vk2d::ResourceManager.
	/// @param[in]	resource_manager
	///				Pointer to resource manager implementation object responsible for
	///				creating this resource.
	/// @param[in]	loader_thread_index
	///				Index to thread pool thread index. Tells which thread pool thread
	///				is responsible for creation and destruction of the internal data
	///				of this resource.
	/// @param[in]	parent_resource
	///				Pointer to a resource that owns this resource. Resources can have
	///				parent and child resources, this is just used to keep track of
	///				resources that are used by other resources and should not be removed
	///				unless the parent resources are removed.
	/// @param[in]	file_paths_listing
	///				A vector of file paths to use when creating the texture.
	///				- Supported file formats are listed in
	///				vk2d::ResourceManager::CreateTextureResource().
	///				- Each file path corresponds to a texture layer in given order. Eg. input
	///				is {{path1}{path2}} then "path1" is texture array layer 0 and "path2" is
	///				texture array layer 1.
	///				- Each texture layer must be the same size. If images in these file paths
	///				are not same size then texture loading will fail.
	VK2D_API																				TextureResource(
		vk2d::_internal::ResourceManagerImpl				*	resource_manager,
		uint32_t												loader_thread,
		vk2d::ResourceBase									*	parent_resource,
		const std::vector<std::filesystem::path>			&	file_paths_listing );

	/// @brief		This constructor is meant for internal use only. All resources are
	///				created from vk2d::ResourceManager only.
	/// @note		Multithreading: Any thread, selected managed by vk2d::ResourceManager.
	/// @param[in]	resource_manager
	///				Pointer to resource manager implementation object responsible for
	///				creating this resource.
	/// @param[in]	loader_thread_index
	///				Index to thread pool thread index. Tells which thread pool thread
	///				is responsible for creation and destruction of the internal data
	///				of this resource.
	/// @param[in]	parent_resource
	///				Pointer to a resource that owns this resource. Resources can have
	///				parent and child resources, this is just used to keep track of
	///				resources that are used by other resources and should not be removed
	///				unless the parent resources are removed.
	/// @param[in]	size
	///				Size of the texture in texels.
	/// @param[in]	texels_listing
	///				Data to use to construct the texture.
	///				- Each texel data vector must be big enough to contain all texel data at
	///				given size. ( must be at least: size.x * size.y ).
	///				- Lenght of the top vector determines the number of arrays the texture has.
	///				- Each top vector index corresponds to the texture array index. Eg. input
	///				is {{data1}{data2}} then "data1" is texture array layer 0 and "data2" is
	///				texture array layer 1.
	///				- Each texture layer must be the same size.
	///				- This data is copied over to internal memory before returning so you do
	///				not need to keep the vector around.
	/// @return 
	VK2D_API																				TextureResource(
		vk2d::_internal::ResourceManagerImpl				*	resource_manager,
		uint32_t												loader_thread,
		vk2d::ResourceBase									*	parent_resource,
		glm::uvec2												size,
		const std::vector<const std::vector<vk2d::Color8>*>	&	texels_listing );

public:
	VK2D_API																				~TextureResource();

	/// @brief		Checks if the resource has been loaded, failed to load or is yet
	///				to be loaded / determined. This function will not wait but returns
	///				immediately with the result.
	/// @note		Multithreading: Any thread.
	/// @return		Status of the resource, see vk2d::ResourceStatus.
	VK2D_API vk2d::ResourceStatus								VK2D_APIENTRY				GetStatus();

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
	VK2D_API vk2d::ResourceStatus								VK2D_APIENTRY				WaitUntilLoaded(
		std::chrono::nanoseconds								timeout						= std::chrono::nanoseconds::max() );

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
	VK2D_API vk2d::ResourceStatus								VK2D_APIENTRY				WaitUntilLoaded(
		std::chrono::steady_clock::time_point					timeout );

	/// @brief		Get texel size of the texture resource.
	/// @return		Texel size of the resource.
	VK2D_API glm::uvec2											VK2D_APIENTRY				GetSize() const;

	/// @brief		All VK2D textures are multi-layer textures. This returns the
	///				amount of layers in the texture.
	/// @return		Number of layers in this texture.
	VK2D_API uint32_t											VK2D_APIENTRY				GetLayerCount() const;

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool												VK2D_APIENTRY				IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::TextureResourceImpl>		impl;
};



}
