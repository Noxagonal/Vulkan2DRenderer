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

	// TODO: Documentation...
	// Texture channel weights are used to mix between different texture's arrays per vertex,
	// the group size varies per amount of texture's arrays.
	// For example if a texture with 3 arrays is used with this mesh then texture_channel_weights
	// group size should be 3 floats per vertex. The sum of values per group should be 1.0 for
	// coherent visuals. If weights { 1.0, 0.0, 0.0 } are used on vertex then that vertex will
	// have only one active texture array layer effecting it, if weights { 0.0, 0.5, 0.5 } are
	// used then texture's array 0 has no effect, array 1 and 2 are mixed together 50% each for
	// that particular vertex, fragments inbetween vertices have different layers mixed smoothly.
	// If this is left at size 0 then only texture array layer of 0 is used for all vertices.
	std::vector<float>								texture_channel_weights			= {};

private:
	bool											generated						= false;
	vk2d::MeshType									generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;
	vk2d::MeshType									mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;
	float											line_width						= 1.0f;		// Only considered when rendering lines
	vk2d::Texture								*	texture							= nullptr;	// Texture resource to be used when rendering, can be used in all modes
	vk2d::Sampler								*	sampler							= nullptr;
};



VK2D_API vk2d::Mesh									VK2D_APIENTRY					GeneratePointMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLineMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points,
	const std::vector<vk2d::VertexIndex_2>		&	indices );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateTriangleMeshFromList(
	const std::vector<vk2d::Vector2f>			&	points,
	const std::vector<vk2d::VertexIndex_3>		&	indices,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateRectangleMesh(
	vk2d::Rect2f									area,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateEllipseMesh(
	vk2d::Rect2f									area,
	bool											filled							= true,
	float											edge_count						= 64.0f );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateEllipsePieMesh(
	vk2d::Rect2f									area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true,
	float											edge_count						= 64.0f );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateRectanglePieMesh(
	vk2d::Rect2f									area,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLatticeMesh(
	vk2d::Rect2f									area,
	vk2d::Vector2f									subdivisions,
	bool											filled							= true );

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
