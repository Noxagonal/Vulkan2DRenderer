
#include "core/SourceCommon.h"

#include "types/Transform.h"


vk2d::Transform::Transform(
	glm::vec2 position,
	glm::vec2 scale,
	float rotation
) :
	position( position ),
	scale( scale ),
	rotation( rotation )
{}

VK2D_API void VK2D_APIENTRY vk2d::Transform::Translate(
	glm::vec2 movement
)
{
	this->position += movement;
}

VK2D_API void VK2D_APIENTRY vk2d::Transform::Scale(
	glm::vec2 scale
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

VK2D_API glm::mat4 VK2D_APIENTRY vk2d::Transform::CalculateTransformationMatrix() const
{
	auto position_matrix = glm::mat4( 1.0f );
	{
		position_matrix[ 3 ][ 0 ] = position.x;
		position_matrix[ 3 ][ 1 ] = position.y;
	}

	auto rotation_matrix = CreateRotationMatrix4( rotation );

	auto scale_matrix = glm::mat4( 1.0f );
	{
		scale_matrix[ 0 ][ 0 ] = scale.x;
		scale_matrix[ 1 ][ 1 ] = scale.y;
	}

	return position_matrix * rotation_matrix * scale_matrix;
}
