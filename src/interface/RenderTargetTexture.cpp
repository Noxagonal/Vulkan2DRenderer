
#include <core/SourceCommon.h>

#include "RenderTargetTextureImpl.h"
#include <mesh/generators/MeshGenerators.hpp>
#include <mesh/modifiers/MeshModifiers.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::RenderTargetTexture::RenderTargetTexture(
	vk2d_internal::InstanceImpl					&	instance,
	const RenderTargetTextureCreateInfo			&	create_info
)
{
	impl = std::make_unique<vk2d_internal::RenderTargetTextureImpl>(
		*this,
		instance,
		create_info
	);
	if( !impl || !impl->IsGood() ) {
		impl = nullptr;
	}

	texture_impl = impl.get();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API vk2d::RenderTargetTexture::~RenderTargetTexture()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::SetRenderCoordinateSpace(
	RenderCoordinateSpace coordinate_space
)
{
	impl->SetRenderCoordinateSpace( coordinate_space );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::SetSize(
	glm::uvec2		new_size
)
{
	impl->SetSize(
		new_size
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API glm::uvec2 vk2d::RenderTargetTexture::GetSize() const
{
	return impl->GetSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API uint32_t vk2d::RenderTargetTexture::GetLayerCount() const
{
	return impl->GetLayerCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::RenderTargetTexture::IsTextureDataReady()
{
	return impl->IsTextureDataReady();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::RenderTargetTexture::BeginRender()
{
	return impl->BeginRender();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::RenderTargetTexture::EndRender(
	glm::vec2	blur_amount,
	BlurType	blur_type
)
{
	return impl->EndRender( blur_type, blur_amount );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawPoint(
	glm::vec2	location,
	Colorf		color,
	float		size
)
{
	auto mesh = mesh_generators::GeneratePointMeshFromList(
		{ location }
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	mesh_modifiers::SetVerticesPointSize( mesh, size );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawLine(
	glm::vec2	point_1,
	glm::vec2	point_2,
	Colorf		color,
	float		line_width
)
{
	auto mesh = mesh_generators::GenerateLineMeshFromList(
		{ point_1, point_2 },
		{ { 0, 1 } }
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	mesh.SetLineWidth( line_width );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawRectangle(
	Rect2f		area,
	bool		filled,
	Colorf		color
)
{
	auto mesh = mesh_generators::GenerateRectangleMesh(
		area,
		filled
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawEllipse(
	Rect2f		area,
	bool		filled,
	float		edge_count,
	Colorf		color
)
{
	auto mesh = mesh_generators::GenerateEllipseMesh(
		area,
		filled,
		edge_count
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawEllipsePie(
	Rect2f					area,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	float							edge_count,
	Colorf					color
)
{
	auto mesh = mesh_generators::GenerateEllipsePieMesh(
		area,
		begin_angle_radians,
		coverage,
		filled,
		edge_count
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawRectanglePie(
	Rect2f			area,
	float			begin_angle_radians,
	float			coverage,
	bool			filled,
	Colorf			color
)
{
	auto mesh = mesh_generators::GenerateRectanglePieMesh(
		area,
		begin_angle_radians,
		coverage,
		filled
	);
	mesh_modifiers::SetVerticesColor( mesh, color );
	DrawMesh( mesh, std::vector { glm::mat4( 1.0f ) } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawTexture(
	glm::vec2					top_left,
	Texture					*	texture,
	Colorf						color
)
{
	if( texture ) {
		auto texture_size = texture->GetSize();
		auto bottom_right = top_left + glm::vec2( float( texture_size.x ), float( texture_size.y ) );
		auto mesh = mesh_generators::GenerateRectangleMesh(
			{ top_left, bottom_right }
		);
		mesh.SetTexture( texture );
		mesh_modifiers::SetVerticesColor( mesh, color );
		DrawMesh( mesh, std::vector{ glm::mat4( 1.0f ) } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawPointList(
	const vk2d_internal::RawVertexData	&	raw_vertex_data,
	std::span<const float>					texture_layer_weights,
	std::span<const glm::mat4>				transformations,
	Texture								*	texture,
	Sampler								*	sampler
)
{
	impl->DrawPointList(
		raw_vertex_data,
		texture_layer_weights,
		transformations,
		texture,
		sampler
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawLineList(
	std::span<const uint32_t>				indices,
	const vk2d_internal::RawVertexData	&	raw_vertex_data,
	std::span<const float>					texture_layer_weights,
	std::span<const glm::mat4>				transformations,
	Texture								*	texture,
	Sampler								*	sampler,
	float									line_width
)
{
	impl->DrawLineList(
		indices,
		raw_vertex_data,
		texture_layer_weights,
		transformations,
		texture,
		sampler,
		line_width
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API void vk2d::RenderTargetTexture::DrawTriangleList(
	std::span<const uint32_t>				indices,
	const vk2d_internal::RawVertexData	&	raw_vertex_data,
	std::span<const float>					texture_layer_weights,
	std::span<const glm::mat4>				transformations,
	bool									filled,
	Texture								*	texture,
	Sampler								*	sampler
)
{
	impl->DrawTriangleList(
		indices,
		raw_vertex_data,
		texture_layer_weights,
		transformations,
		filled,
		texture,
		sampler
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VK2D_API bool vk2d::RenderTargetTexture::IsGood() const
{
	return !!impl;
}
