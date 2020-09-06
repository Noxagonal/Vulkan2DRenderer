#pragma once

#include "../Core/Common.h"



namespace vk2d {
namespace _internal {
class InstanceImpl;
class TextureImpl;
} // _internal

class Instance;


class Texture
{
public:
	VK2D_API																		Texture(
		vk2d::_internal::InstanceImpl				*	instance );
	VK2D_API																		~Texture();

private:
	std::unique_ptr<vk2d::_internal::TextureImpl>		impl;

	bool												is_good						= {};
};



} // vk2d
