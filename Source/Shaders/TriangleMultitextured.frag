#version 450
#extension GL_KHR_vulkan_glsl : enable

// const uint VERTEX_SIZE_IN_FLOATS = 12;

// Set 1: Index buffer as storage buffer.
layout(std430, set=1, binding=0) readonly buffer	IndexBuffer{
	uint		ssbo[];
} index_buffer;

// Vertex.
struct			Vertex {
	vec2		coords;
	vec2		UVs;
	vec4		color;
	float		point_size;
};
layout(std430, set=2, binding=0) readonly buffer	VertexBuffer {
	Vertex		ssbo[];
} vertex_buffer;

// Set 3: Texture and sampler
layout(set=3, binding=0) uniform sampler2DArray		image_sampler;
layout(std140, set=3, binding=1) uniform			image_sampler_data {
	vec4		border_color;
	vec4		border_color_enable;
} sampler_data;

// Set 4: Texture channel weights
layout(std430, set=4, binding=0) readonly buffer	TextureChannelWeights {
	float		ssbo[];
} texture_channel_weights;

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	uint		index_offset;				// Offset into the index buffer.
	uint		index_count;				// Amount of indices this shader should handle.
	uint		vertex_offset;				// Offset to first vertex in vertex buffer.
	uint		texture_channel_offset;		// Location of the texture channels in the texture channel weights ssbo.
	uint		texture_channel_count;		// Just the amount of texture channels.
} push_constants;

// From vertex shader.
layout(location=0) in vec2 fragment_input_UV;
layout(location=1) in vec4 fragment_input_color;
layout(location=2) in vec2 fragment_input_original_coords;

// Color output.
layout(location=0) out vec4 final_fragment_color;



// Get indices from index buffer for current primitive.
uvec3 GetIndices()
{
	uvec3 indices;
	uint index_ssbo_position	= gl_PrimitiveID * push_constants.index_count + push_constants.index_offset;
	indices						= uvec3(
		index_buffer.ssbo[ index_ssbo_position + 0 ],
		index_buffer.ssbo[ index_ssbo_position + 1 ],
		index_buffer.ssbo[ index_ssbo_position + 2 ]
	);
	return indices;
}

// Get vertices from vertex buffer using indices.
Vertex[ 3 ] GetVertices( uvec3 indices )
{
	Vertex vertices[ 3 ];
	uint vertex_ssbo_position	= push_constants.vertex_offset;
	vertices[ 0 ]				= vertex_buffer.ssbo[ indices[ 0 ] + vertex_ssbo_position ];
	vertices[ 1 ]				= vertex_buffer.ssbo[ indices[ 1 ] + vertex_ssbo_position ];
	vertices[ 2 ]				= vertex_buffer.ssbo[ indices[ 2 ] + vertex_ssbo_position ];
	return vertices;
}

// Calculate fragment weight in relation to each
// vertex, using barycentric coordinates.
vec3 CalculateBarycentricWeights( Vertex[ 3 ] vertices )
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

vec4 CalculateWeightedTextureColor( uvec3 indices, vec3 vertex_weights )
{
	// Loop through texture channels and apply color value as needed.
	vec4 weighted_texture_color = vec4( 0.0 );
	for( uint i = 0; i < push_constants.texture_channel_count; ++i ) {
		vec3 vertex_texture_channel_weights;
		vertex_texture_channel_weights[ 0 ]		= texture_channel_weights.ssbo[ indices[ 0 ] * push_constants.texture_channel_count + i ];
		vertex_texture_channel_weights[ 1 ]		= texture_channel_weights.ssbo[ indices[ 1 ] * push_constants.texture_channel_count + i ];
		vertex_texture_channel_weights[ 2 ]		= texture_channel_weights.ssbo[ indices[ 2 ] * push_constants.texture_channel_count + i ];

		vec3 vertex_combined_channel_weights	= vertex_texture_channel_weights * vertex_weights;
		float total_channel_weight	= vertex_combined_channel_weights[ 0 ] + vertex_combined_channel_weights[ 1 ] + vertex_combined_channel_weights[ 2 ];

		if( total_channel_weight > 0.0 ) {
			vec4 tc = texture( image_sampler, vec3( fragment_input_UV, float( i ) ) );
			weighted_texture_color				+= tc * total_channel_weight;
		}
	}

	return weighted_texture_color;
}



// Entrypoint
void MultitexturedTriangle()
{
	if( sampler_data.border_color_enable.x > 0.5 ) {
		if( fragment_input_UV.x < 0.0 || fragment_input_UV.x > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
			return;
		}
	}
	if( sampler_data.border_color_enable.y > 0.5 ) {
		if( fragment_input_UV.y < 0.0 || fragment_input_UV.y > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
			return;
		}
	}

	uvec3	indices				= GetIndices();
	Vertex	vertices[ 3 ]		= GetVertices( indices );
	vec3	vertex_weights		= CalculateBarycentricWeights( vertices );
	vec4	texture_color		= CalculateWeightedTextureColor( indices, vertex_weights );
	final_fragment_color		= texture_color * fragment_input_color;

//	DEBUG:
//	final_fragment_color		= texture_color;
//	final_fragment_color		= vec4( vec3( vertex_weights ), 1.0 );
//	final_fragment_color		= vec4( 1.0, 1.0, 1.0, 1.0 );
}
