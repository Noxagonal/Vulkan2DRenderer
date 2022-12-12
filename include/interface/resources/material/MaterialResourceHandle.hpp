#pragma once

#include <core/Common.hpp>
#include <interface/resources/resource_base/ResourceHandleBase.hpp>
#include "MaterialResource.hpp"
#include "ShaderMemberInfo.hpp"

#include <mesh/vertex/VertexBase.hpp>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<vk2d_internal::VertexBaseOrDerivedType VertexT>
class MaterialResourceHandle :
	public ResourceHandleBase<MaterialResource>
{
	using Base = ResourceHandleBase;

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline MaterialResourceHandle() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline MaterialResourceHandle(
		MaterialResource			*	resource_ptr
	) :
		Base(
			resource_ptr
		)
	{}
};



} // vk2d