#pragma once

#include <core/Common.h>

#include <mesh/vertex/VertexBase.hpp>

#include <vector>



namespace vk2d {



class FontResource;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename VertexT = StandardVertex>
class Mesh;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GeneratePointMeshFromList(
	const std::vector<glm::vec2>				&	points
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateLineMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_2>			&	indices
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateTriangleMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_3>			&	indices,
	bool											filled = true
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateRectangleMesh(
	Rect2f											area,
	bool											filled = true
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateEllipseMesh(
	Rect2f											area,
	bool											filled = true,
	float											edge_count = 64.0f
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateRectanglePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled = true
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateEllipsePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled = true,
	float											edge_count = 64.0f
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT>
)
Mesh<VertexT>										GenerateLatticeMesh(
	Rect2f											area,
	glm::vec2										subdivisions,
	bool											filled = true
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseDerivedType VertexT = StandardVertex>
requires(
	vk2d_internal::VertexHasVertexCoords<VertexT> &&
	vk2d_internal::VertexHasUVCoords<VertexT> &&
	vk2d_internal::VertexHasSingleTextureLayer<VertexT>
)
Mesh<VertexT>										GenerateTextMesh(
	FontResource								*	font,
	glm::vec2										origin,
	std::string										text,
	float											kerning = 0.0f,
	glm::vec2										scale = glm::vec2( 1.0f, 1.0f ),
	bool											vertical = false,
	uint32_t										font_face = 0,
	bool											wait_for_resource_load = true
);

} // vk2d
