#pragma once

#include <core/Common.hpp>

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
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GeneratePointMeshFromList(
		const std::vector<glm::vec2>				&	points
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateLineMeshFromList(
		const std::vector<glm::vec2>				&	points,
		const std::vector<VertexIndex_2>			&	indices
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateTriangleMeshFromList(
		const std::vector<glm::vec2>				&	points,
		const std::vector<VertexIndex_3>			&	indices,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateRectangleMesh(
		Rect2f											area,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateEllipseMesh(
		Rect2f											area,
		bool											filled,
		float											edge_count
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateRectanglePieMesh(
		Rect2f											area,
		float											begin_angle_radians,
		float											coverage,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GeneratePointMeshFromList(
		const std::vector<glm::vec2>				&	points
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateEllipsePieMesh(
		Rect2f											area,
		float											begin_angle_radians,
		float											coverage,
		bool											filled,
		float											edge_count
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateLatticeMesh(
		Rect2f											area,
		glm::vec2										subdivisions,
		bool											filled
	);
	
	template<vk2d_internal::VertexBaseDerivedType VertexT>
	requires(
		vk2d_internal::VertexHasVertexCoords<VertexT> &&
		vk2d_internal::VertexHasUVCoords<VertexT> &&
		vk2d_internal::VertexHasSingleTextureLayer<VertexT>
	)
	friend Mesh<VertexT>								mesh_generators::GenerateTextMesh(
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
