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
uniform vec4 viewport;

//in vec2 vPosition;
in vec3 viewDirection;
layout (location = 0)  out vec4 fColor;

vec4 CalculateViewSpacePosition(float depth) {
	float ndcDepth = depth * 2 - 1;
	float viewCoordZ =  camera.projectTransform[3][2] / (camera.projectTransform[2][3] * ndcDepth - camera.projectTransform[2][2]);
	return vec4(viewDirection * -viewCoordZ, 1);
}

void main()
{
	vec2 texCoord = (gl_FragCoord.xy - viewport.xy) / viewport.zw;
	fColor = texture(gBuffer.color, texCoord);

	vec4 viewSpcePosition = CalculateViewSpacePosition(texture(gBuffer.depth, texCoord).r);
	fColor = vec4(vec3(-viewSpcePosition.z / 40), 1);
	
	vec4 viewSpaceNormal = camera.viewTransform * texture(gBuffer.normal, texCoord);
	fColor = abs(viewSpaceNormal);
}