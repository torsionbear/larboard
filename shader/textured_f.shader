#version 430 core

struct Textures {
	sampler2D diffuseMap;
};

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

uniform Textures textures;

in vec4 fragPosition;
in vec4 fragNormal;
in vec2 fragTexCoord;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fNormal;

void main()
{
	fNormal = fragNormal;
	fragColor = vec4(texture(textures.diffuseMap, fragTexCoord));
}