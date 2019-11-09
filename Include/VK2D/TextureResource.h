#pragma once

#include "Common.h"
#include "Resource.h"

#include <memory>

namespace vk2d {

namespace _internal {
class RendererImpl;
class TextureResourceImpl;
class LoadTask;
class UnloadTask;
}


class TextureResource :
	public Resource
{
	friend class _internal::TextureResourceImpl;
	friend class _internal::ResourceManagerImpl;
	friend class _internal::LoadTask;
	friend class _internal::UnloadTask;

	VK2D_API									TextureResource(
		_internal::ResourceManagerImpl		*	resource_manager_parent );

public:
	VK2D_API									~TextureResource();

	VK2D_API bool				VK2D_APIENTRY	IsLoaded();
	VK2D_API bool				VK2D_APIENTRY	WaitUntilLoaded();

	VK2D_API bool				VK2D_APIENTRY	IsGood();

protected:
	VK2D_API virtual bool		VK2D_APIENTRY	MTLoad(
		_internal::ThreadPrivateResource	*	thread_resource );

	VK2D_API virtual void		VK2D_APIENTRY	MTUnload(
		_internal::ThreadPrivateResource	*	thread_resource );

private:
	std::unique_ptr<_internal::TextureResourceImpl>		impl					= {};

	bool										is_good							= {};
};



}
