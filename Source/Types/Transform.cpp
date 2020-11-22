
#include "../Core/SourceCommon.h"

#include "../../Include/Types/Transform.h"
#include "../../Include/Types/Vector2.hpp"
#include "../../Include/Types/Matrix2.hpp"
#include "../../Include/Types/Matrix3.hpp"


vk2d::Transform::Transform(
	vk2d::Vector2f position,
	vk2d::Vector2f scale,
	float rotation
) :
	position( position ),
	scale( scale ),
	rotation( rotation )
{}

VK2D_API void VK2D_APIENTRY vk2d::Transform::Translate(
	vk2d::Vector2f movement
)
{
	this->position += movement;
}

VK2D_API void VK2D_APIENTRY vk2d::Transform::Scale(
	vk2d::Vector2f scale
)
{
	this->scale *= scale;
}

VK2D_API void VK2D_APIENTRY vk2d::Transform::Rotate(
	float rotation
)
{
	this->rotation += rotation;
}

VK2D_API vk2d::Matrix3f VK2D_APIENTRY vk2d::Transform::CalculateTransformationMatrix()
{
	auto position_matrix_3 = vk2d::Matrix3f( 1.0f );
	{
		position_matrix_3.row_1.z = position.x;
		position_matrix_3.row_2.z = position.y;
	}

	auto scale_matrix_3 = vk2d::Matrix3f( 1.0f );
	{
		scale_matrix_3.row_1.x = scale.x;
		scale_matrix_3.row_2.y = scale.y;
	}

	auto rotation_matrix_3 = vk2d::CreateRotationMatrix3( rotation );

	return position_matrix_3 * scale_matrix_3 * rotation_matrix_3;
}
