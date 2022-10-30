#pragma once

#include <core/Common.h>

#include <types/MeshPrimitives.hpp>
#include <types/MeshGenerators.hpp>
#include <types/Rect2.hpp>
#include <types/Color.hpp>

#include <vector>
#include <string>



namespace vk2d {

namespace vk2d_internal {

class WindowImpl;
class RenderTargetTextureImpl;

}

class FontResource;
class Sampler;
class Texture;



class MeshBase
{
	friend class vk2d_internal::WindowImpl;
	friend class vk2d_internal::RenderTargetTextureImpl;

	friend VK2D_API void							vk2d_internal::GeneratePointMeshFromListImpl(
		MeshBase								&	mesh,
		const std::vector<glm::vec2>			&	points
	);

	friend VK2D_API void							vk2d_internal::GenerateLineMeshFromListImpl(
		MeshBase								&	mesh,
		const std::vector<glm::vec2>			&	points,
		const std::vector<VertexIndex_2>		&	indices
	);

	friend VK2D_API void							vk2d_internal::GenerateTriangleMeshFromListImpl(
		MeshBase								&	mesh,
		const std::vector<glm::vec2>			&	points,
		const std::vector<VertexIndex_3>		&	indices,
		bool										filled
	);

	friend VK2D_API void							vk2d_internal::GenerateRectangleMeshImpl(
		MeshBase								&	mesh,
		Rect2f										area,
		bool										filled
	);

	friend VK2D_API void							vk2d_internal::GenerateEllipseMeshImpl(
		MeshBase								&	mesh,
		Rect2f										area,
		bool										filled,
		float										edge_count
	);

	friend VK2D_API void							vk2d_internal::GenerateEllipsePieMeshImpl(
		MeshBase								&	mesh,
		Rect2f										area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled,
		float										edge_count
	);

	friend VK2D_API void							vk2d_internal::GenerateRectanglePieMeshImpl(
		MeshBase								&	mesh,
		Rect2f										area,
		float										begin_angle_radians,
		float										coverage,
		bool										filled
	);

	friend VK2D_API void							vk2d_internal::GenerateLatticeMeshImpl(
		MeshBase								&	mesh,
		Rect2f										area,
		glm::vec2									subdivisions,
		bool										filled
	);

