#pragma once

#include "../Core/Common.h"

#include "Vector2.hpp"
#include "Matrix3.hpp"

#include <initializer_list>



namespace vk2d {



class Transform
{
public:
	vk2d::Vector2f		position					= {};
	vk2d::Vector2f		scale						= { 1.0f, 1.0f };
	float				rotation					= {};

	VK2D_API										Transform()								= default;
	VK2D_API										Transform(
		const Transform		&	other )				= default;
	VK2D_API										Transform(
		Transform			&&	other )				= default;
	VK2D_API										Transform(
		vk2d::Vector2f			position,
		vk2d::Vector2f			scale,
		float					rotation );

	VK2D_API void				VK2D_APIENTRY		Translate(
		vk2d::Vector2f			movement );
	VK2D_API void				VK2D_APIENTRY		Scale(
		vk2d::Vector2f			scale );
	VK2D_API void				VK2D_APIENTRY		Rotate(
		float					rotation );

	VK2D_API vk2d::Matrix3f		VK2D_APIENTRY		CalculateTransformationMatrix();
};



}
