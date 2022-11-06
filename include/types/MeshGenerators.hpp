#pragma once

#include <core/Common.h>

#include <types/MeshPrimitives.hpp>
#include <types/Rect2.hpp>

#include <vector>

namespace vk2d {

class FontResource;

class MeshBase;

template<typename ...ShaderParamsT>
class Mesh;



namespace vk2d_internal {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GeneratePointMeshFromListImpl(
	MeshBase									&	mesh,
	const std::vector<glm::vec2>				&	points
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateLineMeshFromListImpl(
	MeshBase									&	mesh,
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_2>			&	indices
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateTriangleMeshFromListImpl(
	MeshBase									&	mesh,
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_3>			&	indices,
	bool											filled
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateRectangleMeshImpl(
	MeshBase									&	mesh,
	Rect2f											area,
	bool											filled
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateEllipseMeshImpl(
	MeshBase									&	mesh,
	Rect2f											area,
	bool											filled,
	float											edge_count
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateEllipsePieMeshImpl(
	MeshBase									&	mesh,
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled,
	float											edge_count
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateRectanglePieMeshImpl(
	MeshBase									&	mesh,
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateLatticeMeshImpl(
	MeshBase									&	mesh,
	Rect2f											area,
	glm::vec2										subdivisions,
	bool											filled
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void										GenerateTextMeshImpl(
	MeshBase									&	mesh,
	FontResource								*	font,
	glm::vec2										origin,
	std::string										text,
	float											kerning,
	glm::vec2										scale,
	bool											vertical,
	uint32_t										font_face,
	bool											wait_for_resource_load
);



} // vk2d_internal



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate point mesh from point list.
///
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	points
///				A list of coordinates representing point locations.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GeneratePointMeshFromList(
	const std::vector<glm::vec2>				&	points
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GeneratePointMeshFromListImpl(
		mesh,
		points
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate line mesh from points and connections.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	points
///				A list of coordinates representing end points of lines.
/// 
/// @param[in]	indices
///				Indices defining which points should be connected with a line.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateLineMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_2>			&	indices
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateLineMeshFromListImpl(
		mesh,
		points,
		indices
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate triangle mesh from points and connections.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	points
///				A list of coordinates representing corners of triangles.
/// 
/// @param[in]	indices
///				Indices defining which points should form a triangle.
/// 
/// @param[in]	filled
///				true if triangle mesh is filled, false for wireframe.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateTriangleMeshFromList(
	const std::vector<glm::vec2>				&	points,
	const std::vector<VertexIndex_3>			&	indices,
	bool											filled							= true
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateTriangleMeshFromListImpl(
		mesh,
		points,
		indices,
		filled
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate rectangular mesh from rectangle coordinates.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	area
///				Area of the rectangle that will be covered, depends on the coordinate system. See RenderCoordinateSpace
///				for more info about what scale is used.
/// 
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateRectangleMesh(
	Rect2f											area,
	bool											filled							= true
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateRectangleMeshImpl(
		mesh,
		area,
		filled
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate ellipse mesh from rectangle area and edge count.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
///				used.
/// 
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// 
/// @param[in]	edge_count
///				Number of outer edges, this is a floating point value to prevent popping in case it's animated.
///
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateEllipseMesh(
	Rect2f											area,
	bool											filled							= true,
	float											edge_count						= 64.0f
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateEllipseMeshImpl(
		mesh,
		area,
		filled,
		edge_count
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate an ellipse or a circle that has a "slice" cut out, similar to usual graphs.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See RenderCoordinateSpace for more info about what scale is
///				be used.
/// 
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// 
/// @param[in]	coverage
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full ellipse. Moving value
///				from 0 to 1 makes "whole" pie visible in clockwise direction.
/// 
/// @param[in]	filled
///				true to draw the inside of the pie, false to draw the outline only.
/// 
/// @param[in]	edge_count 
///				How many corners the complete ellipse should have, or quality if you prefer. This is a float value for
///				"smoother" transitions between amount of corners, in case this value is animated.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateEllipsePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true,
	float											edge_count						= 64.0f
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateEllipsePieMeshImpl(
		mesh,
		area,
		begin_angle_radians,
		coverage,
		filled,
		edge_count
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a rectangular pie, similar to drawing a rectangle but which has a pie slice cut out.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	area
///				Area of the rectangle. See RenderCoordinateSpace for more info about what scale is used.
///
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// 
/// @param[in]	coverage 
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the full rectangle. Moving value
///				from 0 to 1 makes "whole" pie visible in clockwise direction.
/// 
/// @param[in]	filled
///				true to draw the inside of the pie rectangle, false to draw the outline only.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateRectanglePieMesh(
	Rect2f											area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateRectanglePieMeshImpl(
		mesh,
		area,
		begin_angle_radians,
		coverage,
		filled
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a lattice mesh, this is useful for distortions.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	area
///				Area of the rectangle. See RenderCoordinateSpace for more info about what scale is used.
/// 
/// @param[in]	subdivisions
///				Number of inside subdivisions, eg. 2*2 will generate lattice with 4*4 rectangles. If fractional values are used
///				then inside subdivisions are moved inward from the bottom right direction, this is to prevent popping in case
///				this value is animated.
/// 
/// @param[in]	filled
///				true if the inside is filled, false to generate a lattice line mesh.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateLatticeMesh(
	Rect2f											area,
	glm::vec2										subdivisions,
	bool											filled							= true
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateLatticeMeshImpl(
		mesh,
		area,
		subdivisions,
		filled
	);
	return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Generate a text mesh that can be used to render text.
/// 
/// @tparam		ShaderParamsT
///				Additional per vertex parameters that are sent to your custom shader.
/// @warning	Custom shader interface must have exactly the same parameters as the mesh.
/// 
/// @param[in]	font
///				A pointer to font resource to use.
/// 
/// @param[in]	origin
///				Left or top coordinate of what is considered the starting point for the text.
///				- For horisontal text origin is left side beginning of text and vertical origin is the baseline.
///				- For vertical text origin is top beginning of text and horisontal offset is middle of the symbols.
/// 
/// @param[in]	text
///				Text characters to generate.
/// 
/// @param[in]	kerning
///				Spacing between letters. Positive values are farther apart. Value is based on size of the font texel size.
/// 
/// @param[in]	scale
///				Scale of the mesh to be generated. Value {1.0, 1.0} will map 1:1 to the font size when render target texture or
///				window coordinate space is either RenderCoordinateSpace::TEXEL_SPACE or
///				RenderCoordinateSpace::TEXEL_SPACE_CENTERED.
/// 
/// @param[in]	vertical
///				true if text is generated vertically, false if horisontally.
/// 
/// @param[in]	font_face
///				Certain fonts may contain multiple font faces, this allows you to select which one to use.
/// 
/// @param[in]	wait_for_resource_load
///				When generating text mesh you usually want it to be available right away. Setting this option to true will wait
///				for the font resource to load before allowing execution to continue. If you're in a time critical situation
///				where you are generating the text every frame and you cannot afford a few millisecond wait for the font to load
///				up you can set this value to false. <br>
///				This function cannot generate text until the font has been loaded so setting this to false will return an empty
///				mesh until the font has been fully loaded by the resource manager.
/// 
/// @return		A new mesh object.
template<typename ...ShaderParamsT>
Mesh<ShaderParamsT...>								GenerateTextMesh(
	FontResource								*	font,
	glm::vec2										origin,
	std::string										text,
	float											kerning							= 0.0f,
	glm::vec2										scale							= glm::vec2( 1.0f, 1.0f ),
	bool											vertical						= false,
	uint32_t										font_face						= 0,
	bool											wait_for_resource_load			= true
)
{
	auto mesh = Mesh<ShaderParamsT...>();
	vk2d_internal::GenerateTextMeshImpl(
		mesh,
		font,
		origin,
		text,
		kerning,
		scale,
		vertical,
		font_face,
		wait_for_resource_load
	);
	return mesh;
}



} // vk2d
