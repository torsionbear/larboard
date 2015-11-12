#version 430 core

struct Textures {
	sampler2D diffuseMap;
	sampler2D heightMap;
}; 

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
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

uniform Textures textures;

in vec4 fragPosition;
//in vec4 fragNormal;
in vec2 fragTexCoord;
layout (location = 0)  out vec4 fragColor;

void main()
{
	//vec4 viewDirection = normalize(camera.viewPosition - fragPosition);
	fragColor = texture(textures.diffuseMap, fragTexCoord);
}