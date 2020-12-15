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



// TODO: Make MeshImpl as well.
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
	VK2D_API void									VK2D_APIENTRY					Translate(
		const vk2d::Vector2f						movement );

	VK2D_API void									VK2D_APIENTRY					Rotate(
		float										rotation_amount_radians,
		vk2d::Vector2f								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Scale(
		vk2d::Vector2f								scaling_amount,
		vk2d::Vector2f								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Scew(
		vk2d::Vector2f								scew_amount,
		vk2d::Vector2f								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Wave(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2f								intensity,
		vk2d::Vector2f								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					TranslateUV(
		const vk2d::Vector2f						movement );

	VK2D_API void									VK2D_APIENTRY					RotateUV(
		float										rotation_amount_radians,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	VK2D_API void									VK2D_APIENTRY					ScaleUV(
		vk2d::Vector2f								scaling_amount,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	VK2D_API void									VK2D_APIENTRY					ScewUV(
		vk2d::Vector2f								scew_amount,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	VK2D_API void									VK2D_APIENTRY					WaveUV(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2f								intensity,
		vk2d::Vector2f								origin							= { 0.5f, 0.5f } );

	VK2D_API void									VK2D_APIENTRY					SetVertexColor(
		vk2d::Colorf								new_color );

	VK2D_API void									VK2D_APIENTRY					SetVertexColorGradient(
		vk2d::Colorf								color_1,
		vk2d::Colorf								color_2,
		vk2d::Vector2f								coord_1,
		vk2d::Vector2f								coord_2 );

	VK2D_API void									VK2D_APIENTRY					ConfineUVToBoundingBox();

	VK2D_API void									VK2D_APIENTRY					SetTexture(
		vk2d::Texture							*	texture_resource_pointer );

	VK2D_API void									VK2D_APIENTRY					SetSampler(
		vk2d::Sampler							*	sampler_pointer );

	VK2D_API void									VK2D_APIENTRY					SetPointSize(
		float										point_size );

	VK2D_API void									VK2D_APIENTRY					SetLineWidth(
		float										line_width );

	VK2D_API void									VK2D_APIENTRY					SetMeshType(
		vk2d::MeshType								type );

	// Vertices are stored here, see vk2d::Vertex for details.
	std::vector<vk2d::Vertex>						vertices						= {};

	// Indices are stored here, for triangles indices are in groups of 3,
	// and for lines indices are in groups of 2.
	std::vector<uint32_t>							indices							= {};

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
