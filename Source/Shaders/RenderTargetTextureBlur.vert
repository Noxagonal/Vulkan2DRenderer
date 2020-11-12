#version 450
#extension GL_KHR_vulkan_glsl : enable



////////////////////////////////////////////////////////////////
// Shader program interface.
////////////////////////////////////////////////////////////////

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	vec4		blur_info;				// [ 0 ] = sigma, [ 1 ] = precomputed normalizer, [ 2 ] = initial coefficient, [ 3 ] = initial natural exponentation.
	vec2		pixel_size;				// Pixel size on a canvas considered ranging from 0 to 1.
} push_constants;

// Output to fragment shader
layout(location=0) out		vec2	fragment_output_coords;
layout(location=1) out		vec2	fragment_output_UV;



////////////////////////////////////////////////////////////////
// Entrypoints.
////////////////////////////////////////////////////////////////

void RenderTargetTextureBlurVertex()
{
	vec2 coords	= vec2( -1.0, -1.0 );
	vec2 uvs	= vec2( 0.0, 0.0 );

	if( gl_VertexIndex == 0 ) {
		coords	= vec2( -1.0, -1.0 );
		uvs		= vec2( 0.0, 0.0 );
	}
	else if( gl_VertexIndex == 1 ) {
		coords	= vec2( -1.0, 3.0 );
		uvs		= vec2( 0.0, 2.0 );
	}
	else if( gl_VertexIndex == 2 ) {
		coords	= vec2( 3.0, -1.0 );
		uvs		= vec2( 2.0, 0.0 );
	}

	fragment_output_UV		= uvs;
	fragment_output_coords	= coords;
	gl_Position				= vec4( coords, 0.0, 1.0 );
}
