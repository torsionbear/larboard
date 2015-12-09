#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = camera.projectTransform * camera.viewTransform * vec4(position, 1);
}