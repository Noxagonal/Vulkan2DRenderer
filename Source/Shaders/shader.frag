#version 450
#extension GL_KHR_vulkan_glsl : enable

// const uint VERTEX_SIZE_IN_FLOATS = 12;

// Set 1: Index buffer as storage buffer.
layout(set=1, binding=0) readonly buffer			IndexBuffer{
	uint	ssbo[];
} index_buffer;

// Set 2: Texture and sampler
layout(set=2, binding=0) uniform sampler2DArray		image_sampler;
layout(set=2, binding=1) uniform					image_sampler_data {
	vec4	border_color;
	vec4	border_color_enable;
} sampler_data;

// Set 3: Texture channel weights
layout(set=3, binding=0) readonly buffer			TextureChannelWeights {
	float ssbo[];
} texture_channel_weights;

// Push constants.
layout(push_constant) uniform PushConstants {
	uint index_offset;				// Offset into the index buffer.
	uint index_count;				// Amount of indices this shader should handle.
	uint texture_channel_offset;	// Location of the texture channels in the texture channel weights ssbo.
	uint texture_channel_count;		// Just the amount of texture channels.
} push_constants;

// From vertex shader.
layout(location=0) in vec2 fragment_input_UV;
layout(location=1) in vec4 fragment_input_color;

// Color output.
layout(location=0) out vec4 final_fragment_color;

void main()
{
//	uint total_vertex_size						= VERTEX_SIZE_IN_FLOATS + push_constants.texture_channel_size;
//	
//	vec4 fragment_combined_texture_color		= vec4( 1.0, 1.0, 1.0, 1.0 );
//
//	if( push_constants.texture_channel_count > 0 ) {
//
//		uvec3 vertex_position_in_ssbo;
//		{
//			uint index_group_offset				= gl_PrimitiveID * 3 + push_constants.index_offset;
//
//			uint index_value_0					= floatBitsToUint( vertex_data.ssbo[ index_group_offset + 0 ] );
//			uint index_value_1					= floatBitsToUint( vertex_data.ssbo[ index_group_offset + 1 ] );
//			uint index_value_2					= floatBitsToUint( vertex_data.ssbo[ index_group_offset + 2 ] );
//			
//			vertex_position_in_ssbo[ 0 ]		= index_value_0 * total_vertex_size + push_constants.vertex_offset;
//			vertex_position_in_ssbo[ 1 ]		= index_value_1 * total_vertex_size + push_constants.vertex_offset;
//			vertex_position_in_ssbo[ 2 ]		= index_value_2 * total_vertex_size + push_constants.vertex_offset;
//		}
//
//		// Figure out the 3 vertices distance weight making up this fragment.
//		vec3 vertex_distance_weight				= vec3( 1.0, 0.0, 0.0 );
//		{
//			// Calculate relative coordinates to the fragment
//			vec2 vertex_0_coords				= vec2(
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 0 ] + 0 ],
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 0 ] + 1 ]
//			) - gl_FragCoord.xy;
//			
//			vec2 vertex_1_coords				= vec2(
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 1 ] + 0 ],
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 1 ] + 1 ]
//			) - gl_FragCoord.xy;
//
//			vec2 vertex_2_coords				= vec2(
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 2 ] + 0 ],
//				vertex_data.ssbo[ vertex_position_in_ssbo[ 2 ] + 1 ]
//			) - gl_FragCoord.xy;
//			
//			float vertex_0_distance				= vertex_0_coords.x * vertex_0_coords.x + vertex_0_coords.y * vertex_0_coords.y;
//			float vertex_1_distance				= vertex_1_coords.x * vertex_1_coords.x + vertex_1_coords.y * vertex_1_coords.y;
//			float vertex_2_distance				= vertex_2_coords.x * vertex_2_coords.x + vertex_2_coords.y * vertex_2_coords.y;
//			float max_distance					= max( vertex_0_distance, max( vertex_1_distance, vertex_2_distance ) );
//
//			if( max_distance != 0.0 ) {
//				vertex_distance_weight[ 0 ]		= vertex_0_distance / max_distance;
//				vertex_distance_weight[ 1 ]		= vertex_1_distance / max_distance;
//				vertex_distance_weight[ 2 ]		= vertex_2_distance / max_distance;
//			}
//		}
//
//		uvec3 texture_channel_begin_position_in_ssbo	= vertex_position_in_ssbo + VERTEX_SIZE_IN_FLOATS;
//		fragment_combined_texture_color					= vec4( 0.0 );
//
//		for( int i = 0; i < push_constants.texture_channel_count; ++i ) {
//			float texture_channel_weight		=
//				vertex_distance_weight[ 0 ] * vertex_data.ssbo[ texture_channel_begin_position_in_ssbo[ 0 ] + i ] +
//				vertex_distance_weight[ 1 ] * vertex_data.ssbo[ texture_channel_begin_position_in_ssbo[ 1 ] + i ] +
//				vertex_distance_weight[ 2 ] * vertex_data.ssbo[ texture_channel_begin_position_in_ssbo[ 2 ] + i ];
//			if( texture_channel_weight > 0.0 ) {
//				float fi	= float( i );
//				fragment_combined_texture_color += texture( imageSampler, vec3( fragment_UV, fi ) ) * texture_channel_weight;
//			}
//		}
//	}

	uvec3 index_ssbo_values;
	{
		uint index_ssbo_position	= gl_PrimitiveID * push_constants.index_count + push_constants.index_offset;
		index_ssbo_values			= uvec3(
			index_buffer.ssbo[ index_ssbo_position + 0 ],
			index_buffer.ssbo[ index_ssbo_position + 1 ],
			index_buffer.ssbo[ index_ssbo_position + 2 ]
		);
	}

	vec2 texture_channel_values	= vec2(
		texture_channel_weights.ssbo[ index_ssbo_values[ 0 ] * push_constants.texture_channel_count + 0 ],
		texture_channel_weights.ssbo[ index_ssbo_values[ 0 ] * push_constants.texture_channel_count + 1 ]
	);

	vec4 fragment_color			= texture( image_sampler, vec3( fragment_input_UV, 0.0 ) ) * fragment_input_color;

	if( sampler_data.border_color_enable.x > 0.5 ) {
		if( fragment_input_UV.x < 0.0 || fragment_input_UV.x > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
		} else {
			final_fragment_color	= fragment_color;
		}
	}
	if( sampler_data.border_color_enable.y > 0.5 ) {
		if( fragment_input_UV.y < 0.0 || fragment_input_UV.y > 1.0 ) {
			final_fragment_color	= sampler_data.border_color;
		} else {
			final_fragment_color	= fragment_color;
		}
	}

//	DEBUG:
	final_fragment_color		= vec4( texture_channel_values, 1.0, 1.0 );
//	final_fragment_color		= vec4( vec3( index_ssbo_values ) / 100.0, 1.0 );
//	final_fragment_color		= texture( image_sampler, vec3( fragment_input_UV, 0.0 ) );
//	final_fragment_color		= fragment_input_color;
//	final_fragment_color		= vec4( 1.0, 1.0, 1.0, 1.0 );
}
