#version 430 core

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
}; 

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

//uniform int tileCountInSight;
uniform float tileSize;
uniform float sightDistance;
uniform ivec2 gridOrigin;
uniform int gridWidth;
uniform ivec2 heightMapOrigin;
uniform ivec2 heightMapSize;
uniform ivec2 diffuseMapOrigin;
uniform ivec2 diffuseMapSize;

uniform Textures textures;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 tileCoord;
//layout(location = 2) in vec2 vertTexCoord;

//out vec4 vPosition;
//out vec4 fragNormal;
//out vec2 vHeightMapTexCoord;
out vec2 vDiffuseMapTexCoord;

//ivec2 GetTileCoord() {
//	int x = gl_InstanceID % gridWidth;
//	int y = gl_InstanceID / gridWidth;
//	return gridOrigin + ivec2(x, y);
//}

void main()
{
	//ivec2 tileCoord = GetTileCoord();
	//vHeightMapTexCoord = (vec2(tileCoord - heightMapOrigin) + position / tileSize) / vec2(heightMapSize);
	vDiffuseMapTexCoord = ((tileCoord.xy + position) / tileSize - diffuseMapOrigin) / vec2(diffuseMapSize);
    gl_Position = vec4(tileCoord.xy + position, tileCoord.z, 1);
	
	//fragNormal = transform.normalTransform * vec4(vertNormal, 0);
}