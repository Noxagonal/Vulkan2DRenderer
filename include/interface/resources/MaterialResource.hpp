#pragma once

#include <core/Common.hpp>

#include <interface/resources/ResourceBase.hpp>

#include <filesystem>



namespace vk2d {
namespace vk2d_internal {

class ResourceManagerImpl;
class PipelineResourceImplBase;
class MaterialResourceImpl;

} // vk2d_internal



//
// Development notes:
// Material resource will eventually take over some mesh parameters, like texture and sampler, as well as introduce ability to
// add custom shaders.
//
// Material resource allows finding and creating required resources ahead of time in another thread, which makes it possible to
// load new materials while the main loop is running without interrupting the main rendering loop too much. Idea is to keep the
// main rendering loop as stutter free as possible.
//
// Potential problems and other considerations:
// - Rendering points, lines and triangles requires a new Vulkan pipeline. Some meshes are incompatible with some methods of
//   rendering. Eg. triangles may be rendered as triangles, triangle lines or points but not as lines. MeshBase already has
//   info how the mesh was generated so that's a start. Currently however, I don't think it's enough.
//   Potential solution is to limit materials to specific types of meshes, which would mean that the material determines the
//   mesh type rather than the mesh itself, if the mesh type is not compatible with the material's mesh type, then it cannot be
//   rendered. Another option is to keep mesh type inside the mesh and allow switching it on the fly... But this would need to
//   trigger a Vulkan pipeline recreation, which would introduce microstutters if the correct pipeline isn't found. I don't like
//   that idea much.
//
// - Where to store texture resource. We could keep the texture resource in the mesh, which would allow easily switching it on
//   the fly without recreating the material, could store it in the material too though. Recreating material with only the
//   texture being changed is very cheap as the pipeline is just going to be reused. It should be pretty easy to keep the
//   texture in the material and allow changing it at any time, it may be a little odd however to be able to change the texture
//   but not the mesh type or shaders.
//
// - Where to store sampler. Same thing as textures, can be changed in the fly. Sampler is very close to what people might
//   consider to be part of the material properties, it would be possible to introduce sampler as part of the material itself
//   instead as a separate object.
//
// - Material could determine the number of textures and samplers used.
//
// - Material must be compatible with mesh, ( see vk2d::Mesh template parameter pack ), material should also
//   use template parameter pack with matching types to allow compile time checking. The material determines the shader
//   interface for the vertex and the mesh determines the data that is sent to the shader.
//
// - In addition to per vertex parameters determined by the mesh, we need per draw parameters. In essense we need 2 template
//   parameter packs. Need to figure out how to introduce this in an easy way.
//   Potential solution below:
//		template<typename...>
//		struct Mesh {
//			using IsMeshType = std::true_type;
//		};
//		
//		template<typename...>
//		struct Draw {
//			using IsDrawType = std::true_type;
//		};
//		
//		template<typename First, typename Second>
//		concept TestTypes = requires {
//			typename First::IsMeshType;
//			typename Second::IsDrawType;
//		};
//		
//		template<
//			typename First,
//			typename Second
//		>
//			requires( TestTypes<First, Second> )
//		class Test
//		{};
//		
//		template<
//			typename ...TypesFirst,
//			typename ...TypesSecond
//		>
//		class Test<
//			Mesh<TypesFirst...>,
//			Draw<TypesSecond...>
//		> {};
//		
//		int main() {
//			Test<Mesh<float, float>, Draw<int>> a;
//			return 0;
//		}
//



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Material determines how the mesh is being drawn.
class MaterialResource :
	public ResourceBase
{
	friend class vk2d_internal::ResourceManagerImpl;
	friend class vk2d_internal::ResourceThreadLoadTask;
	friend class vk2d_internal::ResourceThreadUnloadTask;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API													MaterialResource(
		vk2d_internal::ResourceManagerImpl					&	resource_manager,
		uint32_t												loader_thread,
		ResourceBase										*	parent_resource
	);

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VK2D_API													~MaterialResource();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the resource has been loaded or is in the process of being loaded.
	///
	///				This function will not wait but returns immediately with the result.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus										GetStatus();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block. As soon
	///				as the resource state becomes determined this function will return and code execution can continue.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined. Default value is std::chrono::nanoseconds::max() which
	///				makes this function wait indefinitely.
	///
	/// @return		Status of the resource, see ResourceStatus. Resource status can only be undetermined if timeout was given.
	VK2D_API ResourceStatus										WaitUntilLoaded(
		std::chrono::nanoseconds								timeout						= std::chrono::nanoseconds::max()
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Waits for the resource to load on the calling thread before continuing execution.
	///
	///				For as long as the resource status is undetermined or timeout hasn't been met this function will block. As soon
	///				as the resource state becomes determined this function will return and code execution can continue.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @param[in]	timeout
	///				Maximum time to wait. If resource is still in undetermined state at timeout it will return anyways and the
	///				result will tell that the resource is still undetermined.
	///
	/// @return		Status of the resource, see ResourceStatus.
	VK2D_API ResourceStatus										WaitUntilLoaded(
		std::chrono::steady_clock::time_point					timeout
	);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Checks if the object is good to be used or if a failure occurred in it's creation.
	///
	/// @note		Multithreading: Any thread.
	///
	/// @return		true if class object was created successfully, false if something went wrong
	VK2D_API bool												IsGood() const;

private:
	std::unique_ptr<vk2d_internal::MaterialResourceImpl>		impl;
};



} // vk2d
