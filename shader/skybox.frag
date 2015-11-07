#version 430 core

uniform samplerCube cubeMap;

in vec3 fragTexCoord;
layout (location = 0)  out vec4 fragColor;

void main()
{
	fragColor = texture(cubeMap, fragTexCoord);
}