#pragma once

#include "core/Common.h"

#include <initializer_list>



namespace vk2d {



/// @brief		Transformations are used to efficiently control position, scale and rotation of a draw
///				instance. Each time you draw something you have a chance to pass along transformation
///				object that will be applied to the final vertex coordinates by the GPU.
///				Using transformations is more efficient than modifying the mesh each frame as
///				transformations do not calculate new vertex positions on the CPU.
///				However transformations only apply to the entire model, if you need per vertex control
///				you'll still have to modify the vertices on the CPU by using a mesh modifier function
///				or modifying the vertices manually.
///				This class can also help converting these into a transformation matrix that can be
///				used for transformation hierarchy.
class Transform
{
public:
	glm::vec2						position				= {};
	glm::vec2						scale					= { 1.0f, 1.0f };
	float							rotation				= {};

	VK2D_API												Transform()					= default;
	VK2D_API												Transform(
		const vk2d::Transform	&	other )					= default;
	VK2D_API												Transform(
		vk2d::Transform			&&	other )					= default;
	VK2D_API												Transform(
		glm::vec2					position,
		glm::vec2					scale,
		float						rotation );

	VK2D_API void					VK2D_APIENTRY			Translate(
		glm::vec2					movement );
	VK2D_API void					VK2D_APIENTRY			Scale(
		glm::vec2					scale );
	VK2D_API void					VK2D_APIENTRY			Rotate(
		float						rotation );

	/// @brief		Calculate new transformation matrix from position, scale and rotation that can be used
	///				with scene transformation hierarchy. <br> For example if you wish to draw 2 boxes and have
	///				one of the boxes "follow" the transformations of another then you will need a
	///				transformation matrix to do so.
	///				In the code sample below <tt>box</tt> is a simple box mesh, <tt>parent_transform</tt> is
	///				transformations of the parent box, <tt>child_transform</tt> is transformations of the
	///				child box which we wish to move with the parent. <tt>window</tt> is basic window.
	/// @code
	///				auto parent_matrix = parent_transform.CalculateTransformationMatrix();
	///				auto child_matrix = child_transform.CalculateTransformationMatrix();
	///				auto child_follow_parent_matrix = parent_matrix * child_matrix;
	///				window->DrawMesh( box, { parent_matrix } );
	///				window->DrawMesh( box, { child_follow_parent_matrix } );
	/// @endcode
	/// @return		A new 4*4 matrix that combines location, scale and rotation.
	VK2D_API glm::mat4			VK2D_APIENTRY			CalculateTransformationMatrix() const;
};



/// @brief		Create 2*2 rotation matrix.
/// @tparam		T
///				Matrix precision.
/// @param		rotation
///				Rotation in radians.
/// @return		Rotation matrix.
template<typename T>
glm::mat<2, 2, T> CreateRotationMatrix2(
	T rotation )
{
	auto x = T( glm::cos( rotation ) );
	auto y = T( glm::sin( rotation ) );
	// Matrix input has flipped column and row order.
	return glm::mat<2, 2, T>(
		+x, +y,
		-y, +x
	);
}



/// @brief		Create 3*3 rotation matrix.
/// @tparam		T
///				Matrix precision.
/// @param		rotation
///				Rotation in radians.
/// @return		Rotation matrix.
template<typename T>
glm::mat<3, 3, T> CreateRotationMatrix3(
	T rotation )
{
	auto x = T( glm::cos( rotation ) );
	auto y = T( glm::sin( rotation ) );
	// Matrix input has flipped column and row order.
	return glm::mat<3, 3, T>(
		+x, +y, 0,
		-y, +x, 0,
		0,  0,  1
	);
}



/// @brief		Create 4*4 rotation matrix.
/// @tparam		T
///				Matrix precision.
/// @param		rotation
///				Rotation in radians.
/// @return		Rotation matrix.
template<typename T>
glm::mat<4, 4, T> CreateRotationMatrix4(
	T rotation )
{
	auto x = T( glm::cos( rotation ) );
	auto y = T( glm::sin( rotation ) );
	// Matrix input has flipped column and row order.
	return glm::mat<4, 4, T>(
		+x, +y, 0,  0,
		-y, +x, 0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1
	);
}



}
