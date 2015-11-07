#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

uniform vec4 color;

layout (location = 0)  out vec4 fragColor;

void main()
{
	fragColor = color;
}