#version 430 core

layout (triangles, equal_spacing, ccw) in;

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
}; 
uniform Textures textures;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

in vec2 tcHeightMapTexCoord[];
in vec2 tcDiffuseMapTexCoord[];

out vec3 teDiffuseMapTexCoord;

void main() {
	vec2 heightMapTexCoord = tcHeightMapTexCoord[0] * gl_TessCoord.x + tcHeightMapTexCoord[1] * gl_TessCoord.y + tcHeightMapTexCoord[2] * gl_TessCoord.z;
	float height = texture(textures.heightMap, heightMapTexCoord).x * 30 - 5;
	vec2 diffuseMapTexCoord = tcDiffuseMapTexCoord[0] * gl_TessCoord.x + tcDiffuseMapTexCoord[1] * gl_TessCoord.y + tcDiffuseMapTexCoord[2] * gl_TessCoord.z;
	teDiffuseMapTexCoord = vec3(diffuseMapTexCoord, (height + 5) / 15);
	
	vec4 pos = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
	pos.z = height;
	gl_Position = camera.viewProjectTransform * pos;
}