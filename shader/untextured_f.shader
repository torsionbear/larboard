#version 430 core

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout (std140,  binding = 2) uniform Material {
    vec4 diffuseEmissive;
    vec4 specularShininess;
} material;

in vec4 vPosition;
in vec4 vNormal;
in vec2 vTexCoord;
layout (location = 0)  out vec4 fColor;
layout (location = 1)  out vec4 fNormal;

vec4 viewDirection = normalize(camera.viewPosition - vPosition);

void main()
{
	fNormal = vNormal;
	fColor = vec4(material.diffuseEmissive.rgb, 1);
}