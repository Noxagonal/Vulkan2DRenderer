#pragma once

#include <core/Common.h>

#include "MeshPrimitives.hpp"
#include "generators/MeshGeneratorDeclarations.hpp"
#include <containers/Rect2.hpp>
#include <containers/Color.hpp>

#include <vector>
#include <string>



namespace vk2d {

namespace vk2d_internal {

class WindowImpl;
class RenderTargetTextureImpl;

} // vk2d_internal



class Window;
class RenderTargetTexture;
class FontResource;
class Sampler;
class Texture;



class MeshBase
{
	friend class Window;
	friend class RenderTargetTexture;

	friend class vk2d_internal::WindowImpl;
	friend class vk2d_internal::RenderTargetTextureImpl;

	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GeneratePointMeshFromList(
		const std::vector<glm::vec2>				&	points
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateLineMeshFromList(
		const std::vector<glm::vec2>				&	points,
		const std::vector<VertexIndex_2>			&	indices
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateTriangleMeshFromList(
		const std::vector<glm::vec2>				&	points,
		const std::vector<VertexIndex_3>			&	indices,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateRectangleMesh(
		Rect2f											area,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateEllipseMesh(
		Rect2f											area,
		bool											filled,
		float											edge_count
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateRectanglePieMesh(
		Rect2f											area,
		float											begin_angle_radians,
		float											coverage,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GeneratePointMeshFromList(
		const std::vector<glm::vec2>				&	points
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateEllipsePieMesh(
		Rect2f											area,
		float											begin_angle_radians,
		float											coverage,
		bool											filled,
		float											edge_count
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateLatticeMesh(
		Rect2f											area,
		glm::vec2										subdivisions,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	friend Mesh<VertexT>								GenerateTextMesh(
		FontResource								*	font,
		glm::vec2										origin,
		std::string										text,
		float											kerning,
		glm::vec2										scale,
		bool											vertical,
		uint32_t										font_face,
		bool											wait_for_resource_load
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set texture to be used when rendering this mesh object.
	/// 
	/// @param[in]	texture_pointer
	///				A pointer to a texture object.
	VK2D_API void									SetTexture(
		Texture									*	texture_pointer
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set sampler to be used when rendering this mesh object.
	/// 
	/// @param[in]	sampler_pointer
	///				A pointer to a sampler object.
	VK2D_API void									SetSampler(
		Sampler									*	sampler_pointer
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set line width between vertices when rendering lines or wireframe.
	///
	///				If this mesh is rendered as a list of lines we can define the line width. This is a per draw feature and can not
	///				be controlled per vertex or segment. This uses a GPU feature and is expected to perform well even when rendering
	///				lots of lines, however it can be low quality. For more control over the quality you should render rotated
	///				rectangles or manually construct your own mesh.
	///
	/// @param[in]	line_width
	///				Width of the line.
	VK2D_API void									SetLineWidth(
		float										line_width
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Manually set mesh type to render points, lines, wireframe or triangles.
	///
	///				This has limitations however if the mesh was created by a mesh generator. If mesh was created manually then
	///				there are no safeguards and it's up to the user of this library to make sure that the mesh data makes sense.
	///				<br>
	///				Limitations when this mesh was created by a generator:
	///				<br>
	///				<table>
	///				<tr><th> Generated mesh type	<th> Allowed new type
	///				<tr><td> Filled					<td> <tt>TRIANGLE_FILLED</tt>, <tt>TRIANGLE_WIREFRAME</tt>, <tt>POINT</tt>
	///				<tr><td> Outline				<td> <tt>LINE</tt>, <tt>POINT</tt>
	///				</table>
	/// @param[in]	type
	///				New mesh type.
	VK2D_API void									SetMeshType(
		MeshType									type
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Indices list.
	///
	///				Indices are a list of offsets pointing to the Mesh::vertices defining shape end points. How to use this
	///				list depends on what type of shape this mesh contains. To be more efficient a single vertex can be used multiple
	///				times in part of different triangles or lines. <br>
	///				- For triangles this list size must be a multiple of 3 where every group of 3 indices define a triangle and
	///				contain offsets to the vertices list defining the 3 corners for the triangle.
	///				- For lines this list size must be a multiple of 2 where every group of 2 indices define a line and contain
	///				offsets to the vertices list defining the end points for the line.
	///				- For points indices are not used.
	std::vector<uint32_t>							indices							= {};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Texture channel weights are used to mix between differet texture arrays per vertex.
	///
	///				The size of this list should be either 0, or the amount of vertices and the amount of layers in the used texture
	///				multiplied together. <br>
	///				For example if a texture with 3 layers is used then the first 3 indices of this list tells how much weight each
	///				texture layer has on the first vertex (in same order as texture layers are used). The next 3 indices of this
	///				list tells how much each texture layer effects the second vertex and so on. <br>
	///				For another example if a texture with 5 layers is used then the first 5 indices of this list tells how much each
	///				texture layer effects the first vertex, and the next 5 after that tells how much weight each texture layer has
	///				on the second vertex and so on until we cover all vertices. <br>
	///				Texture layer weights multiply the color from the texture, for example if a 2 layer texture is used then weights
	///				{0.0, 0.0} would result black transparent vertex, {1.0, 0.0} would show the first texture layer, {0.5, 0.5}
	///				would blend both texture layers together, {1.0, 1.0} would add colors of both layers together. <br>
	///				If this list is not large enough to contain all weights of all texture channels for all vertices then these
	///				values are ignored and the texture layers is chosen based on what was set to the vertex. <br>
	///				Texture layer weights between vertices are linearly interpolated so that if one texture layer weight is high for
	///				one vertex and low for the neighbouring vertex, the inbetween regions are smoothly transitioned between the two.
	std::vector<float>								texture_layer_weights			= {};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Axis aligned bounding box.
	///
	///				This tells the area of within all vertex coordinates reside. It can be set manually or recalculated from
	///				vertices by calling Mesh::RecalculateAABB(). It can be used to get size of the mesh, as a fast speed
	///				preliminary check if twoshapes are overlapping or to check if shape should be rendered or not.
	Rect2f											aabb							= {};

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Tells if this mesh was created by a mesh generator.
	bool											generated						= false;

	/// @brief		Tells the original mesh type if it was created by a mesh generator.
	MeshType										generated_mesh_type				= MeshType::TRIANGLE_FILLED;

	/// @brief		Current mesh type.
	MeshType										mesh_type						= MeshType::TRIANGLE_FILLED;

	/// @brief		Line width when rendering lines.
	float											line_width						= 1.0f;

	/// @brief		Texture resource to be used when rendering.
	///
	///				Can be used in all modes. Can be nullptr.
	Texture										*	texture							= nullptr;

	/// @brief		Texture sampler to be used when rendering.
	///
	///				Can be used in all modes. Can be nullptr for default sampler.
	Sampler										*	sampler							= nullptr;
};



} // vk2d
