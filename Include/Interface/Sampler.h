#pragma once

#include "Core/Common.h"

#include "Types/Color.hpp"

#include <memory>

namespace vk2d {

namespace _internal {
class InstanceImpl;
class WindowImpl;
class RenderTargetTextureImpl;
class SamplerImpl;
} // _internal

class Instance;


/// @brief		This tells the sampler which filter to use when fetching the texel from a texture at given
///				UV coordinates. This is needed when texture is smaller or larger than the
///				output.
///				For example, if a small texture is stretched to fill the window surface,
///				the window pixel fragments can be generated inbetween texture texels. In
///				this case the texels of the texture are bigger than the texels of the
///				window. This is called "magnification".
///				Another operation is "minification" where texture texels are smaller than
///				the output texels. In this case a minification filter is used and
///				technically it works much the same way as magnification, that is, the
///				output pixel fragment can land anywhere inbetween texture texels.
enum class SamplerFilter : uint32_t
{
	/// @brief		This gets the color value from a texture texel closest to the fragment
	///				we're currently processing. <br>
	///				<table>
	///				<tr><td> Magnification from 8*8 texel image:		<td><img src="SamplerMagFilterNearest.png">
	///				<tr><td> Minification from 1024*1024 texel image:	<td><img src="SamplerMinFilterNearest.png">
	///				</table>
	NEAREST,

	/// @brief		Linearly interpolates between 4 texture texels, where the fragment landed
	///				inbetween of. Here mipmapping is disabled to show what happens when
	///				a large enough texture is rendered small. The final render appears
	///				grainy and it gets worse the smaller the image is rendered. To
	///				permanently get rid of this type of graininess you will have to
	///				enable mipmapping. <br>
	///				<table>
	///				<tr><td> Magnification from 8*8 texel image:		<td><img src="SamplerMagFilterLinear.png">
	///				<tr><td> Minification from 1024*1024 texel image:	<td><img src="SamplerMinFilterLinear.png">
	///				</table>
	LINEAR,
};

/// @brief		Mipmap mode tells how texture texel color is sampled between different
///				mipmap levels. Mipmaps are basically a collection of sub-textures where
///				each level is half the size of the previous mip level. Mip level 0 is the original
///				texture, mip level 1 is half of the size of the original, mip level 2 is
///				half the size of mip level 1 and so on until we reach the last mip level
///				which will be only one texel in size. <br>
///				On the left is "linear" mipmap mode, on the right is "nearest".
///				(To make it easier to see what's happening, minification and magnification
///				filters have been set to "nearest" and mip level bias is manually adjusted) <br>
///				<img src="SamplerMipmapMode.apng">
enum class SamplerMipmapMode : uint32_t
{
	/// @brief		The fragment gets the texture texel color from mip level that's "closest" to the calculated
	///				level-of-detail for a given fragment.
	NEAREST,
	/// @brief		The fragment gets linearly interpolated texel color between different mip levels,
	///				based on the calculated level-of-detail for a given fragment.
	LINEAR,
};

/// @brief		This tells what color is returned when texture texel is fetched outside the
///				bounds of the texture area (when UV coordinate is outside 0.0 to 1.0 range).
///				Images below show what happens in each address mode, the center image is in
///				UV range 0.0 to 1.0, the light green point shows where UV coordinate
///				0.0, 0.0 lands.
enum class SamplerAddressMode : uint32_t
{
	/// @brief	Texture is repeated. <br>
	///			<img src="SamplerAddressMode_Repeat.png">
	REPEAT,

	/// @brief	Texture is repeated and mirrored to match surroundings. <br>
	///			<img src="SamplerAddressMode_MirroredRepeat.png">
	MIRRORED_REPEAT,

	/// @brief	Closest texture texel is repeated. <br>
	///			<img src="SamplerAddressMode_ClampToEdge.png">
	CLAMP_TO_EDGE,

	/// @brief	Negative UV range -1.0 to -0.0 is mirrored and everything outside UV range
	///			-1.0 to +1.0 gets it's color from the closest texture texel. <br>
	///			<img src="SamplerAddressMode_MirrorClampToEdge.png">
	MIRROR_CLAMP_TO_EDGE,

