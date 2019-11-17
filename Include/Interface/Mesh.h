#pragma once

#include "../Core/Common.h"

#include "RenderPrimitives.h"

#include <vector>

namespace vk2d {

class TextureResource;



enum class MeshType : uint32_t {
	TRIANGLE_FILLED,
	TRIANGLE_WIREFRAME,
	LINE,
	POINT,
};



class Mesh {
public:
	VK2D_API void								VK2D_APIENTRY					Translate(
		const vk2d::Vector2d					movement );

	VK2D_API void								VK2D_APIENTRY					Rotate(
		float									rotate );

	VK2D_API void								VK2D_APIENTRY					Scale(
		vk2d::Vector2d							scaling );

	VK2D_API void								VK2D_APIENTRY					Scew(
		vk2d::Vector2d							scew_amount );

	VK2D_API void								VK2D_APIENTRY					TranslateUV(
		const vk2d::Vector2d					movement );

	VK2D_API void								VK2D_APIENTRY					RotateUV(
		float									rotate );

	VK2D_API void								VK2D_APIENTRY					ScaleUV(
		vk2d::Vector2d							scaling );

	VK2D_API void								VK2D_APIENTRY					ScewUV(
		vk2d::Vector2d							scew_amount );

	VK2D_API void								VK2D_APIENTRY					SetVertexColor(
		vk2d::Color								new_color );

	VK2D_API void								VK2D_APIENTRY					SetVertexColorGradient(
		vk2d::Color								color_1,
		vk2d::Color								color_2,
		vk2d::Vector2d							coord_1,
		vk2d::Vector2d							coord_2 );

	VK2D_API void								VK2D_APIENTRY					UVToBoundingBox();

	std::vector<vk2d::Vertex>					vertices						= {};
	std::vector<uint32_t>						indices							= {};

	bool										created_by_generator			= {};		// This should probably be hidden
	vk2d::MeshType								mesh_type						= vk2d::MeshType::TRIANGLE_FILLED;	// This should probably be hidden
	float										line_width						= 1.0f;		// Only considered when rendering lines
	vk2d::TextureResource					*	texture							= nullptr;	// Texture resource to be used when rendering, can be used in all modes
};



VK2D_API vk2d::Mesh								VK2D_APIENTRY					GeneratePointMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GenerateLineMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points,
	const std::vector<vk2d::VertexIndex_2>	&	indices );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GenerateTriangleMeshFromList(
	const std::vector<vk2d::Vector2d>		&	points,
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	bool										filled							= true );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GenerateBoxMesh(
	vk2d::Vector2d								top_left,
	vk2d::Vector2d								bottom_right,
	bool										filled							= true );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GenerateCircleMesh(
	vk2d::Vector2d								top_left,
	vk2d::Vector2d								bottom_right,
	bool										filled							= true,
	float										edge_count						= 64.0f );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GeneratePieMesh(
	vk2d::Vector2d								top_left,
	vk2d::Vector2d								bottom_right,
	float										begin_angle_radians,
	float										coverage,
	bool										filled							= true,
	float										edge_count						= 64.0f );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GeneratePieBoxMesh(
	vk2d::Vector2d								top_left,
	vk2d::Vector2d								bottom_right,
	float										begin_angle_radians,
	float										coverage,
	bool										filled							= true );

VK2D_API vk2d::Mesh								VK2D_APIENTRY					GenerateLatticeMesh(
	vk2d::Vector2d								top_left,
	vk2d::Vector2d								bottom_right,
	vk2d::Vector2d								subdivisions,
	bool										filled							= true );



} // vk2d
