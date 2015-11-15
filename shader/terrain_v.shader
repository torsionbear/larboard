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

uniform int tileCountInSight;
uniform int tileSize;
uniform ivec2 heightMapOrigin;
uniform ivec2 heightMapSize;
uniform ivec2 diffuseMapOrigin;
uniform ivec2 diffuseMapSize;

uniform Textures textures;

layout(location = 0) in vec2 position;
//layout(location = 1) in vec3 vertNormal;
//layout(location = 2) in vec2 vertTexCoord;

//out vec4 vPosition;
//out vec4 fragNormal;
out vec2 vHeightMapTexCoord;
out vec2 vDiffuseMapTexCoord;

ivec2 GetTileCoord() {
	ivec2 viewPos = ivec2(camera.viewPosition.xy / tileSize); // converting from float to int drops fractional part automatically
	int x = gl_InstanceID % (2 * tileCountInSight);
	int y = gl_InstanceID / (2 * tileCountInSight);	
	return viewPos + ivec2(x, y) - ivec2(tileCountInSight, tileCountInSight);	
}

void main()
{
	ivec2 tileCoord = GetTileCoord();
	vHeightMapTexCoord = (vec2(tileCoord - heightMapOrigin) + position / float(tileSize)) / vec2(heightMapSize);
	vDiffuseMapTexCoord = (vec2(tileCoord - diffuseMapOrigin) + position / float(tileSize)) / vec2(diffuseMapSize);
    gl_Position = vec4(position + tileCoord * tileSize, 0, 1);
	
	//fragNormal = transform.normalTransform * vec4(vertNormal, 0);
}