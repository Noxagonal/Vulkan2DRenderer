#pragma once

#include "SourceCommon.h"

class MeshBuffer {
public:
							MeshBuffer();
							~MeshBuffer();

private:
	void					ResizeHostBuffer(
		VkDeviceSize		new_vertex_size,
		VkDeviceSize		new_index_size );

	void					ResizeDeviceBuffer(
		VkDeviceSize		new_vertex_size,
		VkDeviceSize		new_index_size );
};
