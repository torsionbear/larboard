#version 430 core

struct Textures {
	samplerCube cubeMap;
};

uniform Textures textures;

in vec3 fragTexCoord;
layout (location = 0)  out vec4 fragColor;

void main()
{
	fragColor = texture(textures.cubeMap, fragTexCoord);
}