	/// @brief	Fragment outside UV range 0.0 to 1.0 gets border color (green in this example).
	///			See: vk2d::SamplerCreateInfo::border_color <br>
	///			<img src="SamplerAddressMode_ClampToBorder.png">
	CLAMP_TO_BORDER,
};

/// @var		vk2d::SamplerCreateInfo::minification_filter
/// @brief		See: vk2d::SamplerFilter

/// @var		vk2d::SamplerCreateInfo::magnification_filter
/// @brief		See: vk2d::SamplerFilter

/// @var		vk2d::SamplerCreateInfo::mipmap_mode
/// @brief		See: vk2d::SamplerMipmapMode

/// @var		vk2d::SamplerCreateInfo::address_mode_u
/// @brief		Address mode on texture horizontal axis. See: vk2d::SamplerAddressMode

/// @var		vk2d::SamplerCreateInfo::address_mode_v
/// @brief		Address mode on texture vertical axis. See: vk2d::SamplerAddressMode

/// @var		vk2d::SamplerCreateInfo::border_color
/// @brief		When using clamp to border address mode, use this color outside UV range 0.0 to 1.0.

/// @var		vk2d::SamplerCreateInfo::anisotropy_enable
///	@brief		Anisotropic filtering results the best quality texture filtering when you want
///				smooth results over a surface that shrinks or expands different amounts at different
///				locations. In the animation below, on the left an anisotropic filtering is enabled,
///				on the right it's disabled. (For the purpose of visualization, minification and
///				magnification filters have been set to "nearest". Sampler mipmap mode has been set
///				to "linear", and mipmap bias is manually adjusted over time) <br>
///				<img src="SamplerAnisotropicFiltering.apng">

/// @var		vk2d::SamplerCreateInfo::mipmap_max_anisotropy
/// @brief		Maximum anisotropic filtering affects the quality of the final anisotropic filter
///				results. The higher this value is, the more clear the final image appears in areas
///				that are shrunk down. Supported maximum anisotropy depends on the hardware but is usually
///				around 16.0. This is a pretty cheap operation for modern hardware, so unless you need
///				this for artistic purposes you should just leave it at 16.0. <br>
///				The animation below shows max anisotropic value changing over time. (For the purpose of
///				visualization the image on the left is rendered with minification and magnification
///				filters set to "nearest", and level-of-detail bias is set to 4.3. On the right everything
///				is "linear" and level-of-detail bias set to 0.0) <br>
///				<img src="SamplerMaxAnisotropy.apng">

/// @var		vk2d::SamplerCreateInfo::mipmap_level_of_detail_bias
/// @brief		Level-of-detail bias is a way to manually nudge towards one mip level or another.
///				0.0 is the default and usually what you want. Positive values use higher mip level,
///				meaning less details visible, while negative values use lower mip levels, which can give
///				more visible detail but may also introduce graininess. <br>
///				<img src="SamplerLODBias.apng">

/// @var		vk2d::SamplerCreateInfo::mipmap_min_level_of_detail
/// @brief		This is the minimum mipmap level to use. 0 is the original texture, so minimum 0.0 means that the
///				original texture can be used. If you set this to 1.0 then only mip level 1 and up
///				are used. The higher this value the less detail is made available.

/// @var		vk2d::SamplerCreateInfo::mipmap_max_level_of_detail
/// @brief		This is the maximum mipmap level to use. A texture has a number of mipmap images, higher mip levels
///				have less detail than the lower ones. For example a texture that's 512*512 texels has
///				mip levels 0 = {512*512}, 1 = {256*256}, 2 = {128*128} ... 8 = {2*2}, 9 = {1*1} giving
///				us 10 mip levels. The higher the resolution of the texture the more mip levels we have.
///				This value cuts use of the lower resolution mip levels at a specific level.

/// @brief		Parameters to construct a vk2d::Sampler.
struct SamplerCreateInfo {
	vk2d::SamplerFilter					minification_filter				= vk2d::SamplerFilter::LINEAR;
	vk2d::SamplerFilter					magnification_filter			= vk2d::SamplerFilter::LINEAR;
	vk2d::SamplerMipmapMode				mipmap_mode						= vk2d::SamplerMipmapMode::LINEAR;
	vk2d::SamplerAddressMode			address_mode_u					= vk2d::SamplerAddressMode::REPEAT;
	vk2d::SamplerAddressMode			address_mode_v					= vk2d::SamplerAddressMode::REPEAT;
	vk2d::Colorf						border_color					= { 0.0f, 0.0f, 0.0f, 1.0f };
	bool								anisotropy_enable				= true;
	float								mipmap_max_anisotropy			= 16.0f;
	float								mipmap_level_of_detail_bias		= 0.0f;
	float								mipmap_min_level_of_detail		= 0.0f;
	float								mipmap_max_level_of_detail		= 32.0f;
};



/// @brief		A sampler is used to determine how to fetch texture color values at specific locations
///				and what filters should be used when fetches happen inbetween texture texels.
class Sampler {
	friend class vk2d::_internal::InstanceImpl;
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

	/// @brief		This object should not be directly constructed, it is created by
	///				vk2d::Instance::CreateSampler().
	/// @param[in]	instance
	///				Pointer to instance that owns this object.
	/// @param[in]	create_info
	///				Reference to vk2d::SamplerCreateInfo object defining parameters
	///				for this render target texture.
	VK2D_API																			Sampler(
		vk2d::_internal::InstanceImpl				*	instance,
		const vk2d::SamplerCreateInfo				&	create_info );

public:
	VK2D_API																			~Sampler();

	/// @brief		VK2D class object checker function.
	/// @note		Multithreading: Any thread.
	/// @return		true if class object was created successfully,
	///				false if something went wrong
	VK2D_API bool										VK2D_APIENTRY					IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::SamplerImpl>		impl;
};



} // vk2d
