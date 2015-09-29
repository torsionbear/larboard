#version 430 core

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

out vec4 fragPosition;
out vec4 fragNormal;
out vec2 fragTexCoord;

uniform mat4 worldTransform;
uniform mat4 viewTransform;
uniform mat4 normalTransform;

void main()
{
	fragPosition = worldTransform * vec4(vertPosition, 1);
    gl_Position = viewTransform * fragPosition;
	fragNormal = normalTransform * vec4(vertNormal, 0);
	fragTexCoord = vertTexCoord;
}