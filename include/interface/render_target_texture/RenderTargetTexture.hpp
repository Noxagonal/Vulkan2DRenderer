#pragma once

#include <core/Common.hpp>

#include <containers/BlurType.hpp>
#include <containers/Rect2.hpp>
#include <containers/Transform.hpp>
#include <containers/Color.hpp>
#include <containers/Multisamples.hpp>
#include <containers/RenderCoordinateSpace.hpp>

#include <mesh/Mesh.hpp>
#include <mesh/vertex/RawVertexData.hpp>
#include <mesh/vertex/VertexBase.hpp>

#include <interface/texture/Texture.hpp>

#include <memory>
#include <span>

namespace vk2d {

class Sampler;
class MeshBase;

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
	///				Reference to instance that owns this render target texture.
	/// 
	/// @param[in]	create_info
	///				Reference to RenderTargetTextureCreateInfo object defining parameters for this render target texture.
	VK2D_API												RenderTargetTexture(
		vk2d_internal::InstanceImpl						&	instance,
		const RenderTargetTextureCreateInfo				&	create_info
	);

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
		RenderCoordinateSpace								coordinate_space
	);

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
		glm::uvec2											new_size
	);

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
		BlurType											blur_type					= BlurType::GAUSSIAN
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws points directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawPointList_DEPRICATED(
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations = {},
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr
	)
	{
		DrawPointList_DEPRICATED(
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			texture,
			sampler
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws lines directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	indices
	///				List of indices telling how to form lines between vertices.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawLineList_DEPRICATED(
		std::span<const VertexIndex_2>				indices,
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations = {},
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr,
		float										line_width = 1.0f
	)
	{
		auto indices_span = std::span( reinterpret_cast<const uint32_t*>( indices.data() ), indices.size() * 2 );

		DrawLineList_DEPRICATED(
			indices_span,
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			texture,
			sampler
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draw triangles directly.
	/// 
	///				This option is mostly provided for completeness. This is not really useful in most cases, however if you require
	///				absolute control over how drawing is done, this is the least overhead method of drawing in VK2D.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	indices
	///				List of indices telling how to form lines between vertices.
	/// 
	/// @param[in]	vertices 
	///				Vertices that define where and how points, line endings and polygon corners are drawn.
	///
	/// @param[in]	transformations
	///				Matrices defining transformations that will be applied to all vertices prior to rendering them. If
	///				none are provided then a default transformation is applied. If multiple transformations are provided then this
	///				draw call will render multiple times, each time using a different transformation, this is called instanced
	///				render. The benefit of instanced render is that it saves CPU time and memory when you need to render lots of
	///				similar looking shapes. For clarification: In case of drawing points this also means that if multiple
	///				transformation matrices are given then every point is also drawn multiple times, eg. 10 vertices with 2
	///				transformations means 20 points drawn.
	/// 
	/// @param[in]	texture 
	///				Pointer to texture, see Vertex for UV mapping details. Can be nullptr in which case a white texture is
	///				used (vertex colors only).
	/// 
	/// @param[in]	sampler 
	///				Pointer to sampler which determines how the texture is drawn. Can be nullptr in which case the default sampler
	///				is used.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawTriangleList_DEPRICATED(
		std::span<const VertexIndex_3>				indices,
		std::span<const VertexT>					vertices,
		std::span<const glm::mat4>					transformations = {},
		bool										filled = true,
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr
	)
	{
		auto indices_span = std::span( reinterpret_cast<const uint32_t*>( indices.data() ), indices.size() * 3 );

		DrawTriangleList_DEPRICATED(
			indices_span,
			vk2d::vk2d_internal::RawVertexData( vertices ),
			transformations,
			filled,
			texture,
			sampler
		);
	}

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
		float												size						= 1.0f
	);

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
		float												line_width					= 1.0f
	);

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
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

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
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

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
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

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
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

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
		Colorf												color						= { 1.0f, 1.0f, 1.0f, 1.0f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	/// 
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	///
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw. Using multiple
	///				transformations results the mesh being drawn multiple times using different transformations. This is also called
	///				instanced rendering.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		std::span<const glm::mat4>					transformations
	)
	{
		if( mesh.vertices.size() == 0 ) return;
		auto raw_vertices = vk2d_internal::RawVertexData( std::span<const VertexT>( mesh.vertices ) );

		switch( mesh.mesh_type ) {
		case MeshType::TRIANGLE_FILLED:
			DrawTriangleList_DEPRICATED(
				mesh.indices,
				raw_vertices,
				transformations,
				true,
				mesh.texture,
				mesh.sampler
			);
			break;
		case MeshType::TRIANGLE_WIREFRAME:
			DrawTriangleList_DEPRICATED(
				mesh.indices,
				raw_vertices,
				transformations,
				false,
				mesh.texture,
				mesh.sampler
			);
			break;
		case MeshType::LINE:
			DrawLineList_DEPRICATED(
				mesh.indices,
				raw_vertices,
				transformations,
				mesh.texture,
				mesh.sampler,
				mesh.line_width
			);
			break;
		case MeshType::POINT:
			DrawPointList_DEPRICATED(
				raw_vertices,
				transformations,
				mesh.texture,
				mesh.sampler
			);
			break;
		default:
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	///
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	///
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformation
	///				Draw using transformation.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		const Transform							&	transformation = {}
	)
	{
		auto transformation_matrix = transformation.CalculateTransformationMatrix();

		DrawMesh(
			mesh,
			std::span( &transformation_matrix, 1 )
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Draws Mesh which contains all information needed for the render.
	/// 
	///				Consider using this method of drawing as often as possible. Meshes can be reused, which lowers the overhead
	///				needed to calculate them all the time.
	/// 
	/// @note		Multithreading: Main thread only.
	/// 
	/// @see		Mesh
	/// 
	/// @tparam		VertexT
	///				Vertex type.
	/// 
	/// @param[in]	mesh
	///				Mesh object to draw.
	/// 
	/// @param[in]	transformations
	///				Draw using transformation. This is a std::vector where each element equals one draw. Using multiple
	///				transformations results the mesh being drawn multiple times using different transformations. This is also called
	///				instanced rendering.
	template<vk2d_internal::VertexBaseOrDerivedType VertexT>
	void											DrawMesh(
		const Mesh<VertexT>						&	mesh,
		std::span<const Transform>					transformations
	)
	{
		std::vector<glm::mat4> transformation_matrices( std::size( transformations ) );
		for( size_t i = 0; i < std::size( transformations ); ++i ) {
			transformation_matrices[ i ] = transformations[ i ].CalculateTransformationMatrix();
		}

		DrawMesh(
			mesh,
			transformation_matrices
		);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	/// 
	/// @note		Multithreading: Any thread.
	/// 
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool									IsGood() const;

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawPointList_DEPRICATED(
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations = {},
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawLineList_DEPRICATED(
		std::span<const uint32_t>					indices,
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations = {},
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr,
		float										line_width = 1.0f
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API void									DrawTriangleList_DEPRICATED(
		std::span<const uint32_t>					indices,
		const vk2d_internal::RawVertexData		&	raw_vertex_data,
		std::span<const glm::mat4>					transformations = {},
		bool										filled = true,
		Texture									*	texture = nullptr,
		Sampler									*	sampler = nullptr
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<vk2d_internal::RenderTargetTextureImpl>	impl;
};



} // vk2d
