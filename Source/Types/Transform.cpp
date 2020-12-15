
#include "Core/SourceCommon.h"

#include "Types/Transform.h"
#include "Types/Vector2.hpp"
#include "Types/Matrix4.hpp"


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

VK2D_API vk2d::Matrix4f VK2D_APIENTRY vk2d::Transform::CalculateTransformationMatrix() const
{
	auto position_matrix = vk2d::Matrix4f( 1.0f );
	{
		position_matrix.column_4.x = position.x;
		position_matrix.column_4.y = position.y;
	}

	auto rotation_matrix = vk2d::CreateRotationMatrix4( rotation );

	auto scale_matrix = vk2d::Matrix4f( 1.0f );
	{
		scale_matrix.column_1.x = scale.x;
		scale_matrix.column_2.y = scale.y;
	}

	return position_matrix * rotation_matrix * scale_matrix;
}
