#pragma once

#include "../Core/Common.h"

#include "../Types/Color.hpp"

#include <memory>

namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class RenderTargetTextureImpl;
class SamplerImpl;
} // _internal

class Instance;


/// @brief		Tells which filter to use when fetching the texel from a texture at given
///				UV coordinates, this is needed when texture is smaller or larger than the
///				output, eg. small texture is used on a shape that that fills most of the
///				window surface has an appearance of a blurry texture, this 
enum class SamplerFilter {
	NEAREST,
	LINEAR,

//	CUBIC, // disabled for now, not enough hardware support. This is doable in shader though so I might "software" implement this later.
};

enum class SamplerMipmapMode {
	NEAREST,
	LINEAR,
};

enum class SamplerAddressMode {
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_CLAMP_TO_EDGE,
};

struct SamplerCreateInfo {
	vk2d::SamplerFilter					minification_filter				= vk2d::SamplerFilter::LINEAR;		///< 
	vk2d::SamplerFilter					magnification_filter			= vk2d::SamplerFilter::LINEAR;		///< 
	vk2d::SamplerMipmapMode				mipmap_mode						= vk2d::SamplerMipmapMode::LINEAR;	///< 
	vk2d::SamplerAddressMode			address_mode_u					= vk2d::SamplerAddressMode::REPEAT;	///< 
	vk2d::SamplerAddressMode			address_mode_v					= vk2d::SamplerAddressMode::REPEAT;	///< 
	vk2d::Colorf						border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };		///< 
	bool								mipmap_enable					= true;								///< 
	float								mipmap_max_anisotropy			= 16.0f;							///< 
	float								mipmap_level_of_detail_bias		= 0.0f;								///< 
	float								mipmap_min_level_of_detail		= 0.0f;								///< 
	float								mipmap_max_level_of_detail		= 128.0f;							///< 
};



class Sampler {
	friend class vk2d::_internal::InstanceImpl;
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

	VK2D_API																			Sampler(
		vk2d::_internal::InstanceImpl				*	instance,
		const vk2d::SamplerCreateInfo				&	create_info );

public:
	VK2D_API																			~Sampler();

	VK2D_API bool										VK2D_APIENTRY					IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::SamplerImpl>		impl;
};



} // vk2d
