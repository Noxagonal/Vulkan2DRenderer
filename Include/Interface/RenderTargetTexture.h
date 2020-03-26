#pragma once

#include "../Core/Common.h"

#include "../../Include/Interface/RenderPrimitives.h"

#include "../../Include/Interface/RenderTargetCommon.hpp"
#include "../../Include/Interface/Mesh.h"

#include <memory>

namespace vk2d {

class TextureResource;
class FontResource;
class Sampler;

namespace _internal {

class InstanceImpl;
class RenderTargetTextureImpl;

} // _internal



struct RenderTargetTextureCreateInfo
{
	vk2d::RenderCoordinateSpace				coordinate_space			= vk2d::RenderCoordinateSpace::TEXEL_SPACE;	// Coordinate space of the render target.
	vk2d::Vector2u							size						= vk2d::Vector2u( 512, 512 );				// Render target texture size.
	vk2d::Multisamples						samples						= vk2d::Multisamples::SAMPLE_COUNT_1;		// Multisample count.
	bool									enable_blur					= false;									// Ability to blur the texture on the fly.
};



class RenderTargetTexture
{
	friend class vk2d::_internal::InstanceImpl;

	VK2D_API																				RenderTargetTexture(
		vk2d::_internal::InstanceImpl						*	instance,
		const vk2d::RenderTargetTextureCreateInfo			&	create_info );

public:
	VK2D_API																				~RenderTargetTexture();

	VK2D_API void												VK2D_APIENTRY				SetSize(
		vk2d::Vector2u											new_size );

	VK2D_API vk2d::Vector2u										VK2D_APIENTRY				GetSize();

	// Begins the render operations. You must call this before using any drawing commands.
	// For best performance you should calculate game logic first, when you're ready to draw
	// call this function just before your first draw command.
	VK2D_API bool												VK2D_APIENTRY				BeginRender();

	// Ends the rendering operations. You must call this after you're done drawing.
	// This will display the results on screen.
	VK2D_API bool												VK2D_APIENTRY				EndRender();
	/*
	VK2D_API void												VK2D_APIENTRY				DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channels,
		bool													filled						= true,
		vk2d::TextureResource								*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	VK2D_API void												VK2D_APIENTRY				DrawLineList(
		const std::vector<vk2d::VertexIndex_2>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channels,
		vk2d::TextureResource								*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	VK2D_API void												VK2D_APIENTRY				DrawPointList(
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channels,
		vk2d::TextureResource								*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	VK2D_API void												VK2D_APIENTRY				DrawTexture(
		vk2d::Vector2f											top_left,
		vk2d::Vector2f											bottom_right,
		vk2d::TextureResource								*	texture,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh									&	mesh );
		*/

	VK2D_API bool												VK2D_APIENTRY				IsGood();

private:
	std::unique_ptr<vk2d::_internal::RenderTargetTextureImpl>	impl;
	bool														is_good						= {};
};



} // vk2d
