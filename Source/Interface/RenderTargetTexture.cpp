
#include "../Header/Core/SourceCommon.h"

#include "../../Include/Interface/RenderTargetTexture.h"
#include "../Header/Impl/RenderTargetTextureImpl.h"



VK2D_API vk2d::RenderTargetTexture::RenderTargetTexture(
	vk2d::_internal::InstanceImpl						*	instance,
	const vk2d::RenderTargetTextureCreateInfo			&	create_info
)
{
	impl = std::make_unique<vk2d::_internal::RenderTargetTextureImpl>(
		instance,
		create_info
	);

	if( impl && impl->IsGood() ) {
		texture_impl	= &*impl;
		is_good			= true;
	} else {
		is_good			= false;
		impl			= nullptr;
	}
}


VK2D_API vk2d::RenderTargetTexture::~RenderTargetTexture()
{}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::SetSize(
	vk2d::Vector2u		new_size
)
{
	impl->SetSize(
		new_size
	);
}

VK2D_API vk2d::Vector2u VK2D_APIENTRY vk2d::RenderTargetTexture::GetSize() const
{
	return impl->GetSize();
}

VK2D_API uint32_t VK2D_APIENTRY vk2d::RenderTargetTexture::GetLayerCount() const
{
	return impl->GetLayerCount();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::WaitUntilLoaded()
{
	return impl->WaitUntilLoaded();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::BeginRender()
{
	return impl->BeginRender();
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::EndRender()
{
	return impl->EndRender();
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawTriangleList(
	const std::vector<vk2d::VertexIndex_3>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	bool										filled,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawTriangleList(
		indices,
		vertices,
		texture_channels,
		filled,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawLineList(
	const std::vector<vk2d::VertexIndex_2>	&	indices,
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawLineList(
		indices,
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPointList(
	const std::vector<vk2d::Vertex>			&	vertices,
	const std::vector<float>				&	texture_channels,
	vk2d::Texture							*	texture,
	vk2d::Sampler							*	sampler
)
{
	impl->DrawPointList(
		vertices,
		texture_channels,
		texture
	);
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPoint(
	vk2d::Vector2f			location,
	vk2d::Colorf			color,
	float					size
)
{
	auto mesh = vk2d::GeneratePointMeshFromList(
		{ location }
	);
	mesh.SetVertexColor( color );
	mesh.SetPointSize( size );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawLine(
	vk2d::Vector2f					point_1,
	vk2d::Vector2f					point_2,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateLineMeshFromList(
		{ point_1, point_2 },
		{ { 0, 1 } }
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateBoxMesh(
		top_left,
		bottom_right,
		filled
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawCircle(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GenerateCircleMesh(
		top_left,
		bottom_right,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPie(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	float							edge_count,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GeneratePieMesh(
		top_left,
		bottom_right,
		begin_angle_radians,
		coverage,
		filled,
		edge_count
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawPieBox(
	vk2d::Vector2f					top_left,
	vk2d::Vector2f					bottom_right,
	float							begin_angle_radians,
	float							coverage,
	bool							filled,
	vk2d::Colorf					color
)
{
	auto mesh = vk2d::GeneratePieBoxMesh(
		top_left,
		bottom_right,
		begin_angle_radians,
		coverage,
		filled
	);
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawTexture(
	vk2d::Vector2f				top_left,
	vk2d::Vector2f				bottom_right,
	vk2d::Texture			*	texture,
	vk2d::Colorf				color
)
{
	auto mesh = vk2d::GenerateBoxMesh(
		top_left,
		bottom_right
	);
	mesh.SetTexture( texture );
	mesh.SetVertexColor( color );
	impl->DrawMesh( mesh );
}

VK2D_API void VK2D_APIENTRY vk2d::RenderTargetTexture::DrawMesh(
	const vk2d::Mesh	&	mesh
)
{
	impl->DrawMesh(
		mesh
	);
}

VK2D_API bool VK2D_APIENTRY vk2d::RenderTargetTexture::IsGood() const
{
	return is_good;
}
