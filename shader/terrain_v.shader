#version 430 core

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
}; 

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

uniform Textures textures;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 tileCoord;

out vec2 vDiffuseMapTexCoord;

void main()
{
	vDiffuseMapTexCoord = ((tileCoord.xy + position) / terrain.tileSize - terrain.diffuseMapOrigin) / vec2(terrain.diffuseMapSize);
    gl_Position = vec4(tileCoord.xy + position, tileCoord.z, 1);
	
	//fragNormal = transform.normalTransform * vec4(vertNormal, 0);
}