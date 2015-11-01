#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	vec4 viewPosition;
} camera;

uniform vec4 color;

layout (location = 0)  out vec4 fragColor;

void main()
{
	fragColor = color;
}