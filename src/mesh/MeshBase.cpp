
#include <core/SourceCommon.hpp>

#include <containers/Rect2.hpp>
#include <containers/Color.hpp>
#include <containers/Transform.hpp>

#include <interface/resources/FontResource.hpp>
#include <interface/resources/font/FontResourceImpl.hpp>

#include <interface/resources/TextureResource.hpp>
#include <interface/Texture.hpp>

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
