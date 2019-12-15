#pragma once

#include "../Core/Common.h"

#include "RenderPrimitives.h"

#include <vector>

namespace vk2d {
namespace _internal {
class WindowImpl;
}

class TextureResource;



enum class MeshType : uint32_t {
	TRIANGLE_FILLED,
	TRIANGLE_WIREFRAME,
	LINE,
	POINT,
};



class Mesh {
	friend class vk2d::_internal::WindowImpl;
	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GeneratePointMeshFromList(
		const std::vector<vk2d::Vector2d>		&	points );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateLineMeshFromList(
		const std::vector<vk2d::Vector2d>		&	points,
		const std::vector<vk2d::VertexIndex_2>	&	indices );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateTriangleMeshFromList(
		const std::vector<vk2d::Vector2d>		&	points,
		const std::vector<vk2d::VertexIndex_3>	&	indices,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateBoxMesh(
		vk2d::Vector2d								top_left,
		vk2d::Vector2d								bottom_right,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateCircleMesh(
		vk2d::Vector2d								top_left,
		vk2d::Vector2d								bottom_right,
		bool										filled,
		float										edge_count );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GeneratePieMesh(
		vk2d::Vector2d								top_left,
		vk2d::Vector2d								bottom_right,
		float										begin_angle_radians,
		float										coverage,
		bool										filled,	
		float										edge_count );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GeneratePieBoxMesh(
		vk2d::Vector2d								top_left,
		vk2d::Vector2d								bottom_right,
		float										begin_angle_radians,
		float										coverage,
		bool										filled );

	friend VK2D_API vk2d::Mesh						VK2D_APIENTRY					GenerateLatticeMesh(
		vk2d::Vector2d								top_left,
		vk2d::Vector2d								bottom_right,
		vk2d::Vector2d								subdivisions,
		bool										filled );

public:
	VK2D_API void									VK2D_APIENTRY					Translate(
		const vk2d::Vector2d						movement );

	VK2D_API void									VK2D_APIENTRY					Rotate(
		float										rotation_amount_radians,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Scale(
		vk2d::Vector2d								scaling_amount,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Scew(
		vk2d::Vector2d								scew_amount,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					Wave(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2d								intensity,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					TranslateUV(
		const vk2d::Vector2d						movement );

	VK2D_API void									VK2D_APIENTRY					RotateUV(
		float										rotation_amount_radians,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					ScaleUV(
		vk2d::Vector2d								scaling_amount,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					ScewUV(
		vk2d::Vector2d								scew_amount,
		vk2d::Vector2d								origin							= {} );

	VK2D_API void									VK2D_APIENTRY					WaveUV(
		float										direction_radians,
		float										frequency,
		float										animation,
		vk2d::Vector2d								intensity,
		vk2d::Vector2d								origin							= { 0.5f, 0.5f } );

	VK2D_API void									VK2D_APIENTRY					SetVertexColor(
		vk2d::Color									new_color );

	VK2D_API void									VK2D_APIENTRY					SetVertexColorGradient(
		vk2d::Color									color_1,
		vk2d::Color									color_2,
		vk2d::Vector2d								coord_1,
		vk2d::Vector2d								coord_2 );

	VK2D_API void									VK2D_APIENTRY					ConfineUVToBoundingBox();

	VK2D_API void									VK2D_APIENTRY					SetTexture(
		vk2d::TextureResource					*	texture_resource_pointer );

	VK2D_API void									VK2D_APIENTRY					SetPointSize(
		float										point_size );

	VK2D_API void									VK2D_APIENTRY					SetLineSize(
		float										line_width );

	VK2D_API void									VK2D_APIENTRY					SetMeshType(
		vk2d::MeshType								type );

	std::vector<vk2d::Vertex>						vertices						= {};
	std::vector<uint32_t>							indices							= {};

private:
	bool											generated						= false;
	vk2d::MeshType									generated_mesh_type				= vk2d::MeshType::TRIANGLE_FILLED;
	vk2d::MeshType									mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;
	float											line_width						= 1.0f;		// Only considered when rendering lines
	vk2d::TextureResource						*	texture							= nullptr;	// Texture resource to be used when rendering, can be used in all modes
};



VK2D_API vk2d::Mesh									VK2D_APIENTRY					GeneratePointMeshFromList(
	const std::vector<vk2d::Vector2d>			&	points );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLineMeshFromList(
	const std::vector<vk2d::Vector2d>			&	points,
	const std::vector<vk2d::VertexIndex_2>		&	indices );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateTriangleMeshFromList(
	const std::vector<vk2d::Vector2d>			&	points,
	const std::vector<vk2d::VertexIndex_3>		&	indices,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateBoxMesh(
	vk2d::Vector2d									top_left,
	vk2d::Vector2d									bottom_right,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateCircleMesh(
	vk2d::Vector2d									top_left,
	vk2d::Vector2d									bottom_right,
	bool											filled							= true,
	float											edge_count						= 64.0f );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GeneratePieMesh(
	vk2d::Vector2d									top_left,
	vk2d::Vector2d									bottom_right,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true,
	float											edge_count						= 64.0f );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GeneratePieBoxMesh(
	vk2d::Vector2d									top_left,
	vk2d::Vector2d									bottom_right,
	float											begin_angle_radians,
	float											coverage,
	bool											filled							= true );

VK2D_API vk2d::Mesh									VK2D_APIENTRY					GenerateLatticeMesh(
	vk2d::Vector2d									top_left,
	vk2d::Vector2d									bottom_right,
	vk2d::Vector2d									subdivisions,
	bool											filled							= true );



} // vk2d
