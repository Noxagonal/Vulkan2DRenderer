#version 450
#extension GL_KHR_vulkan_glsl : enable

// Vertex
layout(location=0) in vec2 vertex_Coords;
layout(location=1) in vec2 vertex_UVs;
layout(location=2) in vec4 vertex_Color;
layout(location=3) in float vertex_PointSize;

// Push constants
layout(push_constant) uniform WindowScaling {
	vec2 multiplier;
	vec2 offset;
} window_scaling;

// Output to fragment shader
layout(location=0) out vec2 fragment_UV;
layout(location=1) out vec4 fragment_Color;

void main()
{
	fragment_UV		= vertex_UVs;
	fragment_Color	= vertex_Color;
	vec2 coords		= vertex_Coords * window_scaling.multiplier + window_scaling.offset;
	gl_Position		= vec4( coords, 1.0f, 1.0f );
	gl_PointSize	= vertex_PointSize;
}
