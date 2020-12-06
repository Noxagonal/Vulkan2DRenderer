#pragma once

#include "../Core/Common.h"

#include "../Types/BlurType.h"
#include "../Types/Vector2.hpp"
#include "../Types/Rect2.hpp"
#include "../Types/Matrix4.hpp"
#include "../Types/Transform.h"
#include "../Types/Color.hpp"
#include "../Types/Multisamples.h"
#include "../Types/RenderCoordinateSpace.hpp"

#include "Texture.h"

#include <memory>

namespace vk2d {

class Sampler;
class Mesh;

namespace _internal {

class InstanceImpl;
class RenderTargetTextureImpl;

} // _internal



/// @brief		Creation parameters for the vk2d::RenderTargetTexture.
struct RenderTargetTextureCreateInfo
{
	vk2d::RenderCoordinateSpace				coordinate_space			= vk2d::RenderCoordinateSpace::TEXEL_SPACE;	///< Coordinate system to be used, see vk2d::RenderCoordinateSpace.
	vk2d::Vector2u							size						= vk2d::Vector2u( 512, 512 );				///< Render target texture texel size.
	vk2d::Multisamples						samples						= vk2d::Multisamples::SAMPLE_COUNT_1;		///< Multisampling, must be a singular value, see vk2d::Multisamples. Uses more GPU resources if higher than 1.
	bool									enable_blur					= false;									///< Enable ability to blur the render target texture at the end of the render. Uses more GPU resources if enabled.
};



/// @brief		Render target texture is a texture that can be used like a regular texture
///				and also rendered to. It is a lot more resource heavy than a regular texture,
///				however it enables a lot of flexibility in the host application. You can also
///				blur the render target texture at the end of its rendering operation.
///				Render target texture is primarily meant for realtime use where you re-render
///				it each frame (and thus it also uses a fair bit of memory as it is
///				double-buffered), however you do not need to re-render it each frame, just
///				re-render it whenever the contents of the render target texture change.
///				You can of course render to a render target texture only once in it's
///				lifetime and just use it like a regular texture throughout the application,
///				however in this situation, if you can, you should instead create a
///				vk2d::TextureResource.
class RenderTargetTexture :
	public vk2d::Texture
{
	friend class vk2d::_internal::InstanceImpl;

	/// @brief		This object should not be directly constructed, it is created by
	///				vk2d::Instance::CreateRenderTargetTexture().
	/// @note		Multithreading: Main thread only.
	/// @param[in]	instance
	///				Pointer to instance that owns this render target texture.
	/// @param[in]	create_info
	///				Reference to vk2d::RenderTargetTextureCreateInfo object defining
	///				parameters for this render target texture.
	VK2D_API																				RenderTargetTexture(
		vk2d::_internal::InstanceImpl						*	instance,
		const vk2d::RenderTargetTextureCreateInfo			&	create_info );

public:
	VK2D_API																				~RenderTargetTexture();

	/// @brief		Sets the texel size of the render target texture, recreates all
	///				internal resources which means this render target texture will
	///				synchronize which will cause some microstutters during hot
	///				render loop. If you can, prefer setting the render target
	///				texture size high enough that you don't need to call this
	///				function. If you do not know the reasonable estimate for the
	///				maximum size at creation time, try to reserve some extra texel
	///				space around the planned content and minimize calls to this
	///				function.
	/// @note		Multithreading: Main thread only.
	/// @warning
	///				- Do not call inside it's own render block. It is an error to call
	///				<tt><b> this->SetSize()		</b></tt> function between
	///				<tt><b> this->BeginRender()	</b></tt> and
	///				<tt><b> this->EndRender()	</b></tt>
	///				- If called after vk2d::RenderTargetTexture::EndRender() and
	///				before vk2d::Window::EndRender() where this render target
	///				texture is used, render target texture render contents may be
	///				lost.
	/// @param[in]	new_size
	///				New texel size of this render target texture.
	VK2D_API void												VK2D_APIENTRY				SetSize(
		vk2d::Vector2u											new_size );

	/// @brief		Gets the texel size of the render target texture.
	/// @note		Multithreading: Main thread only.
	/// @return		Size of the render target texture in texels.
	VK2D_API vk2d::Vector2u										VK2D_APIENTRY				GetSize() const;

	/// @brief		Gets the texture layer count. This is provided for compatibility
	///				with vk2d::Texture. It always returns 1.
	/// @note		Multithreading: Any thread.
	/// @return		Number of texture layers. (Render target texture always returns 1)
	VK2D_API uint32_t											VK2D_APIENTRY				GetLayerCount() const;

	/// @brief		Tells if the texture data is ready to be used.
	/// @note		Multithreading: Main thread only.
	/// @return		true if texture data is ready to be used, false otherwise.
	VK2D_API bool												VK2D_APIENTRY				IsTextureDataReady();

	// Begins the render operations. You must call this before using any drawing commands.
	// For best performance you should calculate game logic first, when you're ready to draw
	// call this function just before your first draw command.

	/// @brief		Begins the render operations. This signals the beginning of the render block and
	///				you must call this before using any drawing commands.
	///				For best performance you should calculate game logic first, when you're ready to draw
	///				call this function just before your first draw command. Every draw call must be
	///				between this and vk2d::RenderTargetTexture::EndRender().
	/// @see		vk2d::RenderTargetTexture::EndRender()
	/// @note		Multithreading: Main thread only.
	/// @return		true on success, false if something went wrong.
	VK2D_API bool												VK2D_APIENTRY				BeginRender();

	// Ends the rendering operations. You must call this after you're done drawing.
	// This will display the results on screen.

	/// @brief		Ends the rendering operations. This signals the end of the render block and you must
	///				call this after you're done drawing everything in order to display the results on the
	///				render target texture.
	/// @note
	///				- Multithreading: Main thread only.
	///				- For performance testing: Timing this function is always useless as nothing is actually
	///				rendered here. Rendering of render target textures is only submitted to the GPU at
	///				vk2d::Window::EndRender() for performance reasons, also in case of multi-level render
	///				target texture dependencies these dependencies are only resolved once window surface
	///				is rendered to.
	/// @param[in]	blur_amount
	///				Amount of vertical and horisontal blur in texels. Input is in floats for smoothness.
	/// @param[in]	blur_type
	///				Type of blur to use, see vk2d::BlurType for more info.
	/// @return		true on success, false if something went wrong.
	VK2D_API bool												VK2D_APIENTRY				EndRender(
		vk2d::Vector2f											blur_amount					= {},
		vk2d::BlurType											blur_type					= vk2d::BlurType::GAUSSIAN );

	/// @brief		Draw triangles directly. This is the best option if you wish to render vertices directly.
	///				Every other VK2D draw operation internally calls this function to do the actual drawing.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	indices
	///				Indices are the index numbers into the vertices list where each vertex is considered the
	///				corner point of the triangle.
	/// @param[in]	vertices
	///				Vertices are the corners of a triangle. A vertex also defines more data related to that
	///				corner and that data gets interpolated inside the triangle.
	/// @param[in]	texture_channel_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those
	///				texture layers have per vertex. The size of this vector parameter should be the number
	///				of texture layers multiplied by number of vertices. Data should be formatted so that for
	///				each vertex, weight of each texture layer is given. <br>
	///				For example: a simple triangle with texture with 2 layers should have this order:
	///				(v = vertex, t = texture layer) <br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1][v2 t0][v2 t1]</tt>
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior
	///				to rendering them. If none are provided then a default transformation is applied.
	///				If multiple transformations are provided then this draw call will render multiple
	///				times, each time using a different transformation, this is called instanced render.
	///				The benefit of instanced render is that it saves CPU time and memory when you need to
	///				render lots of similar looking shapes.
	/// @param[in]	filled
	///				Tells if the inside of the triangle should be filled, true for filled, false for wireframe.
	/// @param[in]	texture
	///				Pointer to any object that implements vk2d::Texture interface, such as
	///				vk2d::TextureResource or vk2d::RenderTargetTexture.
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	VK2D_API void												VK2D_APIENTRY				DrawTriangleList(
		const std::vector<vk2d::VertexIndex_3>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>					&	transformations				= { vk2d::Matrix4f( 1.0f ) },
		bool													filled						= true,
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	/// @brief		Draw lines directly. This option is useful when you want to draw simple lines.
	///				Eg. for debugging. For more sophisticated rendering you should prefer rendering triangles.
	///				Every other VK2D draw operation internally calls this function to do the actual drawing.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	indices
	///				Indices are the index numbers into the vertices list where each vertex is considered the
	///				end point of the line.
	/// @param[in]	vertices
	///				Vertices are the end points of a line. A vertex also defines more data related to that
	///				end point and that data gets interpolated for each point along the line.
	/// @param[in]	texture_channel_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those
	///				texture layers have per vertex. The size of this vector parameter should be the number
	///				of texture layers multiplied by number of vertices. Data should be formatted so that for
	///				each vertex, weight of each texture layer is given. <br>
	///				For example: a simple line with texture with 2 layers should have this order:
	///				(v = vertex, t = texture layer) <br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1]</tt>
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior
	///				to rendering them. If none are provided then a default transformation is applied.
	///				If multiple transformations are provided then this draw call will render multiple
	///				times, each time using a different transformation, this is called instanced render.
	///				The benefit of instanced render is that it saves CPU time and memory when you need to
	///				render lots of similar looking shapes.
	/// @param[in]	texture
	///				Pointer to any object that implements vk2d::Texture interface, such as
	///				vk2d::TextureResource or vk2d::RenderTargetTexture.
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	/// @param[in]	line_width
	///				Lines can be drawn thicker or thinner than 1 texel width, however this applies to all
	///				lines being drawn at once and end points are not rounded.
	VK2D_API void												VK2D_APIENTRY				DrawLineList(
		const std::vector<vk2d::VertexIndex_2>				&	indices,
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>					&	transformations				= { vk2d::Matrix4f( 1.0f ) },
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr,
		float													line_width					= 1.0f );

	/// @brief		Draw points directly. This option is mostly provided for completeness. This is not really
	///				useful in most cases, however vertices can have different sizes which can be used for some
	///				particle effects or similar.
	///				Every other VK2D draw operation internally calls this function to do the actual drawing.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	vertices
	///				Vertices are the points to draw.
	/// @param[in]	texture_channel_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those
	///				texture layers have per vertex. The size of this vector parameter should be the number
	///				of texture layers multiplied by number of vertices. Data should be formatted so that for
	///				each vertex, weight of each texture layer is given. <br>
	///				For example: 2 points with texture with 2 layers should have this order:
	///				(v = vertex, t = texture layer) <br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1]</tt>
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior
	///				to rendering them. If none are provided then a default transformation is applied.
	///				If multiple transformations are provided then this draw call will render multiple
	///				times, each time using a different transformation, this is called instanced render.
	///				The benefit of instanced render is that it saves CPU time and memory when you need to
	///				render lots of similar looking shapes. For clarification: In case of drawing points this
	///				also means that if multiple transformation matrices are given then every point is also
	///				drawn multiple times, eg. 10 vertices with 2 transformations means 20 points drawn.
	/// @param[in]	texture
	///				Pointer to any object that implements vk2d::Texture interface, such as
	///				vk2d::TextureResource or vk2d::RenderTargetTexture.
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	VK2D_API void												VK2D_APIENTRY				DrawPointList(
		const std::vector<vk2d::Vertex>						&	vertices,
		const std::vector<float>							&	texture_channel_weights,
		const std::vector<vk2d::Matrix4f>					&	transformations				= { vk2d::Matrix4f( 1.0f ) },
		vk2d::Texture										*	texture						= nullptr,
		vk2d::Sampler										*	sampler						= nullptr );

	/// @brief		Draw a simple point with a color and size. This is really inefficient however and as
	///				soon as you need 2 or more points drawn, consider using
	///				vk2d::RenderTargetTexture::DrawPointList() instead.
	///				This type of draw call is provided mostly for ease of use on simple situations and
	///				for completeness.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	location
	///				Location where to draw the point on the render target texture surface.
	/// @param[in]	color
	///				Color of the point.
	/// @param[in]	size
	///				Size of the point we're drawing in texels.
	VK2D_API void												VK2D_APIENTRY				DrawPoint(
		vk2d::Vector2f											location,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float													size						= 1.0f );

	/// @brief		Draw a simple line with a color and size. This is really inefficient however and as
	///				soon as you need 2 or more lines drawn, consider using
	///				vk2d::RenderTargetTexture::DrawLineList() instead.
	///				This type of draw call is provided mostly for ease of use on simple situations and
	///				for completeness.
	/// @note		Multithreading: Main thread only.
	/// @param[in]	point_1
	///				Location of the first end of the line.
	/// @param[in]	point_2
	///				Location fo the second end of the line.
	/// @param[in]	color
	///				Color of the line.
	VK2D_API void												VK2D_APIENTRY				DrawLine(
		vk2d::Vector2f											point_1,
		vk2d::Vector2f											point_2,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float													line_width					= 1.0f );

	VK2D_API void												VK2D_APIENTRY				DrawRectangle(
		vk2d::Rect2f											area,
		bool													filled						= true,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawEllipse(
		vk2d::Rect2f											area,
		bool													filled						= true,
		float													edge_count					= 64.0f,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawEllipsePie(
		vk2d::Rect2f											area,
		float													begin_angle_radians,
		float													coverage,
		bool													filled						= true,
		float													edge_count					= 64.0f,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawRectanglePie(
		vk2d::Rect2f											area,
		float													begin_angle_radians,
		float													coverage,
		bool													filled						= true,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawTexture(
		vk2d::Vector2f											top_left,
		vk2d::Texture										*	texture,
		vk2d::Colorf											color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	VK2D_API void												VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh									&	mesh,
		const vk2d::Transform								&	transformations				= {} );

	VK2D_API void												VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh									&	mesh,
		const std::vector<vk2d::Transform>					&	transformations );

	VK2D_API void												VK2D_APIENTRY				DrawMesh(
		const vk2d::Mesh									&	mesh,
		const std::vector<vk2d::Matrix4f>					&	transformations );

	VK2D_API bool												VK2D_APIENTRY				IsGood() const;

private:
	std::unique_ptr<vk2d::_internal::RenderTargetTextureImpl>	impl;
};



} // vk2d
