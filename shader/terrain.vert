#version 430 core

struct Textures {
	sampler2D diffuseMap;
	sampler2D heightMap;
}; 

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

uniform int tileCountInSight;
uniform int tileSize;
uniform ivec2 mapOrigin;
uniform ivec2 mapSize; 

uniform Textures textures;

layout(location = 0) in vec2 vertPosition;
//layout(location = 1) in vec3 vertNormal;
//layout(location = 2) in vec2 vertTexCoord;

out vec4 fragPosition;
//out vec4 fragNormal;
out vec2 fragTexCoord;

ivec2 GetTileCoord() {
	ivec2 viewPos = ivec2(camera.viewPosition.xy / tileSize); // converting from float to int drops fractional part automatically
	int x = gl_InstanceID % (2 * tileCountInSight);
	int y = gl_InstanceID / (2 * tileCountInSight);	
	return viewPos + ivec2(x, y) - ivec2(tileCountInSight, tileCountInSight);	
}

void main()
{
	ivec2 tileCoord = GetTileCoord();
	fragTexCoord = (vec2(tileCoord - mapOrigin) + vertPosition / float(tileSize)) / vec2(mapSize);
	fragPosition = vec4(vertPosition + tileCoord * tileSize, texture(textures.heightMap, fragTexCoord).x * 10 - 5, 1);
    gl_Position = camera.viewProjectTransform * fragPosition;
	
	//fragNormal = transform.normalTransform * vec4(vertNormal, 0);
}