#version 450
#extension GL_KHR_vulkan_glsl : enable



// Defines position where user main function will be imported.
#define USER_SHADER_FRAG_MAIN_CALL



// Vertex.
struct Vertex {
	vec2		coords;
	vec2		UVs;
	vec4		color;
	float		point_size;
	uint		single_texture_channel;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader program interface.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Set 2: Index buffer as storage buffer.
layout(std430, set=2, binding=0) readonly buffer	IndexBuffer{
	uint		ssbo[];
} index_buffer;

// Set 3: Vertex buffer.
layout(std430, set=3, binding=0) readonly buffer	VertexBuffer {
	Vertex		ssbo[];
} vertex_buffer;

// Set 4: Sampler
layout(set=4, binding=0) uniform sampler			image_sampler;
layout(std140, set=4, binding=1) uniform			image_sampler_data {
	vec4		border_color;
	uvec2		border_color_enable;
} sampler_data;

// Set 5: Texture
layout(set=5, binding=0) uniform texture2DArray		sampled_image;

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	uint		transformation_offset;			// Offset into the transformation buffer.
	uint		index_offset;					// Offset into the index buffer.
	uint		index_count;					// Amount of indices this shader should handle.
	uint		vertex_offset;					// Offset to first vertex in vertex buffer.
} push_constants;

// From vertex shader.
layout(location=0) in		vec2	fragment_input_UV;
layout(location=1) in		vec4	fragment_input_color;
layout(location=2) in		vec2	fragment_input_original_coords;
layout(location=3) in flat	uint	fragment_input_vertex_index;

// Color output.
layout(location=0) out vec4 final_fragment_color;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Supporting functions.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get triangle indices.
//
// This is used to get indices of the polygon current fragment is inside of.
uvec3 GetTriangleIndices()
{
	uint index_ssbo_position	= gl_PrimitiveID * push_constants.index_count + push_constants.index_offset;
	uvec3 indices				= uvec3(
		index_buffer.ssbo[ index_ssbo_position + 0 ],
		index_buffer.ssbo[ index_ssbo_position + 1 ],
		index_buffer.ssbo[ index_ssbo_position + 2 ]
	);
	return indices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get line indices.
//
// This is used to get indices of the line current fragment is inbetween of.
uvec2 GetLineIndices()
{
	uint index_ssbo_position	= gl_PrimitiveID * push_constants.index_count + push_constants.index_offset;
	uvec2 indices				= uvec2(
		index_buffer.ssbo[ index_ssbo_position + 0 ],
		index_buffer.ssbo[ index_ssbo_position + 1 ]
	);
	return indices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get the actual vertices from triangle indices.
Vertex[ 3 ] GetTriangleVertices( uvec3 indices )
{
	Vertex vertices[ 3 ];
	uint vertex_ssbo_position	= push_constants.vertex_offset;
	vertices[ 0 ]				= vertex_buffer.ssbo[ indices[ 0 ] + vertex_ssbo_position ];
	vertices[ 1 ]				= vertex_buffer.ssbo[ indices[ 1 ] + vertex_ssbo_position ];
	vertices[ 2 ]				= vertex_buffer.ssbo[ indices[ 2 ] + vertex_ssbo_position ];
	return vertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get the actual vertices from line indices.
Vertex[ 2 ] GetLineVertices( uvec2 indices )
{
	Vertex vertices[ 2 ];
	uint vertex_ssbo_position	= push_constants.vertex_offset;
	vertices[ 0 ]				= vertex_buffer.ssbo[ indices[ 0 ] + vertex_ssbo_position ];
	vertices[ 1 ]				= vertex_buffer.ssbo[ indices[ 1 ] + vertex_ssbo_position ];
	return vertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate fragment weight in relation to each vertex, using barycentric coordinates.
//
// This is used to calculate current fragment's relative distance to 3 vertices of 
vec3 CalculateTriangleWeights( Vertex[ 3 ] vertices )
{
	vec2 v[ 3 ];
	v[ 0 ]		= vertices[ 0 ].coords;
	v[ 1 ]		= vertices[ 1 ].coords;
	v[ 2 ]		= vertices[ 2 ].coords;

	vec2 p		= fragment_input_original_coords;

	float t		= ( v[ 1 ].y - v[ 2 ].y ) * ( v[ 0 ].x - v[ 2 ].x ) + ( v[ 2 ].x - v[ 1 ].x ) * ( v[ 0 ].y - v[ 2 ].y );
	float e1	= ( v[ 1 ].y - v[ 2 ].y ) * ( p.x - v[ 2 ].x ) + ( v[ 2 ].x - v[ 1 ].x ) * ( p.y - v[ 2 ].y );
	float e2	= ( v[ 2 ].y - v[ 0 ].y ) * ( p.x - v[ 2 ].x ) + ( v[ 0 ].x - v[ 2 ].x ) * ( p.y - v[ 2 ].y );
	
	vec3 vertex_weights;

	vertex_weights[ 0 ]		= e1 / t;
	vertex_weights[ 1 ]		= e2 / t;
	vertex_weights[ 2 ]		= 1.0 - vertex_weights[ 0 ] - vertex_weights[ 1 ];

	return vertex_weights;
}

// Calculate fragment weights in relation to each
// vertex, using inverse linear interpolation.
vec2 CalculateLineWeights( Vertex[ 2 ] vertices )
{
	vec2 v[ 2 ];
	v[ 0 ]		= vertices[ 0 ].coords;
	v[ 1 ]		= vertices[ 1 ].coords;

	vec2 p		= fragment_input_original_coords;

//	float dist				= distance( v[ 0 ], v[ 1 ] );
	vec2 vz					= v[ 0 ] - v[ 1 ];
	float dist				= vz.x * vz.x + vz.y * vz.y;

	if( dist > 0.0 ) {
//		dist_p[ 0 ]			= distance( v[ 0 ], p );
		vec2 pz				= v[ 0 ] - p;
		float dist_p		= pz.x * pz.x + pz.y * pz.y;
		dist_p				= dist_p / dist;

		return vec2( dist_p, 1.0 - dist_p );
	}
	return vec2( 1.0, 0.0 );
}

