#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout (std140, binding = 1) uniform Transform {
    layout (row_major) mat4 worldTransform;
    layout (row_major) mat4 normalTransform;
} transform;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

out vec4 vPosition;
out vec4 vNormal;
out vec2 vTexCoord;

void main()
{
	vPosition = transform.worldTransform * vec4(vertPosition, 1);
    gl_Position = camera.projectTransform * camera.viewTransform * vPosition;
	vNormal = transform.normalTransform * vec4(vertNormal, 0);
	vTexCoord = vertTexCoord;
}