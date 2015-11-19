#version 430 core

layout (triangles, equal_spacing, ccw) in;

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
}; 

uniform float tileSize;
uniform float sightDistance;
uniform ivec2 gridOrigin;
uniform int gridWidth;
uniform ivec2 heightMapOrigin;
uniform ivec2 heightMapSize;
uniform ivec2 diffuseMapOrigin;
uniform ivec2 diffuseMapSize;

uniform Textures textures;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

//in vec2 tcHeightMapTexCoord[];
in vec2 tcDiffuseMapTexCoord[];

out vec3 teDiffuseMapTexCoord;
//out vec4 normal;

float Height(vec4 position) {
	vec2 heightMapTexCoord = (position.xy / tileSize - heightMapOrigin)  / heightMapSize;
	return texture(textures.heightMap, heightMapTexCoord).x * 30 - 0.6;
}

vec2 DiffuseMapTexCoord(ivec2 tileCoord, vec4 position) {
	return (vec2(tileCoord - diffuseMapOrigin) + position.xy / tileSize) / vec2(diffuseMapSize);
}

void main() {
	vec4 position = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
	
	//vec2 heightMapTexCoord = tcHeightMapTexCoord[0] * gl_TessCoord.x + tcHeightMapTexCoord[1] * gl_TessCoord.y + tcHeightMapTexCoord[2] * gl_TessCoord.z;
	float height = Height(position);
	vec2 diffuseMapTexCoord = tcDiffuseMapTexCoord[0] * gl_TessCoord.x + tcDiffuseMapTexCoord[1] * gl_TessCoord.y + tcDiffuseMapTexCoord[2] * gl_TessCoord.z;
	teDiffuseMapTexCoord = vec3(diffuseMapTexCoord, (height + 10) / 15);
	
	position.z = height;
	gl_Position = camera.viewProjectTransform * position ;
}