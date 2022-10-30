
#include <core/SourceCommon.h>

#include <types/Rect2.hpp>
#include <types/Color.hpp>
#include <types/Transform.h>

#include <interface/resources/FontResource.h>
#include <interface/resources/FontResourceImpl.h>

#include <interface/resources/TextureResource.h>
#include <interface/Texture.h>

#include <types/MeshBase.hpp>
#include <types/MeshCommon.hpp>



VK2D_API void vk2d::MeshBase::Translate(
	const glm::vec2			movement )
{
	for( auto & i : vertices ) {
		i.vertex_coords		+= movement;
	}
}

VK2D_API void vk2d::MeshBase::Rotate(
	float					rotation_amount_radians,
	glm::vec2				origin )
{
	auto rotation_matrix	= CreateRotationMatrix2( rotation_amount_radians );

	for( auto & i : vertices ) {
		i.vertex_coords		-= origin;
		i.vertex_coords		= rotation_matrix * i.vertex_coords;
		i.vertex_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::Scale(
	glm::vec2				scaling_amount,
	glm::vec2				origin
)
{
	for( auto & i : vertices ) {
		i.vertex_coords		-= origin;
		i.vertex_coords		*= scaling_amount;
		i.vertex_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::Scew(
	glm::vec2				scew_amount,
	glm::vec2				origin
)
{
	for( auto & i : vertices ) {
		auto c = i.vertex_coords - origin;
		i.vertex_coords.x	= c.y * scew_amount.x + c.x;
		i.vertex_coords.y	= c.x * scew_amount.y + c.y;
		i.vertex_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::DirectionalWave(
	float						direction_radians,
	float						frequency,
	float						animation,
	glm::vec2					intensity,
	glm::vec2					origin
)
{
	auto size	= aabb.bottom_right - aabb.top_left;

	glm::vec2	dir {
		std::cos( direction_radians ),
		std::sin( direction_radians )
	};
	// Matrix input has flipped column and row order.
	auto forward_rotation_matrix = glm::mat2(
		+dir.x, +dir.y,
		-dir.y, +dir.x
	);
	auto backward_rotation_matrix = glm::mat2(
		+dir.x, -dir.y,
		+dir.y, +dir.x
	);

	for( auto & i : vertices ) {
		auto c		= i.vertex_coords - origin;
		c			= backward_rotation_matrix * c;

		auto d		= float( animation / RAD ) + float( frequency * ( ( c.x / size.x + c.y / size.y ) / 2.0f ) * RAD );
		i.vertex_coords		= {
			std::cos( d ) * intensity.x + c.x,
			std::sin( d ) * intensity.y + c.y };

		i.vertex_coords		= forward_rotation_matrix * i.vertex_coords;
		i.vertex_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::TranslateUV(
	const glm::vec2			movement
)
{
	for( auto & i : vertices ) {
		i.uv_coords		+= movement;
	}
}

VK2D_API void vk2d::MeshBase::RotateUV(
	float					rotation_amount_radians,
	glm::vec2				origin
)
{
	auto rotation_matrix	= CreateRotationMatrix2( rotation_amount_radians );

	for( auto & i : vertices ) {
		i.uv_coords		-= origin;
		i.uv_coords		= rotation_matrix * i.uv_coords;
		i.uv_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::ScaleUV(
	glm::vec2				scaling_amount,
	glm::vec2				origin
)
{
	for( auto & i : vertices ) {
		i.uv_coords		-= origin;
		i.uv_coords		*= scaling_amount;
		i.uv_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::ScewUV(
	glm::vec2				scew_amount,
	glm::vec2				origin
)
{
	for( auto & i : vertices ) {
		auto c = i.uv_coords - origin;
		i.uv_coords.x	= c.y * scew_amount.x + c.x;
		i.uv_coords.y	= c.x * scew_amount.y + c.y;
		i.uv_coords		+= origin;
	}
}

VK2D_API void vk2d::MeshBase::DirectionalWaveUV(
	float					direction_radians,
	float					frequency,
	float					animation,
	glm::vec2				intensity,
	glm::vec2				origin
)
{
	glm::vec2	dir {
		std::cos( direction_radians ),
		std::sin( direction_radians )
	};
	// Matrix input has flipped column and row order.
	auto forward_rotation_matrix = glm::mat2(
		+dir.x, +dir.y,
		-dir.y, +dir.x
	);
	auto backward_rotation_matrix = glm::mat2(
		+dir.x, -dir.y,
		+dir.y, +dir.x
	);

	for( auto & i : vertices ) {
		auto c				= i.uv_coords - origin;
		c					= backward_rotation_matrix * c;

		auto d				= float( animation / RAD ) + float( frequency * ( ( c.x + c.y ) / 2.0f ) * RAD );
		i.uv_coords			= {
			std::cos( d ) * intensity.x + c.x,
			std::sin( d ) * intensity.y + c.y };

		i.uv_coords			= forward_rotation_matrix * i.uv_coords;
		i.uv_coords			+= origin;
	}
}

VK2D_API void vk2d::MeshBase::SetVertexColor(
	Colorf				new_color )
{
	for( auto & v : vertices ) {
		v.color		= new_color;
	}
}

VK2D_API void vk2d::MeshBase::SetVertexColorGradient(
	Colorf			color_1,
	Colorf			color_2,
	glm::vec2		coord_1,
	glm::vec2		coord_2
)
{
	glm::vec2		coord_vector	= coord_2 - coord_1;
	glm::vec2		coord_dir		= {};
	float			coord_lenght	= std::sqrt( coord_vector.x * coord_vector.x + coord_vector.y * coord_vector.y );
	if( coord_lenght > 0.0f ) {
		coord_dir					= coord_vector / coord_lenght;
	} else {
		coord_lenght				= KINDA_SMALL_VALUE;
		coord_dir					= { 1.0f, 0.0f };
	}

	// Matrix input has flipped column and row order.
	auto forward_rotation_matrix = glm::mat2(
		+coord_dir.x, +coord_dir.y,
		-coord_dir.y, +coord_dir.x
	);
	auto backward_rotation_matrix = glm::mat2(
		+coord_dir.x, -coord_dir.y,
		+coord_dir.y, +coord_dir.x
	);
	auto coord_linearilized = backward_rotation_matrix * ( coord_dir * coord_lenght );

	for( auto & v : vertices ) {
		auto c = v.vertex_coords - coord_1;

		c = backward_rotation_matrix * c;
		auto cx = c.x / coord_linearilized.x;

		if( cx < 0.0f )			v.color = color_1;
		else if( cx > 1.0f )	v.color = color_2;
		else {
			auto g = Colorf(
				color_1.r * ( 1.0f - cx ) + color_2.r * cx,
				color_1.g * ( 1.0f - cx ) + color_2.g * cx,
				color_1.b * ( 1.0f - cx ) + color_2.b * cx,
				color_1.a * ( 1.0f - cx ) + color_2.a * cx
			);
			v.color		= g;
			//v.color = { 0, 1, 0, 1 };
		}
		c = forward_rotation_matrix * c;

		v.vertex_coords = c + coord_1;
	}
}

VK2D_API void vk2d::MeshBase::RecalculateUVsToBoundingBox()
{
	auto size = aabb.bottom_right - aabb.top_left;
	for( auto & v : vertices ) {
		auto vp = v.vertex_coords - aabb.top_left;
		v.uv_coords		= vp / size;
	}
}

VK2D_API void vk2d::MeshBase::SetTexture(
	Texture * texture_pointer
)
{
	texture		= texture_pointer;
}

VK2D_API void vk2d::MeshBase::SetSampler(
	Sampler * sampler_pointer
)
{
	sampler		= sampler_pointer;
}

VK2D_API void vk2d::MeshBase::SetPointSize(
	float point_size
)
{
	for( auto & v : vertices ) {
		v.point_size = point_size;
	}
}

VK2D_API void vk2d::MeshBase::SetLineWidth(
	float line_width
)
{
	this->line_width = line_width;
}

VK2D_API void vk2d::MeshBase::SetMeshType(
	MeshType type
)
{
	if( generated ) {
		mesh_type	= type;
		return;
	}

	switch( mesh_type ) {
	case MeshType::TRIANGLE_FILLED:
		switch( type ) {
		case MeshType::TRIANGLE_FILLED:
			mesh_type	= type;
			break;
		case MeshType::TRIANGLE_WIREFRAME:
			mesh_type	= type;
			break;
		case MeshType::LINE:
//			mesh_type	= type;
			break;
		case MeshType::POINT:
			mesh_type	= type;
			break;
		default:
			break;
		}
		break;
	case MeshType::TRIANGLE_WIREFRAME:
		switch( type ) {
		case MeshType::TRIANGLE_FILLED:
			mesh_type	= type;
			break;
		case MeshType::TRIANGLE_WIREFRAME:
			mesh_type	= type;
			break;
		case MeshType::LINE:
//			mesh_type	= type;
			break;
		case MeshType::POINT:
			mesh_type	= type;
			break;
		default:
			break;
		}
		break;
	case MeshType::LINE:
		switch( type ) {
		case MeshType::TRIANGLE_FILLED:
//			mesh_type	= type;
			break;
		case MeshType::TRIANGLE_WIREFRAME:
//			mesh_type	= type;
			break;
		case MeshType::LINE:
			mesh_type	= type;
			break;
		case MeshType::POINT:
			mesh_type	= type;
			break;
		default:
			break;
		}
		break;
	case MeshType::POINT:
		// Point cannot be anything else.
		break;
	default:
		break;
	}
}

VK2D_API vk2d::Rect2f & vk2d::MeshBase::RecalculateAABBFromVertices()
{
	if( std::size( vertices ) > 0 ) {
		return aabb = vk2d_internal::CalculateAABBFromVertexList( vertices );
	} else {
		return aabb = {};
	}
}
