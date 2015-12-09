#version 430 core

struct Textures {
	sampler2D diffuseMap;
};

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

layout (std140,  binding = 2) uniform Material {
    vec4 diffuseEmissive;
    vec4 specularShininess;
} material;

uniform Textures textures;

in vec4 vPosition;
in vec4 vNormal;
in vec2 vTexCoord;
layout (location = 0) out vec4 fDiffuseEmissive;
layout (location = 1) out vec4 fSpecularShininess;
layout (location = 2) out vec4 fNormal;

void main()
{
	fNormal = vNormal;
	fDiffuseEmissive = vec4(texture(textures.diffuseMap, vTexCoord).rgb, material.diffuseEmissive.a);
	fSpecularShininess = material.specularShininess;
}