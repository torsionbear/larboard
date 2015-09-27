#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec2 fTexCoord;

uniform mat4 worldTransform;
uniform mat4 viewTransform;

void main()
{
    gl_Position = viewTransform * worldTransform * vPosition;
	fTexCoord = vTexCoord;
}