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
uniform ivec2 mapOrigin;
uniform ivec2 mapSize; 

uniform Textures textures;

layout(location = 0) in vec2 vertPosition;
//layout(location = 1) in vec3 vertNormal;
//layout(location = 2) in vec2 vertTexCoord;

out vec4 fragPosition;
//out vec4 fragNormal;
out vec3 fragTexCoord;

ivec2 GetTileCoord() {
	ivec2 viewPos = ivec2(camera.viewPosition.xy / tileSize); // converting from float to int drops fractional part automatically
	int x = gl_InstanceID % (2 * tileCountInSight);
	int y = gl_InstanceID / (2 * tileCountInSight);	
	return viewPos + ivec2(x, y) - ivec2(tileCountInSight, tileCountInSight);	
}

void main()
{
	ivec2 tileCoord = GetTileCoord();
	fragTexCoord = vec3((vec2(tileCoord - mapOrigin) + vertPosition / float(tileSize)) / vec2(mapSize), 0);
	float height = texture(textures.heightMap, fragTexCoord.xy).x * 30 - 10;
	fragTexCoord.z = (height + 10) / 15;
	fragPosition = vec4(vertPosition + tileCoord * tileSize, height, 1);
    gl_Position = camera.viewProjectTransform * fragPosition;
	
	//fragNormal = transform.normalTransform * vec4(vertNormal, 0);
}