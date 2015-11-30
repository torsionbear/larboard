#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout(vertices = 3) out;

uniform float tileSize;
uniform float sightDistance;
uniform ivec2 gridOrigin;
uniform int gridWidth;
uniform ivec2 heightMapOrigin;
uniform ivec2 heightMapSize;
uniform ivec2 diffuseMapOrigin;
uniform ivec2 diffuseMapSize;

//in vec3 vPosition[];
//in vec2 vHeightMapTexCoord[];
in vec2 vDiffuseMapTexCoord[];

//out vec4 tcPosition[];
//out vec2 tcHeightMapTexCoord[];
out vec2 tcDiffuseMapTexCoord[];

float CalculateOuterTessLevel(vec4 vertex0, vec4 vertex1) {
	float edgeLength = length(vertex0 - vertex1);
	float distanceFactor = sightDistance / 60;
	float distance0 = length(2 * camera.viewPosition - vertex0 - vertex1) / 2;
	return max(edgeLength * 12 / distance0, 1.0);
}
void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	//tcHeightMapTexCoord[gl_InvocationID] = vHeightMapTexCoord[gl_InvocationID];
	tcDiffuseMapTexCoord[gl_InvocationID] = vDiffuseMapTexCoord[gl_InvocationID];
	
	// see opengl4.5 specification (glspec45.core.pdf page395) for relation between edge index and vertex index
	gl_TessLevelOuter[0] = CalculateOuterTessLevel(gl_in[1].gl_Position, gl_in[2].gl_Position);
	gl_TessLevelOuter[1] = CalculateOuterTessLevel(gl_in[2].gl_Position, gl_in[0].gl_Position);
	gl_TessLevelOuter[2] = CalculateOuterTessLevel(gl_in[0].gl_Position, gl_in[1].gl_Position);
	gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3;
}