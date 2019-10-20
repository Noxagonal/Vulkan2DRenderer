// Check this

#version 450
#extension GL_KHR_vulkan_glsl : enable

// Vertex
layout(location=0) in vec2 vertex_Coords;
layout(location=1) in vec2 vertex_UVs;
layout(location=2) in vec4 vertex_Color;

// Output to fragment shader
layout(location=0) out vec2 fragment_UV;
layout(location=1) out vec4 fragment_Color;

void main()
{
	fragment_UV		= vertex_UVs;
	fragment_Color	= vertex_Color;
	gl_Position		= vec4( vertex_Coords, 0.0f, 1.0f );
	gl_PointSize	= 1.0f;		// todo: add means to get this info from the renderer to the shader
}
