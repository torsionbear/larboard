#version 430 core

struct Textures {
	samplerCube cubeMap;
};

uniform Textures textures;

in vec3 vTexCoord;

layout (location = 0)  out vec4 fColor;

void main()
{
	fColor = texture(textures.cubeMap, vTexCoord);
}