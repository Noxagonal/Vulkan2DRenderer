#pragma once

#include <core/SourceCommon.hpp>

#include <interface/cursor/Cursor.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace vk2d {
namespace vk2d_internal {



class CursorImpl
{
	friend class WindowImpl;

public:
	CursorImpl(
		const CursorImpl					&	other
	) = delete;

	CursorImpl(
		CursorImpl							&&	other
	) = default;

	CursorImpl(
		InstanceImpl						&	instance,
		const std::filesystem::path			&	image_path,
		glm::ivec2								hot_spot
	);

	CursorImpl(
		InstanceImpl						&	instance,
		glm::uvec2								image_size,
		const std::vector<Color8>			&	image_data,
		glm::ivec2								hot_spot
	);

	~CursorImpl();

	CursorImpl								&	operator=(
		const CursorImpl					&	other
		) = delete;

	CursorImpl								&	operator=(
		CursorImpl							&&	other
		) = default;

	bool										IsGood();

	InstanceImpl							&	GetInstance();
	const std::vector<Color8>				&	GetTexelData();
	GLFWcursor								*	GetGLFWcursor();
	glm::uvec2									GetSize();
	glm::ivec2									GetHotSpot();

private:
	InstanceImpl							&	instance;
	std::vector<Color8>							pixel_data = {};
	GLFWcursor								*	cursor = nullptr;
	VkExtent2D									extent = {};
	VkOffset2D									hotSpot = {};

	bool										is_good = {};
};



} // vk2d_internal
} // vk2d
