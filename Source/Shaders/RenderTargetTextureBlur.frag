#version 450
#extension GL_KHR_vulkan_glsl : enable



// !!! WARNING !!!
// Please note that NVIDIA algorithm is currently broken, the brightness
// of the image varies with different sigma values, it's representitive
// enough for perfromance tests but useless otherwise.

#define GBA_NAIVE						1
#define GBA_NVIDIA						2
#define GBA_DOUBLE_TEXEL_SINGLE_FETCH	3
#define GBA_PRECOMP_COMBINED			4

// GBA_PRECOMP_COMBINED is the fastest while maintaining perfect accuracy.
#define GAUSSIAN_BLUR_ALGORITHM			GBA_PRECOMP_COMBINED



////////////////////////////////////////////////////////////////
// Shader program interface.
////////////////////////////////////////////////////////////////

// Set 0: Sampler
layout(set=0, binding=0) uniform sampler			image_sampler;

// Set 1: Texture
layout(set=1, binding=0) uniform texture2D			sampled_image;

// Push constants.
layout(std140, push_constant) uniform PushConstants {
	vec4		blur_info;				// [ 0 ] = sigma, [ 1 ] = precomputed normalizer, [ 2 ] = initial coefficient, [ 3 ] = initial natural exponentation.
	vec2		pixel_size;				// Pixel size on a canvas considered ranging from 0 to 1.
} push_constants;

// From vertex shader.
layout(location=0) in		vec2	fragment_input_coords;
layout(location=1) in		vec2	fragment_input_UV;

// Color output.
layout(location=0) out vec4 final_fragment_color;



#define M_PI 3.1415926535897932384626433832795



////////////////////////////////////////////////////////////////
// Entrypoints.
////////////////////////////////////////////////////////////////

void RenderTargetTexture_BoxBlur_Horisontal()
{
	vec2	pixel_step	= push_constants.pixel_size;
	int		range		= int( push_constants.blur_info[ 0 ] * 3.0 );
	vec4	acc			= vec4( 0.0 );
	float	norm		= 0.0;
	for( int i = 1 - range; i < range; ++i ) {
		vec2 uv = fragment_input_UV;
		uv.x += pixel_step.x * i;
		acc += texture( sampler2D( sampled_image, image_sampler ), uv );
		norm += 1.0;
	}
	
	acc /= norm;
	final_fragment_color	= acc;
}



void RenderTargetTexture_BoxBlur_Vertical()
{
	vec2	pixel_step	= push_constants.pixel_size;
	int		range		= int( push_constants.blur_info[ 0 ] * 3.0 );
	vec4	acc			= vec4( 0.0 );
	float	norm		= 0.0;
	for( int i = 1 - range; i < range; ++i ) {
		vec2 uv = fragment_input_UV;
		uv.y += pixel_step.y * i;
		acc += texture( sampler2D( sampled_image, image_sampler ), uv );
		norm += 1.0;
	}

	acc /= norm;
	final_fragment_color	= acc;
}







#if( GAUSSIAN_BLUR_ALGORITHM == GBA_NAIVE )

void RenderTargetTexture_GaussianBlur_Horisontal()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( push_constants.pixel_size.x, 0.0 );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;

	for (int i = -support; i <= support; i++) {
		float coeff = exp(-0.5 * float(i) * float(i) / (sigma * sigma));
		acc += texture( sampler2D( sampled_image, image_sampler ), loc + float( i ) * dir ) * coeff;
		norm += coeff;
	}
	acc *= 1.0/norm;                               // normalize for unity gain

    // Output to screen
    final_fragment_color = acc;
}

void RenderTargetTexture_GaussianBlur_Vertical()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( 0.0, push_constants.pixel_size.y );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;

	for (int i = -support; i <= support; i++) {
		float coeff = exp(-0.5 * float(i) * float(i) / (sigma * sigma));
		acc += texture( sampler2D( sampled_image, image_sampler ), loc + float( i ) * dir ) * coeff;
		norm += coeff;
	}
	acc *= 1.0/norm;                               // normalize for unity gain

    // Output to screen
    final_fragment_color = acc;
}

#endif







#if( GAUSSIAN_BLUR_ALGORITHM == GBA_NVIDIA )

