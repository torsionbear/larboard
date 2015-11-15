#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout(vertices = 3) out;

uniform int tileCountInSight;
uniform int tileSize;
uniform ivec2 mapOrigin;
uniform ivec2 mapSize; 

//in vec3 vPosition[];
in vec2 vHeightMapTexCoord[];
in vec2 vDiffuseMapTexCoord[];

//out vec4 tcPosition[];
out vec2 tcHeightMapTexCoord[];
out vec2 tcDiffuseMapTexCoord[];

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcHeightMapTexCoord[gl_InvocationID] = vHeightMapTexCoord[gl_InvocationID];
	tcDiffuseMapTexCoord[gl_InvocationID] = vDiffuseMapTexCoord[gl_InvocationID];
	
	gl_TessLevelInner[0] = 3;
	gl_TessLevelOuter[0] = 3;
	gl_TessLevelOuter[1] = 3;
	gl_TessLevelOuter[2] = 3;
}