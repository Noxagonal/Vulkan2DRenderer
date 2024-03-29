#pragma once

#include "core/Common.h"

#include "types/BlurType.h"
#include "types/Rect2.hpp"
#include "types/Transform.h"
#include "types/Color.hpp"
#include "types/Multisamples.h"
#include "types/RenderCoordinateSpace.hpp"

#include "interface/Texture.h"

#include <memory>

namespace vk2d {

class Sampler;
class Mesh;

namespace vk2d_internal {

class InstanceImpl;
class RenderTargetTextureImpl;

} // vk2d_internal


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Parameters to construct a RenderTargetTexture.
struct RenderTargetTextureCreateInfo
{
	/// @brief		Coordinate system to be used, see RenderCoordinateSpace.
	RenderCoordinateSpace					coordinate_space			= RenderCoordinateSpace::TEXEL_SPACE;

	/// @brief		Render target texture texel size.
	glm::uvec2								size						= glm::uvec2( 512, 512 );

	/// @brief		Multisampling, must be a singular value, see Multisamples. Uses more GPU resources if higher than 1.
	Multisamples							samples						= Multisamples::SAMPLE_COUNT_1;

	/// @brief		Enable ability to blur the render target texture at the end of the render. Uses more GPU resources if
	///				enabled.
	bool									enable_blur					= false;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Render target texture.
///
///				Render target texture is a texture that can be used like a regular texture and also rendered to. It is a lot
///				more resource heavy than a regular texture, however it enables a lot of flexibility in the host application. You
///				can also blur the render target texture at the end of its rendering operation. Render target texture is
///				primarily meant for realtime use where you re-render it each frame (and thus it also uses a fair bit of memory
///				as it is double-buffered), however you do not need to re-render it each frame, just re-render it whenever the
///				contents of the render target texture change. You can of course render to a render target texture only once in
///				it's lifetime and just use it like a regular texture throughout the application, however in this situation, you
///				should instead consider creating a TextureResource.
class RenderTargetTexture :
	public Texture
{
	friend class vk2d_internal::InstanceImpl;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Constructor. This object cannot be directly constructed
	///
	///				This object is created with Instance::CreateRenderTargetTexture().
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	instance
	///				Pointer to instance that owns this render target texture.
	/// 
	/// @param[in]	create_info
	///				Reference to RenderTargetTextureCreateInfo object defining parameters for this render target texture.
	VK2D_API												RenderTargetTexture(
		vk2d_internal::InstanceImpl						*	instance,
		const RenderTargetTextureCreateInfo				&	create_info );

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API												~RenderTargetTexture();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Switch render coordinate space.
	///
	///				Can be set at any time, but will not take effect until the start of the next frame.
	///
	/// @see		RenderCoordinateSpace
	/// 
	/// @param[in]	coordinate_space
	///				Coordinate space to switch to.
	VK2D_API void											SetRenderCoordinateSpace(
		RenderCoordinateSpace								coordinate_space );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Sets the texel size of the render target texture.
	///
	///				This recreates all internal resources which means this render target texture will synchronize which will cause
	///				some microstutters during hot render loop. If you can, prefer setting the render target texture size high enough
	///				that you don't need to call this function. If you do not know the reasonable estimate for the maximum size at
	///				creation time, try to reserve some extra texel space around the planned content and minimize calls to this
	///				function.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @warning
	///				- Contents will be lost and you will need to re-render this render target texture.
	///				- Do not call inside it's own render block. It is an error to call <tt><b> this->SetSize() </b></tt> function
	///				between <tt><b> this->BeginRender() </b></tt> and <tt><b> this->EndRender() </b></tt>.
	/// 
	/// @param[in]	new_size
	///				New texel size of this render target texture.
	VK2D_API void											SetSize(
		glm::uvec2											new_size );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the texel size of the render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		Size of the render target texture in texels.
	VK2D_API glm::uvec2										GetSize() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Gets the texture layer count.
	///
	///				This is provided for compatibility with Texture. It always returns 1.
	///
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		Number of texture layers. (Render target texture always returns 1)
	VK2D_API uint32_t										GetLayerCount() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tells if the texture data is ready to be used.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true if texture data is ready to be used, false otherwise.
	VK2D_API bool											IsTextureDataReady();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Begins the render operations.
	///
	///				This signals the beginning of the render block and you must call this before using any drawing commands.
	///				For best performance you should calculate game logic first, when you're ready to draw call this function just
	///				before your first draw command. Every draw call must be between this and RenderTargetTexture::EndRender().
	/// 
	/// @see		RenderTargetTexture::EndRender()
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @return		true on success, false if something went wrong.
	VK2D_API bool											BeginRender();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Ends and finalizes the rendering operations.
	///
	///				This signals the end of the render block and you must call this after you're done drawing everything in order
	///				to display the results on the render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	///	@note		For performance testing: Timing this function is always useless as nothing is actually rendered here yet.
	///
	/// @note		Rendering of render target textures is only submitted to the GPU at Window::EndRender() for performance
	///				reasons. In cases where render target textures render from other render target textures, these render
	///				target textures form dependency chains and these dependency chains are only solved just before the final
	///				submission to the GPU.
	///				<br>
	///				This does 2 things:
	///				- Cyclic dependencies are not allowed. For example, you cannot have "X" render target texture depend on "Y"
	///				which depends on "X".
	///				- Order in which you draw render target textures does not matter. For example, if you draw render target texture
	///				"X" first, which uses render target texture "Y" as a texture, then render "Y", "Y" will actually be rendered
	///				before "X" even though "X" drawn first in your application. This also makes "Y" show contents of the current
	///				frame instead of previous frame.
	///
	/// @param[in]	blur_amount
	///				Amount of vertical and horisontal blur in texels. Input is in floats for smoothness.
	/// 
	/// @param[in]	blur_type
	///				Type of blur to use, see BlurType for more info.
	/// 
	/// @return		true on success, false if something went wrong.
	VK2D_API bool											EndRender(
		glm::vec2											blur_amount					= {},
		BlurType											blur_type					= BlurType::GAUSSIAN );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw triangles directly.
	///
	///				This is the best option if you wish to render vertices directly. Every other VK2D draw operation internally
	///				calls this function to do the actual drawing.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	indices
	///				Indices are the index numbers into the vertices list where each vertex is considered the corner point of the
	///				triangle.
	/// 
	/// @param[in]	vertices
	///				Vertices are the corners of a triangle. A vertex also defines more data related to that corner and that data
	///				gets interpolated inside the triangle.
	/// 
	/// @param[in]	texture_layer_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those texture layers have per
	///				vertex. The size of this vector parameter should be the number of texture layers multiplied by number of
	///				vertices. Data should be formatted so that for each vertex, weight of each texture layer is given.<br>
	///				For example: a simple triangle with texture with 2 layers should have this order:
	///				(v = vertex, t = texture layer) <br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1][v2 t0][v2 t1]</tt>
	/// 
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior to rendering them.
	///				If none are provided then a default transformation is applied. If multiple transformations are provided then
	///				this draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes.
	/// 
	/// @param[in]	filled
	///				Tells if the inside of the triangle should be filled, true for filled, false for wireframe.
	/// 
	/// @param[in]	texture
	///				Pointer to any object that implements Texture interface, such as TextureResource or
	///				RenderTargetTexture.
	/// 
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	VK2D_API void											DrawTriangleList(
		const std::vector<VertexIndex_3>				&	indices,
		const std::vector<Vertex>						&	vertices,
		const std::vector<float>						&	texture_layer_weights,
		const std::vector<glm::mat4>					&	transformations				= {},
		bool												filled						= true,
		Texture											*	texture						= nullptr,
		Sampler											*	sampler						= nullptr );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw lines directly.
	///
	///				This option is useful when you want to draw simple lines. Eg. for debugging. For more sophisticated rendering
	///				you should prefer rendering triangles. Every other VK2D draw operation internally calls this function to do the
	///				actual drawing.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	indices
	///				Indices are the index numbers into the vertices list where each vertex is considered the end point of the line.
	/// 
	/// @param[in]	vertices
	///				Vertices are the end points of a line. A vertex also defines more data related to that end point and that data
	///				gets interpolated for each point along the line.
	/// 
	/// @param[in]	texture_layer_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those texture layers have per
	///				vertex. The size of this vector parameter should be the number of texture layers multiplied by number of
	///				vertices. Data should be formatted so that for each vertex, weight of each texture layer is given. <br>
	///				For example: a simple line with texture with 2 layers should have this order: (v = vertex, t = texture layer)
	///				<br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1]</tt>
	/// 
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes.
	/// 
	/// @param[in]	texture
	///				Pointer to any object that implements Texture interface, such as TextureResource or
	///				RenderTargetTexture.
	/// 
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	/// 
	/// @param[in]	line_width
	///				Lines can be drawn thicker or thinner than 1 texel width, however this applies to all lines being drawn at once
	///				and end points are not rounded.
	VK2D_API void											DrawLineList(
		const std::vector<VertexIndex_2>				&	indices,
		const std::vector<Vertex>						&	vertices,
		const std::vector<float>						&	texture_layer_weights,
		const std::vector<glm::mat4>					&	transformations				= {},
		Texture											*	texture						= nullptr,
		Sampler											*	sampler						= nullptr,
		float												line_width					= 1.0f );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw points directly.
	///
	///				This option is mostly provided for completeness. This is not really useful in most cases, however vertices can
	///				have different sizes which can be used for some particle effects or similar. Every other VK2D draw operation
	///				internally calls this function to do the actual drawing.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	vertices
	///				Vertices are the points to draw.
	/// 
	/// @param[in]	texture_layer_weights
	///				If you're using multi-layer-textures, here you can define how much weight each of those texture layers have per
	///				vertex. The size of this vector parameter should be the number of texture layers multiplied by number of
	///				vertices. Data should be formatted so that for each vertex, weight of each texture layer is given. <br>
	///				For example: 2 points with texture with 2 layers should have this order: (v = vertex, t = texture layer) <br>
	///				<tt>[v0 t0][v0 t1][v1 t0][v1 t1]</tt>
	/// 
	/// @param[in]	transformations
	///				A vector of matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture
	///				Pointer to any object that implements Texture interface, such as TextureResource or
	///				RenderTargetTexture.
	/// 
	/// @param[in]	sampler
	///				Pointer to a sampler object that tells how to read the texture.
	VK2D_API void											DrawPointList(
		const std::vector<Vertex>						&	vertices,
		const std::vector<float>						&	texture_layer_weights,
		const std::vector<glm::mat4>					&	transformations				= {},
		Texture											*	texture						= nullptr,
		Sampler											*	sampler						= nullptr );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a simple point with a color and size.
	///
	///				This is really inefficient however and as soon as you need 2 or more points drawn, consider using
	///				RenderTargetTexture::DrawPointList() instead. This type of draw call is provided mostly for ease of use on
	///				simple situations and for completeness.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	location
	///				Location where to draw the point on the render target texture surface.
	/// 
	/// @param[in]	color
	///				Color of the point.
	/// 
	/// @param[in]	size
	///				Size of the point we're drawing in texels.
	VK2D_API void											DrawPoint(
		glm::vec2											location,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float												size						= 1.0f );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a simple line with a color and size.
	///
	///				This is really inefficient however and as soon as you need 2 or more lines drawn, consider using
	///				RenderTargetTexture::DrawLineList() instead. This type of draw call is provided mostly for ease of use on
	///				simple situations and for completeness.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	point_1
	///				Location of the first end of the line.
	/// 
	/// @param[in]	point_2
	///				Location fo the second end of the line.
	/// 
	/// @param[in]	color
	///				Color of the line.
	VK2D_API void											DrawLine(
		glm::vec2											point_1,
		glm::vec2											point_2,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f },
		float												line_width					= 1.0f );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a rectangle directly into the render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Defines the top-left and bottom-right coordinates of the rectangle.
	/// 
	/// @param[in]	filled
	///				Tells if the inside of the shape is filled, true if filled, false to draw the edges only.
	/// 
	/// @param[in]	color
	///				Color of the rectangle.
	VK2D_API void											DrawRectangle(
		Rect2f												area,
		bool												filled						= true,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw an ellipse/circle directly into the render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Defines the top-left and bottom-right coordinates of a rectangle bounding box of the ellipse.
	/// 
	/// @param[in]	filled
	///				Tells if the inside of the shape is filled, true if filled, false to draw the edges only.
	/// 
	/// @param[in]	edge_count
	///				Determines the number of sides of the ellipse. This is a floating point value where whole numbers determine the
	///				complete edges, the fractional is used to make transition from one whole number to another smoother, eg. value
	///				3.5 would draw 3 complete edges and 1 edge that's half the size of the previous 3.
	/// 
	/// @param[in]	color
	///				Color of the ellipse.
	VK2D_API void											DrawEllipse(
		Rect2f												area,
		bool												filled						= true,
		float												edge_count					= 64.0f,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw an ellipse/circle pie directly into the render target texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Defines the top-left and bottom-right coordinates of a rectangle bounding box of the complete ellipse.
	/// 
	/// @param[in]	begin_angle_radians
	///				Angle that tells where the pie cut into the ellipse should start.
	/// 
	/// @param[in]	coverage
	///				Value in range from 0.0 to 1.0 telling how much of the ellipse is "whole" eg. 0.0 is nothing visible, 0.75
	///				quarter of the ellipse is cut out, 1.0 is just a complete ellipse. Note that this is not a percentage of the
	///				visible area but a percentage of the angle (PI*2*coverage).
	/// 
	/// @param[in]	filled
	///				Tells if the inside of the shape is filled, true if filled, false to draw the edges only.
	/// 
	/// @param[in]	edge_count
	///				Determines the number of sides of the ellipse. This is a floating point value where whole numbers determine the
	///				complete edges, the fractional is used to make transition from one whole number to another smoother, eg. value
	///				3.5 would draw 3 complete edges and 1 edge that's half the size of the previous 3. In this case this value
	///				applies to the complete elliptical shape and not just the visible shape.
	///
	/// @param[in]	color
	///				Color of the ellipse pie.
	VK2D_API void											DrawEllipsePie(
		Rect2f												area,
		float												begin_angle_radians,
		float												coverage,
		bool												filled						= true,
		float												edge_count					= 64.0f,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a rectangular pie directly into the render target texture.
	/// 
	///				This is a rectangle shape that can have a cake slice cut in it.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	area
	///				Defines the top-left and bottom-right coordinates of the rectangle.
	/// 
	/// @param[in]	begin_angle_radians
	///				Angle that tells where the pie cut into the rectangle should start.
	/// 
	/// @param[in]	coverage
	///				Value in range from 0.0 to 1.0 telling how much of the rectangle is "whole" eg. 0.0 is nothing visible, 0.75
	///				quarter of the rectangle is cut out, 1.0 is just a complete rectangle. Note that this is not a percentage of the
	///				visible area but a percentage of the angle (PI*2*coverage).
	/// 
	/// @param[in]	filled
	///				Tells if the inside of the shape is filled, true if filled, false to draw the edges only.
	/// 
	/// @param[in]	color
	///				Color of the rectangle pie.
	VK2D_API void											DrawRectanglePie(
		Rect2f												area,
		float												begin_angle_radians,
		float												coverage,
		bool												filled						= true,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw texture into the render target texture using the actual size of the texture.
	/// 
	///				Mostly useful for debugging, for final application prefer to use something else as the render size depends on
	///				the size of the texture.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @warning	Draw size is determined by texel size of the image, render target texture coordinate space must be either
	///				RenderCoordinateSpace::TEXEL_SPACE or RenderCoordinateSpace::TEXEL_SPACE_CENTERED, othervise
	///				rendered image will not be displayed correctly.
	/// 
	/// @param[in]	top_left
	///				Texture origin is the the top left corner. Bottom left coordinates are determined by the texture size.
	/// 
	/// @param[in]	texture
	///				A pointer to the texture to draw, if the texture is multi-layer then only the first layer is drawn.
	/// 
	/// @param[in]	color
	///				Multiplier for the the texture colors, eg. { 1.0, 0.0, 0.0, 1.0 } only renders the red channel of the texture,
	///				{ 1.0, 1.0, 1.0, 0.5 } renders the texture half transparent, { 10.0, 10.0, 10.0, 1.0 } will render the texture
	///				overexposed.
	VK2D_API void											DrawTexture(
		glm::vec2											top_left,
		Texture											*	texture,
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f } );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw a mesh object with single optional transform.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	mesh
	///				Reference to a mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				Optional transformations to use when drawing the mesh.
	VK2D_API void											DrawMesh(
		const Mesh										&	mesh,
		const Transform									&	transformations				= {} );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw one or more instances of a single mesh object using transforms.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	mesh
	///				Reference to a mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				An array of transformations to use when drawing the mesh. Number of transformations tells how many times to draw
	///				this mesh using each transformation.
	VK2D_API void											DrawMesh(
		const Mesh										&	mesh,
		const std::vector<Transform>					&	transformations );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw one or more instances of a single mesh object using transformation matrices.
	///
	/// @note		Multithreading: Main thread only.
	/// 
	/// @param[in]	mesh
	///				Reference to a mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				An array of transformation matrices to use when drawing the mesh. Number of transformations tells how many times
	///				to draw this mesh using each transformation.
	VK2D_API void											DrawMesh(
		const Mesh										&	mesh,
		const std::vector<glm::mat4>					&	transformations );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool											IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::RenderTargetTextureImpl>	impl;
};



} // vk2d
