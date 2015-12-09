#version 430 core

uniform vec4 color;

layout (location = 0)  out vec4 fColor;

void main()
{
	fColor = color;
}