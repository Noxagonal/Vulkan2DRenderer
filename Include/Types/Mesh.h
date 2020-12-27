#pragma once

#include "Core/Common.h"

#include "Types/MeshPrimitives.hpp"
#include "Types/Vector2.hpp"
#include "Types/Rect2.hpp"
#include "Types/Color.hpp"

#include <vector>
#include <string>

namespace vk2d {
namespace _internal {
class WindowImpl;
class RenderTargetTextureImpl;
}

class FontResource;
class Sampler;
class Texture;



/// @brief		A mesh represents a collection of vertices and their relations
///				forming a shape that can be drawn onto a surface.
/// 
///				- A mesh can be created using any of the mesh generator functions,
///				or constructing it manually.
///				- A mesh can be modified using any of the modifier functions or
///				by manually modifying the vertices and indices.
class Mesh {
	friend class vk2d::_internal::WindowImpl;
	friend class vk2d::_internal::RenderTargetTextureImpl;

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GeneratePointMeshFromList(
		const std::vector<vk2d::Vector2f>		&	points );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateLineMeshFromList(
		const std::vector<vk2d::Vector2f>		&	points,
		const std::vector<vk2d::VertexIndex_2>	&	indices );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateTriangleMeshFromList(
		const std::vector<vk2d::Vector2f>		&	points,
		const std::vector<vk2d::VertexIndex_3>	&	indices,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateRectangleMesh(
		vk2d::Rect2f								area,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateEllipseMesh(
		vk2d::Rect2f								area,
		bool										filled,
		float										edge_count );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateEllipsePieMesh(
		vk2d::Rect2f								area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled,	
		float										edge_count );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateRectanglePieMesh(
		vk2d::Rect2f								area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateLatticeMesh(
		vk2d::Rect2f								area,
		vk2d::Vector2f								subdivisions,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateTextMesh(
		vk2d::FontResource						*	font,
		vk2d::Vector2f								origin,
		std::string									text,
		float										kerning,
		vk2d::Vector2f								scale,
		bool										vertical,
		uint32_t									font_face,
		bool										wait_for_resource_load );

public:
	/// @brief		Moves all the vertices in some direction.
	/// @param[in]	movement
	///				A math vector that will be added to each vertex position.
	VK2D_API void									VK2D_APIENTRY					Translate(
		const vk2d::Vector2f						movement );

	/// @brief		Rotate the mesh around a pivot point.
	/// @param[in]	rotation_amount_radians
	///				Rotation amount in radians, positive direction is clockwise.
	/// @param[in]	origin
	///				The pivot point of which the mesh vertices are rotated around.
	VK2D_API void									VK2D_APIENTRY					Rotate(
		float										rotation_amount_radians,
		vk2d::Vector2f								origin							= {} );

	/// @brief		Moves all the vertices closer or father from the origin point.
	/// @param[in]	scaling_amount
	///				0.0 scales everything into a single point, 1.0 does nothing,
	///				2.0 doubles the current size of the mesh.
	/// @param[in]	origin
	///				Origin point of which everything is scaled towards or away from.
	VK2D_API void									VK2D_APIENTRY					Scale(
		vk2d::Vector2f								scaling_amount,
		vk2d::Vector2f								origin							= {} );

	/// @brief		Scews the mesh in vertical and horisontal directions based on origin point.
	/// @param[in]	scew_amount
	///				Positive values scew vertices on bottom right of origin to be moved
	///				towards bottom right and vertices top left of origin to be moved
	///				towards top left corner. <br>
	///				Amount is a half of the other dimension. This way if you have a cube
	///				for example, setting amount to {0.0, 1.0} will result the left side of
	///				the cube to be offsetted 0.5 of the total height upwards and the right
	///				side of the cube to be offsetted 0.5 of the total height downwards.
	///				This way the total angle is 45 degrees when scewing single axis by 1.0.
	/// @param[in]	origin
	///				Origin point of which everything is scewed in relation to.
	VK2D_API void									VK2D_APIENTRY					Scew(
		vk2d::Vector2f								scew_amount,
		vk2d::Vector2f								origin							= {} );

	/// @brief		Makes a simple wave pattern that can be animated. This is purely artistic
	///				effect.
	/// @param[in]	direction_radians
	///				Direction where the waves are moving towards. Angle is in radians.
	/// @param[in]	frequency
	///				Frequency is the number of waves inside the mesh bounding box average size.
	/// @param[in]	animation
	///				Animation is a float value that determines where the peaks of the waves
	///				are, slowly increasing this value will make the waves move.
	///				This value rolls over at 0.0 and 1.0. Meaning that 0.0 and 1.0 will produce
	///				the exact same pattern.
	/// @param[in]	intensity
	///				This determines the wave peak intensity per axis. This is applied from the
	///				perspective of the direction the wave and not the final result.
	/// @param[in]	origin
	///				Origin point where waves are generated from, since this is a directional
	///				wave this parameter only really matters if you're animating the rotation
	///				of the direction of the waves.
	VK2D_API void									VK2D_APIENTRY					DirectionalWave(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2f								intensity,
		vk2d::Vector2f								origin							= {} );

	/// @brief		Moves vertices UV coordinates to some direction. This directly moves the
	///				UVs to some direction so texture movement will appear exact opposite of
	///				moving vertices.
	/// @param[in]	movement
	///				Direction to move the UVs towards. Note that this is in UV coordinates which
	///				are always in range of 0.0 to 1.0, in case a UV coordinate goes outside
	///				this range, a vk2d::Sampler will determine what to do with it.
	VK2D_API void									VK2D_APIENTRY					TranslateUV(
		const vk2d::Vector2f						movement );

	/// @brief		Rotate UV coordinates on all vertices around origin. This directly scales
	///				the UVs so texture rotation will appear inverted.
	/// @param[in]	rotation_amount_radians
	///				Rotation amount in radians, positive values will turn the UVs counter
	///				clockwise and the texture will appear to rotate counter-clockwise.
	/// @param[in]	origin
	///				The pivot point of which the mesh UVs are rotated around. Please note that
	///				UV coordinates are always in range of 0.0 to 1.0 so origin outside this
	///				range will appear to both translate and rotate.
	VK2D_API void									VK2D_APIENTRY					RotateUV(
		float										rotation_amount_radians,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	/// @brief		Scaling UVs will work exactly the same as to scaling vertices except for
	///				UV coordinates, so scaling everything to 0.0 will make texture to be sampled
	///				from a single point, making texture appear infinitely large.
	/// @param[in]	scaling_amount
	///				Scale amount, 0.0 will scale all UVs into a single point, 1.0 does nothing,
	///				2.0 doubles the UV scale.
	/// @param[in]	origin
	///				Origin point of which everything is scaled towards or away from.
	VK2D_API void									VK2D_APIENTRY					ScaleUV(
		vk2d::Vector2f								scaling_amount,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	/// @brief		Scewing UVs works similarly to scewing vertices except for UVs. This means
	///				everything appears the opposite of the actual effect.
	/// @param[in]	scew_amount
	///				Positive values scew UVs on bottom right of origin to be moved
	///				towards bottom right and vertices top left of origin to be moved
	///				towards top left corner. <br>
	///				Amount is a half of the other dimension. This way the total angle is
	///				45 degrees when scewing single axis by 1.0.
	/// @param[in]	origin
	///				Origin point of which everything is scewed in relation to.
	VK2D_API void									VK2D_APIENTRY					ScewUV(
		vk2d::Vector2f								scew_amount,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	/// @brief		Makes a simple wave pattern that can be animated in the UV coordinate space.
	///				This is purely artistic effect. Please note that this directly effects the
	///				UV coordinates so effects will appear inverse.
	/// @param[in]	direction_radians
	///				Direction where the waves are moving towards. Angle is in radians.
	/// @param[in]	frequency
	///				Frequency is the number of waves inside the 0.0 to 1.0 UV range.
	/// @param[in]	animation
	///				Animation is a float value that determines where the peaks of the waves
	///				are, slowly increasing this value will make the waves move.
	///				This value rolls over at 0.0 and 1.0. Meaning that 0.0 and 1.0 will produce
	///				the exact same pattern.
	/// @param[in]	intensity
	///				This determines the wave peak intensity per axis. This is applied from the
	///				perspective of the direction the wave and not the final result.
	/// @param[in]	origin
	///				Origin point where waves are generated from, since this is a directional
	///				wave this parameter only really matters if you're animating the rotation
	///				of the direction of the waves.
	VK2D_API void									VK2D_APIENTRY					DirectionalWaveUV(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2f								intensity,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	/// @brief		Set all vertices to a specific color.
	/// @param[in]	new_color
	///				Color to apply to all vertices.
	VK2D_API void									VK2D_APIENTRY					SetVertexColor(
		vk2d::Colorf								new_color );

	/// @brief		Set color to all vertices following linear gradient. Gradient is generated
	///				between two points.
	/// @param[in]	color_1
	///				First color.
	/// @param[in]	color_2
	///				Second color.
	/// @param[in]	coord_1
	///				Location of the first color.
	/// @param[in]	coord_2
	///				Location of the second color.
	VK2D_API void									VK2D_APIENTRY					SetVertexColorGradient(
		vk2d::Colorf								color_1,
		vk2d::Colorf								color_2,
		vk2d::Vector2f								coord_1,
		vk2d::Vector2f								coord_2 );

	/// @brief		Recalculates new UV locations for each vertex so that the entire
	///				UV space is contained within the bounding box of the mesh.
	VK2D_API void									VK2D_APIENTRY					RecalculateUVsToBoundingBox();

	/// @brief		Set texture to be used when rendering this mesh object.
	/// @param[in]	texture_pointer
	///				A pointer to a texture object.
	VK2D_API void									VK2D_APIENTRY					SetTexture(
		vk2d::Texture							*	texture_pointer );

	/// @brief		Set sampler to be used when rendering this mesh object.
	/// @param[in]	sampler_pointer
	///				A pointer to a sampler object.
	VK2D_API void									VK2D_APIENTRY					SetSampler(
		vk2d::Sampler							*	sampler_pointer );

	/// @brief		If this mesh is rendered as a list of points we can define how
	///				large those points will be. This is a per vertex feature so you
	///				can manually set the size per vertex, this just sets all
	///				vertices to be the same size.
	///				This uses a GPU feature and is expected to perform well even
	///				when rendering lots of points, however it can be low quality.
	///				For more control over the quality you should render rectangles
	///				or circles instead.
	/// @param[in]	point_size
	///				Texel size of the point to be rendered.
	VK2D_API void									VK2D_APIENTRY					SetPointSize(
		float										point_size );

	/// @brief		If this mesh is rendered as a list of lines we can define the
	///				line width. This is a per draw feature and can not be controlled
	///				per vertex or segment.
	///				This uses a GPU feature and is expected to perform well even
	///				when rendering lots of lines, however it can be low quality.
	///				For more control over the quality you should render rotated
	///				rectangles or manually construct your own mesh.
	/// @param[in]	line_width
	///				Width of the line.
	VK2D_API void									VK2D_APIENTRY					SetLineWidth(
		float										line_width );

	/// @brief		Manually set mesh type to render points, lines, wireframe or
	///				triangles. This has limitations however if the mesh was created
	///				by a mesh generator. If mesh was created manually then there are
	///				no safeguards and it's up to the user of this library to make
	///				sure that the mesh data makes sense. <br>
	///				Limitations when this mesh was created by a generator:
	///				<br>
	///				<table>
	///				<tr><th> Generated mesh type	<th> Allowed new type
	///				<tr><td> Filled					<td> <tt>TRIANGLE_FILLED</tt>, <tt>TRIANGLE_WIREFRAME</tt>, <tt>POINT</tt>
	///				<tr><td> Outline				<td> <tt>LINE</tt>, <tt>POINT</tt>
	///				</table>
	/// @param[in]	type
	///				New mesh type.
	VK2D_API void									VK2D_APIENTRY					SetMeshType(
		vk2d::MeshType								type );

	/// @brief		Recalculates Axis Aligned Bounding Box from vertex coordinates.
	///				Updates vk2d::Mesh::aabb variable and returns reference to it.
	/// @return		Reference to the interal aabb variable.
	VK2D_API vk2d::Rect2f						&	VK2D_APIENTRY					RecalculateAABBFromVertices();

	/// @brief		Vertices are stored here, see vk2d::Vertex for details.
	std::vector<vk2d::Vertex>						vertices						= {};

	/// @brief		Indices are a list of offsets pointing to the vk2d::Mesh::vertices
	///				defining shape end points. How to use this list depends on what type
	///				of shape this mesh contains. To be more efficient a single vertex can
	///				be used multiple times in part of different triangles or lines. <br>
	///				- For triangles this list size must be a multiple of 3 where every
	///				group of 3 indices define a triangle and contain offsets to the
	///				vertices list defining the 3 corners for the triangle.
	///				- For lines this list size must be a multiple of 2 where every group of
	///				2 indices define a line and contain offsets to the vertices list
	///				defining the end points for the line.
	///				- For points indices are not used.
	std::vector<uint32_t>							indices							= {};

	/// @brief		Texture channel weights are used to mix between differet texture arrays per vertex.
	///
	///				The size of this list should be either 0, or the amount of vertices and the amount
	///				of layers in the used texture multiplied together. <br>
	///				For example if a texture with 3 layers is used then the first 3 indices of this
	///				list tells how much weight each texture layer has on the first vertex (in same order
	///				as texture layers are used). The next 3 indices of this list tells how much each
	///				texture layer effects the second vertex and so on. <br>
	///				For another example if a texture with 5 layers is used then the first 5 indices of
	///				this list tells how much each texture layer effects the first vertex, and the next
	///				5 after that tells how much weight each texture layer has on the second vertex and
	///				so on until we cover all vertices. <br>
	///				Texture layer weights multiply the color from the texture, for example if a 2
	///				layer texture is used then weights {0.0, 0.0} would result black transparent vertex,
	///				{1.0, 0.0} would show the first texture layer, {0.5, 0.5} would blend both texture
	///				layers together, {1.0, 1.0} would add colors of both layers together. <br>
	///				If this list is not large enough to contain all weights of all texture channels for
	///				all vertices then these values are ignored and the texture layers is chosen based
	///				on what was set to the vertex. <br>
	///				Texture layer weights between vertices are linearly interpolated so that if one
	///				texture layer weight is high for one vertex and low for the neighbouring vertex,
	///				the inbetween regions are smoothly transitioned between the two.
	std::vector<float>								texture_layer_weights			= {};

	/// @brief		Axis aligned bounding box.
	///
	///				This tells the area of within all vertex coordinates reside. It can be set manually
	///				or recalculated from vertices by calling vk2d::Mesh::RecalculateAABB().
	///				It can be used to get size of the mesh, as a fast speed preliminary check if two
	///				shapes are overlapping or to check if shape should be rendered or not.
	vk2d::Rect2f									aabb							= {};

private:
	bool											generated						= false;							///< Tells if this mesh was created by a mesh generator.
	vk2d::MeshType									generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;	///< Tells the original mesh type if it was created by a mesh generator.
	vk2d::MeshType									mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;	///< Current mesh type.
	float											line_width						= 1.0f;								///< Line width when rendering lines.
	vk2d::Texture								*	texture							= nullptr;							///< Texture resource to be used when rendering. Can be used in all modes. Can be nullptr.
	vk2d::Sampler								*	sampler							= nullptr;							///< Texture sampler to be used when rendering. Can be used in all modes. Can be nullptr for default sampler.
};



/// @brief		Generate point mesh from point list.
/// @param[in]	points
///				A list of coordinates representing point locations.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GeneratePointMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points );

/// @brief		Generate line mesh from points and connections.
/// @param[in]	points
///				A list of coordinates representing end points of lines.
/// @param[in]	indices
///				Indices defining which points should be connected with a line.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLineMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points,
	const std::vector<vk2d::VertexIndex_2>		&	indices );

/// @brief		Generate triangle mesh from points and connections.
/// @param[in]	points
///				A list of coordinates representing corners of triangles.
/// @param[in]	indices
///				Indices defining which points should form a triangle.
/// @param[in]	filled
///				true if triangle mesh is filled, false for wireframe.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateTriangleMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points,
	const std::vector<vk2d::VertexIndex_3>		&	indices,
	bool											filled							= true );

/// @brief		Generate rectangular mesh from rectangle coordinates.
/// @param[in]	area
///				Area of the rectangle that will be covered, depends on the coordinate system. See
///				vk2d::RenderCoordinateSpace for more info about what scale is used.
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateRectangleMesh(
	vk2d::Rect2f									area,
	bool											filled							= true );

/// @brief		Generate ellipse mesh from rectangle area and edge count.
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See vk2d::RenderCoordinateSpace for
///				more info about what scale is used.
/// @param[in]	filled
///				true for filled mesh, false to generate line mesh of the outline.
/// @param[in]	edge_count
///				Number of outer edges, this is a floating point value to prevent
///				popping in case it's animated.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateEllipseMesh(
	vk2d::Rect2f									area,
	bool											filled							= true,
	float											edge_count						= 64.0f );

/// @brief		Generate an ellipse or a circle that has a "slice" cut out, similar to usual pie
///				graphs.
/// @param[in]	area
///				Rectangle area in which the ellipse must fit. See vk2d::RenderCoordinateSpace for
///				more info about what scale is be used.
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// @param[in]	coverage
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the
///				full ellipse. Moving value from 0 to 1 makes "whole" pie visible in clockwise
///				direction.
/// @param[in]	filled
///				true to draw the inside of the pie, false to draw the outline only.
/// @param[in]	edge_count 
///				How many corners the complete ellipse should have, or quality if you prefer.
///				This is a float value for "smoother" transitions between amount of corners,
///				in case this value is animated.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateEllipsePieMesh(
	vk2d::Rect2f									area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true,
	float											edge_count						= 64.0f );

/// @brief		Generate a rectangular pie, similar to drawing a rectangle but which has a pie slice cut out.
/// @param[in]	area
///				Area of the rectangle. See vk2d::RenderCoordinateSpace for more info about what scale is used.
/// @param[in]	begin_angle_radians
///				Angle (in radians) where the slice cut should start. (towards positive is clockwise direction)
/// @param[in]	coverage 
///				Size of the slice, value is between 0 to 1 where 0 is not visible and 1 draws the
///				full rectangle. Moving value from 0 to 1 makes "whole" pie visible in clockwise
///				direction.
/// @param[in]	filled
///				true to draw the inside of the pie rectangle, false to draw the outline only.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateRectanglePieMesh(
	vk2d::Rect2f									area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true );

/// @brief		Generate a lattice mesh, this is useful for distortions.
/// @param[in]	area
///				Area of the rectangle. See vk2d::RenderCoordinateSpace for more info about what scale is used.
/// @param[in]	subdivisions
///				Number of inside subdivisions, eg. 2*2 will generate lattice with 4*4 rectangles.
///				If fractional values are used then inside subdivisions are moved inward from the
///				bottom right direction, this is to prevent popping in case this value is animated.
/// @param[in]	filled
///				true if the inside is filled, false to generate a lattice line mesh.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLatticeMesh(
	vk2d::Rect2f									area,
	vk2d::Vector2f									subdivisions,
	bool											filled							= true );

/// @brief		Generate a text mesh that can be used to render text.
/// @param[in]	font
///				A pointer to font resource to use.
/// @param[in]	origin
///				Left or top coordinate of what is considered the starting point for the text.
///				- For horisontal text origin is left side beginning of text and vertical
///				origin is the baseline.
///				- For vertical text origin is top beginning of text and horisontal offset is
///				middle of the symbols.
/// @param[in]	text
///				Text characters to generate.
/// @param[in]	kerning
///				Spacing between letters. Positive values are farther apart. Value is based on
///				size of the font texel size.
/// @param[in]	scale
///				Scale of the mesh to be generated. Value {1.0, 1.0} will map 1:1 to the font size
///				when render target texture or window coordinate space is either
///				vk2d::RenderCoordinateSpace::TEXEL_SPACE
///				or vk2d::RenderCoordinateSpace::TEXEL_SPACE_CENTERED.
/// @param[in]	vertical
///				true if text is generated vertically, false if horisontally.
/// @param[in]	font_face
///				Certain fonts may contain multiple font faces, this allows you to select which
///				one to use.
/// @param[in]	wait_for_resource_load
///				When generating text mesh you usually want it to be available right away.
///				Setting this option to true will wait for the font resource to load before
///				allowing execution to continue. If you're in a time critical situation
///				where you are generating the text every frame and you cannot afford a few
///				millisecond wait for the font to load up you can set this value to false. <br>
///				This function cannot generate text until the font has been loaded so
///				setting this to false will return an empty mesh until the font has been
///				fully loaded by the resource manager.
/// @return		A new mesh object.
VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateTextMesh(
	vk2d::FontResource							*	font,
	vk2d::Vector2f									origin,
	std::string										text,
	float											kerning							= 0.0f,
	vk2d::Vector2f									scale							= vk2d::Vector2f( 1.0f, 1.0f ),
	bool											vertical						= false,
	uint32_t										font_face						= 0,
	bool											wait_for_resource_load			= true );



} // vk2d
