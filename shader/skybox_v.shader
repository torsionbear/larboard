#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout(location = 0) in vec3 position;

out vec3 vTexCoord;

void main()
{
    vTexCoord = position;
    gl_Position = camera.projectTransform * camera.rotationInverse * vec4(position, 1);
}