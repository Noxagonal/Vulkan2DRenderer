#pragma once

#include "../Core/Common.h"
#include "Resource.h"

#include <memory>

namespace vk2d {

namespace _internal {
class RendererImpl;
class WindowImpl;
class TextureResourceImpl;
class LoadTask;
class UnloadTask;
} // _internal



struct Texel {
	uint8_t			r;
	uint8_t			g;
	uint8_t			b;
	uint8_t			a;
};



class TextureResource :
	public Resource
{
	friend class vk2d::_internal::TextureResourceImpl;
	friend class vk2d::_internal::ResourceManagerImpl;
	friend class vk2d::_internal::LoadTask;
	friend class vk2d::_internal::UnloadTask;
	friend class vk2d::_internal::WindowImpl;

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
