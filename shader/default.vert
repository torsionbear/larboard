#version 430 core

layout (std140, row_major, binding = 0) uniform Viewpoint {
	mat4 viewTransform;
	vec4 viewPosition;
} viewpoint;

layout (std140, binding = 1) uniform Transform {
    layout (row_major) mat4 worldTransform;
    layout (row_major) mat4 normalTransform;
} transform;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

out vec4 fragPosition;
out vec4 fragNormal;
out vec2 fragTexCoord;

void main()
{
	fragPosition = transform.worldTransform * vec4(vertPosition, 1);
    gl_Position = viewpoint.viewTransform * fragPosition;
	fragNormal = transform.normalTransform * vec4(vertNormal, 0);
	fragTexCoord = vertTexCoord;
}