void RenderTargetTexture_GaussianBlur_Horisontal()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( push_constants.pixel_size.x, 0.0 );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;
	
	float		delta			= 1.0;
	vec3 g;
	g.x		= 1.0 / ( sqrt( 2.0 * M_PI ) * sigma );		// g.x is the coefficient
	g.y		= exp( -0.5 * delta * delta / ( sigma * sigma ) );
	g.z		= g.y * g.y;

	texture( sampler2D( sampled_image, image_sampler ), loc ) * g.x;
	norm += g.x;

	for (int i = 1; i <= support; i++) {
		g.xy *= g.yz;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc + float( i ) * dir ) * g.x;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - float( i ) * dir ) * g.x;
		norm += g.x * 2.0;
	}
	acc *= 1.0/norm;                               // normalize for unity gain

    // Output to screen
    final_fragment_color = acc;
}

void RenderTargetTexture_GaussianBlur_Vertical()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( 0.0, push_constants.pixel_size.y );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;

	float		delta			= 1.0;
	vec3 g;
	g.x		= 1.0 / ( sqrt( 2.0 * M_PI ) * sigma );		// g.x is the coefficient
	g.y		= exp( -0.5 * delta * delta / ( sigma * sigma ) );
	g.z		= g.y * g.y;

	acc += texture( sampler2D( sampled_image, image_sampler ), loc ) * g.x;
	norm += g.x;

	for (int i = 1; i <= support; i++) {
		g.xy *= g.yz;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc + float( i ) * dir ) * g.x;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - float( i ) * dir ) * g.x;
		norm += g.x * 2.0;
	}
	acc *= 1.0/norm;                               // normalize for unity gain

    // Output to screen
    final_fragment_color = acc;
}

#endif







#if( GAUSSIAN_BLUR_ALGORITHM == GBA_DOUBLE_TEXEL_SINGLE_FETCH )

void RenderTargetTexture_GaussianBlur_Horisontal()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( push_constants.pixel_size.x, 0.0 );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;
	float		delta			= 1.0;

	float coeff = exp( -0.5 * delta * delta / ( sigma * sigma ) );
	acc		+= texture( sampler2D( sampled_image, image_sampler ), loc) * coeff;
	norm	+= coeff;

	for( int i = 1; i <= support; i += 2 ) {

		vec2 t_offsets = vec2( i, i + 1 );

		// Actual implementation didn't calculate coefficients on the fly like this but fetched them from a buffer memory.
		// Performance of this is expected to be better than fetching from memory.
		vec2 t_weights;
		t_weights[ 0 ] = exp( -0.5 * t_offsets[ 0 ] * t_offsets[ 0 ] / ( sigma * sigma ) );
		t_weights[ 1 ] = exp( -0.5 * t_offsets[ 1 ] * t_offsets[ 1 ] / ( sigma * sigma ) );

		float tf_weight	= t_weights[ 0 ] + t_weights[ 1 ];
		float tf_offset	= ( t_offsets[ 0 ] * t_weights[ 0 ] + t_offsets[ 1 ] * t_weights[ 1 ] ) / tf_weight;
		//float tf_offset		= ( t_offsets[ 0 ] + t_offsets[ 1 ] ) * 0.5f;	// Inaccurate, but used to compare performance.

		acc += texture( sampler2D( sampled_image, image_sampler ), loc + tf_offset * dir ) * tf_weight;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - tf_offset * dir ) * tf_weight;
		norm += tf_weight * 2.0;
	}

	acc *= 1.0 / norm;

	final_fragment_color = acc;
}

void RenderTargetTexture_GaussianBlur_Vertical()
{
	float		sigma			= push_constants.blur_info[ 0 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( 0.0, push_constants.pixel_size.y );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator
	float		norm			= 0.0;
	float		delta			= 1.0;

	float coeff = exp( -0.5 * delta * delta / ( sigma * sigma ) );
	acc		+= texture( sampler2D( sampled_image, image_sampler ), loc) * coeff;
	norm	+= coeff;

	for( int i = 1; i <= support; i += 2 ) {

		vec2 t_offsets = vec2( i, i + 1 );

		// Actual implementation didn't calculate coefficients on the fly like this but fetched them from a buffer memory.
		// Performance of this is expected to be better than fetching from memory.
		vec2 t_weights;
		t_weights[ 0 ] = exp( -0.5 * t_offsets[ 0 ] * t_offsets[ 0 ] / ( sigma * sigma ) );
		t_weights[ 1 ] = exp( -0.5 * t_offsets[ 1 ] * t_offsets[ 1 ] / ( sigma * sigma ) );

		float tf_weight	= t_weights[ 0 ] + t_weights[ 1 ];
		float tf_offset	= ( t_offsets[ 0 ] * t_weights[ 0 ] + t_offsets[ 1 ] * t_weights[ 1 ] ) / tf_weight;
		//float tf_offset		= ( t_offsets[ 0 ] + t_offsets[ 1 ] ) * 0.5f;	// Inaccurate, but used to compare performance.

		acc += texture( sampler2D( sampled_image, image_sampler ), loc + tf_offset * dir ) * tf_weight;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - tf_offset * dir ) * tf_weight;
		norm += tf_weight * 2.0;
	}

	acc *= 1.0 / norm;

	final_fragment_color = acc;
}

