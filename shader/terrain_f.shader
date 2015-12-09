#version 430 core

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
};

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

//layout (std140,  binding = 2) uniform Material {
//    vec4 diffuse;
//    vec4 specular;
//	vec4 emissive;
//	float ambientIntensity;
//    float shininess;
//	float transparency;
//} material;
struct Material {
    vec4 diffuseEmissive;
    vec4 specularShininess;
};
Material material = Material(vec4(0.8, 0.8, 0.8, 0.0), vec4(0.2, 0.2, 0.2, 0.01));

uniform Textures textures;

in vec4 tePosition;
in vec3 teNormal;
in vec3 teDiffuseMapTexCoord;
layout (location = 0) out vec4 fDiffuseEmissive;
layout (location = 1) out vec4 fSpecularShininess;
layout (location = 2) out vec4 fNormal;

vec4 viewDirection = normalize(camera.viewPosition - tePosition);

void main()
{
	fNormal = vec4(teNormal, 0);
	vec3 fragColor0 = texture(textures.diffuseTextureArray, teDiffuseMapTexCoord - vec3(0, 0, 0.5)).rgb;
	vec3 fragColor1 = texture(textures.diffuseTextureArray, teDiffuseMapTexCoord + vec3(0, 0, 0.5)).rgb;
	float interp = fract(teDiffuseMapTexCoord.z);
	fDiffuseEmissive = vec4(fragColor0 * (1 - interp) + fragColor1 * interp, material.diffuseEmissive.a);
	fSpecularShininess = material.specularShininess;
}