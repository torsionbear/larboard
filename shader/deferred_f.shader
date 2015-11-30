#version 430 core

struct GBuffer {
	sampler2D color;
	sampler2D normal;
	sampler2D depth;
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

uniform GBuffer gBuffer;

in vec2 vTexCoord;
in vec3 viewDirection;
layout (location = 0)  out vec4 fColor;

vec4 CalculateViewSpacePosition(float depth) {
	float ndcDepth = depth * 2 - 1;
	float viewCoordZ =  camera.projectTransform[3][2] / (camera.projectTransform[2][3] * ndcDepth - camera.projectTransform[2][2]);
	return vec4(viewDirection * -viewCoordZ, 1);
}

void main()
{
	fColor = texture(gBuffer.color, vTexCoord);

	vec4 viewSpacePosition = CalculateViewSpacePosition(texture(gBuffer.depth, vTexCoord).r);
	if(-0.1 - viewSpacePosition.z < 0.001) {
		fColor = vec4(1, 0, 0, 1);
	} else {
		fColor = vec4(vec3(-viewSpacePosition.z / 40), 1);
	}
	//fColor = vec4(vec3(-viewSpacePosition.z / 40), 1);
	
	vec4 viewSpaceNormal = camera.viewTransform * texture(gBuffer.normal, vTexCoord);
	//fColor = abs(viewSpaceNormal);
}