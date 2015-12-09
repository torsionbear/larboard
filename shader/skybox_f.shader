#version 430 core

struct Textures {
	samplerCube cubeMap;
};

uniform Textures textures;

in vec3 vPosition;

layout (location = 0) out vec4 fDiffuseEmissive;
layout (location = 1) out vec4 fSpecularShininess;
layout (location = 2) out vec4 fNormal;

void main()
{
	// set emissive to compensate ambient value so (emissive + ambient) = 1.0
	fDiffuseEmissive = vec4(texture(textures.cubeMap, vPosition).rgb, 0.5);
	fSpecularShininess = vec4(0);
	fNormal = vec4(normalize(-vPosition), 0);
}