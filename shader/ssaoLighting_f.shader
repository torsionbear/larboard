#version 430 core

struct GBuffer {
	sampler2D color;
	sampler2D normal;
	sampler2D depth;
	sampler2D occlusion;
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
uniform int randomVectorTexSize;

in vec2 vTexCoord;
in vec3 viewDirection;

layout (location = 0)  out vec4 fColor;

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
	float ndcDepth = screenSpaceDepth * 2 - 1;
	return camera.projectTransform[3][2] / (camera.projectTransform[2][3] * ndcDepth - camera.projectTransform[2][2]);
}

vec4 CalculateViewSpacePosition(float depth) {
	float viewCoordZ =  screenSpaceDepthToViewSpaceDepth(depth);
	return vec4(viewDirection * -viewCoordZ, 1);
}

void main()
{
	vec2 texelSize = 1.0 / textureSize(gBuffer.occlusion, 0);
	float occlusion = 0;
	for(int i = 0; i < randomVectorTexSize; ++i) {
		for(int j = 0; j < randomVectorTexSize; ++j) {
			vec2 offset = (vec2(float(i), float(j)) - randomVectorTexSize * 0.5) * texelSize;
			occlusion += texture(gBuffer.occlusion, vTexCoord + offset).r;
		}
	}
	occlusion = occlusion / (randomVectorTexSize * randomVectorTexSize);
	
	fColor = texture(gBuffer.color, vTexCoord) * (1 - occlusion);
	//fColor = vec4(vec3(1.0 - occlusion), 1.0);	

	float screenSpaceDepth = texture(gBuffer.depth, vTexCoord).r;
}