#endif







#if( GAUSSIAN_BLUR_ALGORITHM == GBA_PRECOMP_COMBINED )

void RenderTargetTexture_GaussianBlur_Horisontal()
{
	// Fast gaussian pass using Nvidia's GPU Gems gaussian calculations and
	// Rastergrid's efficient texel fetch using linear texture sampling.
	// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch40.html
	// See: http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

	float		sigma			= push_constants.blur_info[ 0 ];
	float		precomp_norm	= push_constants.blur_info[ 1 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( push_constants.pixel_size.x, 0.0 );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator

	vec3 g;
	g.x		= push_constants.blur_info[ 2 ];		// g.x is the coefficient
	g.y		= push_constants.blur_info[ 3 ];
	g.z		= g.y * g.y;

	acc		+= texture( sampler2D( sampled_image, image_sampler ), loc ) * g.x * precomp_norm;

	for( int i = 1; i <= support; i += 2 ) {
	
		vec2 t_weights;
		g.xy			*= g.yz;
		t_weights[ 0 ]	= g.x * precomp_norm;
		g.xy			*= g.yz;
		t_weights[ 1 ]	= g.x * precomp_norm;

		vec2 t_offsets	= vec2( float( i ), float( i + 1 ) );

		float tf_weight	= t_weights[ 0 ] + t_weights[ 1 ];
		float tf_offset	= ( t_offsets[ 0 ] * t_weights[ 0 ] + t_offsets[ 1 ] * t_weights[ 1 ] ) / tf_weight;

		acc += texture( sampler2D( sampled_image, image_sampler ), loc + tf_offset * dir ) * tf_weight;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - tf_offset * dir ) * tf_weight;
	}

	final_fragment_color = acc;
}

void RenderTargetTexture_GaussianBlur_Vertical()
{
	// Fast gaussian pass using Nvidia's GPU Gems gaussian calculations and
	// Rastergrid's efficient texel fetch using linear texture sampling.
	// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch40.html
	// See: http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

	float		sigma			= push_constants.blur_info[ 0 ];
	float		precomp_norm	= push_constants.blur_info[ 1 ];
	int			support			= int( sigma * 3.0 );							// int(sigma * 3.0) truncation
	vec2		loc				= gl_FragCoord.xy * push_constants.pixel_size;	// center pixel coordinate
	vec2		dir				= vec2( 0.0, push_constants.pixel_size.y );		// horiz=(1.0, 0.0), vert=(0.0, 1.0)
	vec4		acc				= vec4( 0.0 );									// accumulator

	vec3 g;
	g.x		= push_constants.blur_info[ 2 ];		// g.x is the coefficient
	g.y		= push_constants.blur_info[ 3 ];
	g.z		= g.y * g.y;

	acc		+= texture( sampler2D( sampled_image, image_sampler ), loc ) * g.x * precomp_norm;

	for( int i = 1; i <= support; i += 2 ) {

		vec2 t_weights;
		g.xy			*= g.yz;
		t_weights[ 0 ]	= g.x * precomp_norm;
		g.xy			*= g.yz;
		t_weights[ 1 ]	= g.x * precomp_norm;

		vec2 t_offsets	= vec2( float( i ), float( i + 1 ) );

		float tf_weight	= t_weights[ 0 ] + t_weights[ 1 ];
		float tf_offset	= ( t_offsets[ 0 ] * t_weights[ 0 ] + t_offsets[ 1 ] * t_weights[ 1 ] ) / tf_weight;

		acc += texture( sampler2D( sampled_image, image_sampler ), loc + tf_offset * dir ) * tf_weight;
		acc += texture( sampler2D( sampled_image, image_sampler ), loc - tf_offset * dir ) * tf_weight;
	}

	final_fragment_color = acc;
}

#endif
