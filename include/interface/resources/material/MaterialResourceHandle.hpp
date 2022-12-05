#pragma once

#include <core/Common.hpp>
#include <interface/resources/resource_base/ResourceHandleBase.hpp>
#include "MaterialResource.hpp"
#include "ShaderMemberInfo.hpp"

#include <mesh/vertex/VertexBase.hpp>



namespace vk2d {

class ResourceManager;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseOrDerivedType VertexT>
class MaterialResourceHandle :
	public ResourceHandleBase<MaterialResource, ResourceManager>
{
	using Base = ResourceHandleBase;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MaterialResourceHandle() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MaterialResourceHandle(
		ResourceManager				*	resource_manager,
		MaterialResource			*	resource_ptr
	) :
		Base(
			resource_manager,
			resource_ptr
		)
	{}
};



} // vk2d
