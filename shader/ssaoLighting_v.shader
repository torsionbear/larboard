#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

layout(location = 0) in vec2 position;

out vec2 vTexCoord;
out vec3 viewDirection; // vec3 whose z component is always -1

void main()
{
	gl_Position = vec4(position, 0, 1);
	vTexCoord = (position + 1) / 2;
	viewDirection = vec3(position / vec2(camera.projectTransform[0][0], camera.projectTransform[1][1]), -1);
}