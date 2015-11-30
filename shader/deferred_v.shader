#version 430 core

layout(location = 0) in vec2 position;

out vec3 viewDirection; // vec3 whose z component is always -1
uniform vec3 nearPlane; // vec3(halfWidth, halfHeight, distance)

void main()
{
	gl_Position = vec4(position, 0, 1);
	viewDirection = vec3(position * nearPlane.xy / nearPlane.z, -1);
}