
#include <core/SourceCommon.h>

#include <containers/Rect2.hpp>
#include <containers/Color.hpp>
#include <containers/Transform.hpp>

#include <interface/resources/FontResource.h>
#include <interface/resources/FontResourceImpl.h>

#include <interface/resources/TextureResource.h>
#include <interface/Texture.h>

#include <mesh/MeshBase.hpp>



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
	assert( !generated );

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
