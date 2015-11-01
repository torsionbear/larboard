#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	vec4 viewPosition;
} camera;

layout(location = 0) in vec3 vertPosition;

void main()
{
    gl_Position = camera.viewTransform * vec4(vertPosition, 1);
}