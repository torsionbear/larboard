#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

layout (std140, binding = 4) uniform Terrain {	
	ivec2 heightMapOrigin;
	ivec2 heightMapSize;
	ivec2 diffuseMapOrigin;
	ivec2 diffuseMapSize;
	float tileSize;
	float sightDistance;	
} terrain;

layout(vertices = 3) out;

in vec2 vDiffuseMapTexCoord[];
out vec2 tcDiffuseMapTexCoord[];

float CalculateOuterTessLevel(vec4 vertex0, vec4 vertex1) {
	float edgeLength = length(vertex0 - vertex1);
	float distanceFactor = terrain.sightDistance / 60;
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