	friend VK2D_API void							vk2d_internal::GenerateTextMeshImpl(
		MeshBase								&	mesh,
		FontResource							*	font,
		glm::vec2									origin,
		std::string									text,
		float										kerning,
		glm::vec2									scale,
		bool										vertical,
		uint32_t									font_face,
		bool										wait_for_resource_load
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Moves all the vertices in some direction.
	/// 
	/// @param[in]	movement
	///				A math vector that will be added to each vertex position.
	VK2D_API void									Translate(
		const glm::vec2								movement
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Rotate the mesh around a pivot point.
	/// 
	/// @param[in]	rotation_amount_radians
	///				Rotation amount in radians, positive direction is clockwise.
	/// 
	/// @param[in]	origin
	///				The pivot point of which the mesh vertices are rotated around.
	VK2D_API void									Rotate(
		float										rotation_amount_radians,
		glm::vec2									origin							= {}
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Moves all the vertices closer or father from the origin point.
	/// 
	/// @param[in]	scaling_amount
	///				0.0 scales everything into a single point, 1.0 does nothing, 2.0 doubles the current size of the mesh.
	/// 
	/// @param[in]	origin
	///				Origin point of which everything is scaled towards or away from.
	VK2D_API void									Scale(
		glm::vec2									scaling_amount,
		glm::vec2									origin							= {}
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Scews the mesh in vertical and horisontal directions based on origin point.
	/// 
	/// @param[in]	scew_amount
	///				Positive values scew vertices on bottom right of origin to be moved towards bottom right and vertices top left
	///				of origin to be moved towards top left corner. <br>
	///				Amount is a half of the other dimension. This way if you have a cube for example, setting amount to {0.0, 1.0}
	///				will result the left side of the cube to be offsetted 0.5 of the total height upwards and the right side of the
	///				cube to be offsetted 0.5 of the total height downwards. This way the total angle is 45 degrees when scewing
	///				single axis by 1.0.
	/// 
	/// @param[in]	origin
	///				Origin point of which everything is scewed in relation to.
	VK2D_API void									Scew(
		glm::vec2									scew_amount,
		glm::vec2									origin							= {}
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Makes a simple wave pattern that can be animated.
	/// 
	/// @param[in]	direction_radians
	///				Direction where the waves are moving towards. Angle is in radians.
	/// 
	/// @param[in]	frequency
	///				Frequency is the number of waves inside the mesh bounding box average size.
	/// 
	/// @param[in]	animation
	///				Animation is a float value that determines where the peaks of the waves are, slowly increasing this value will
	///				make the waves move. This value rolls over at 0.0 and 1.0. Meaning that 0.0 and 1.0 will produce the exact same
	///				pattern.
	/// 
	/// @param[in]	intensity
	///				This determines the wave peak intensity per axis. This is applied from the perspective of the direction the wave
	///				and not the final result.
	/// 
	/// @param[in]	origin
	///				Origin point where waves are generated from, since this is a directional wave this parameter only really matters
	///				if you're animating the rotation of the direction of the waves.
	VK2D_API void									DirectionalWave(
		float										direction_radians,
		float										frequency,
		float										animation,
		glm::vec2									intensity,
		glm::vec2									origin							= {}
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Moves vertices UV coordinates to some direction.
	///
	///				This directly moves the UVs to some direction so texture movement will appear exact opposite of moving vertices.
	/// 
	/// @param[in]	movement
	///				Direction to move the UVs towards. Note that this is in UV coordinates which are always in range of 0.0 to 1.0,
	///				in case a UV coordinate goes outside this range, a Sampler will determine what to do with it.
	VK2D_API void									TranslateUV(
		const glm::vec2								movement
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Rotate UV coordinates on all vertices around origin.
	///
	///				This directly scales the UVs so texture rotation will appear inverted.
	/// 
	/// @param[in]	rotation_amount_radians
	///				Rotation amount in radians, positive values will turn the UVs counter clockwise and the texture will appear to
	///				rotate counter-clockwise.
	/// 
	/// @param[in]	origin
	///				The pivot point of which the mesh UVs are rotated around. Please note that UV coordinates are always in range of
	///				0.0 to 1.0 so origin outside this range will appear to both translate and rotate.
	VK2D_API void									RotateUV(
		float										rotation_amount_radians,
		glm::vec2									origin							= { 0.5f, 0.5f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Scale UV coordinates on all vertices around origin.
	///
	///				Scaling UVs will work exactly the same as to scaling vertices except for UV coordinates, so scaling everything
	///				to 0.0 will make texture to be sampled from a single point, making texture appear infinitely large.
	/// 
	/// @param[in]	scaling_amount
	///				Scale amount, 0.0 will scale all UVs into a single point, 1.0 does nothing, 2.0 doubles the UV scale.
	/// 
	/// @param[in]	origin
	///				Origin point of which everything is scaled towards or away from.
	VK2D_API void									ScaleUV(
		glm::vec2									scaling_amount,
		glm::vec2									origin							= { 0.5f, 0.5f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Skew UV coordinates on all vertices around origin.
	///
	///				Scewing UVs works similarly to scewing vertices except for UVs. This means everything appears the opposite of
	///				the actual effect.
	/// 
	/// @param[in]	scew_amount
	///				Positive values scew UVs on bottom right of origin to be moved towards bottom right and vertices top left of
	///				origin to be moved towards top left corner. <br>
	///				Amount is a half of the other dimension. This way the total angle is 45 degrees when scewing single axis by 1.0.
	/// 
	/// @param[in]	origin
	///				Origin point of which everything is scewed in relation to.
	VK2D_API void									ScewUV(
		glm::vec2									scew_amount,
		glm::vec2									origin							= { 0.5f, 0.5f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Makes a simple wave pattern that can be animated.
	///
	///				Makes a simple wave pattern that can be animated in the UV coordinate space. This is purely artistic effect.
	///				Please note that this directly effects the UV coordinates so effects will appear inverse.
	/// 
	/// @param[in]	direction_radians
	///				Direction where the waves are moving towards. Angle is in radians.
	/// 
	/// @param[in]	frequency
	///				Frequency is the number of waves inside the 0.0 to 1.0 UV range.
	/// 
	/// @param[in]	animation
	///				Animation is a float value that determines where the peaks of the waves are, slowly increasing this value will
	///				make the waves move. This value rolls over at 0.0 and 1.0. Meaning that 0.0 and 1.0 will produce the exact same
	///				pattern.
	/// 
	/// @param[in]	intensity
	///				This determines the wave peak intensity per axis. This is applied from the perspective of the direction the wave
	///				and not the final result.
	/// 
	/// @param[in]	origin
	///				Origin point where waves are generated from, since this is a directional wave this parameter only really matters
	///				if you're animating the rotation of the direction of the waves.
	VK2D_API void									DirectionalWaveUV(
		float										direction_radians,
		float										frequency,
		float										animation,
		glm::vec2									intensity,
		glm::vec2									origin							= { 0.5f, 0.5f }
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set all vertices to a specific color.
	/// 
	/// @param[in]	new_color
	///				Color to apply to all vertices.
	VK2D_API void									SetVertexColor(
		Colorf										new_color
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Set color to all vertices following linear gradient.
	///
	///				Gradient is generated between two points.
	/// 
	/// @param[in]	color_1
	///				First color.
	/// 
	/// @param[in]	color_2
	///				Second color.
	/// 
	/// @param[in]	coord_1
	///				Location of the first color.
	/// 
	/// @param[in]	coord_2
	///				Location of the second color.
	VK2D_API void									SetVertexColorGradient(
		Colorf										color_1,
		Colorf										color_2,
		glm::vec2									coord_1,
		glm::vec2									coord_2
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Recalculates new UV locations for each vertex to be within the bounding box.
	///
	///				This makes the uv coordinates for each vertex to be inside of the UV map.
	VK2D_API void									RecalculateUVsToBoundingBox();

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
	/// @brief		Set size of a single vertex when rendering point meshes.
	///
	///				If this mesh is rendered as a list of points we can define how large those points will be. This is a per vertex
	///				feature so you can manually set the size per vertex, this just sets all vertices to be the same size. This uses
	///				a GPU feature and is expected to perform well even when rendering lots of points, however it can be low quality.
	///				For more control over the quality you should render rectangles or circles instead.
	/// 
	/// @param[in]	point_size
	///				Texel size of the point to be rendered.
	VK2D_API void									SetPointSize(
		float										point_size
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
	/// @brief		Recalculates Axis Aligned Bounding Box from vertex coordinates.
	/// 
	///				Updates Mesh::aabb variable and returns reference to it.
	/// 
	/// @return		Reference to the interal aabb variable.
	VK2D_API Rect2f								&	RecalculateAABBFromVertices();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Vertices list.
	///
	/// @see		Vertex
	std::vector<Vertex>								vertices						= {};

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
