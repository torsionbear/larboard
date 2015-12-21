#version 430 core

struct GBuffer {
	sampler2D diffuseEmissive;
	sampler2D specularShininess;
	sampler2D normal;
	sampler2D occlusion;
	sampler2D depth;
};

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

uniform GBuffer gBuffer;
uniform vec3 samples[64];
uniform sampler2D randomVectorTex;
uniform int randomVectorTexSize;
uniform ivec2 occlusionTextureSize;

in vec2 vTexCoord;
in vec3 viewDirection;

layout (location = 3)  out float occlusion;

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
	float ndcDepth = screenSpaceDepth * 2 - 1;
	return camera.projectTransform[3][2] / (camera.projectTransform[2][3] * ndcDepth - camera.projectTransform[2][2]);
}

vec4 CalculateViewSpacePosition(float depth) {
	float viewCoordZ =  screenSpaceDepthToViewSpaceDepth(depth);
	return vec4(viewDirection * -viewCoordZ, 1);
}

const float occlusionRange = 2.0;

void main()
{
	float screenSpaceDepth = texture(gBuffer.depth, vTexCoord).r;
	
	occlusion = 0;
	// do not calculate occlusion for pixels at far clip plane, e.g. skybox
	if(screenSpaceDepth < 1) {
		vec4 viewSpacePosition = CalculateViewSpacePosition(screenSpaceDepth);
		//if(-0.1 - viewSpacePosition.z < 0.001) {
		//	fColor = vec4(1, 0, 0, 1);
		//} else {
		//	fColor = vec4(vec3(-viewSpacePosition.z / 40), 1);
		//}
		//fColor = vec4(vec3(-viewSpacePosition.z / 1000), 1);	
		vec4 viewSpaceNormal = camera.viewTransform * texture(gBuffer.normal, vTexCoord);
		//fColor = viewSpaceNormal;
		vec3 normal = viewSpaceNormal.xyz;
		vec4 randomVector = texture(randomVectorTex, vTexCoord * float(occlusionTextureSize) / float(randomVectorTexSize));
		
		vec3 tangent = normalize(randomVector.xyz - normal * dot(randomVector.xyz, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 TBN = mat3(tangent, bitangent, normal);
		for(int i = 0; i < 64; ++i) {
			vec3 viewSpaceSample = TBN * samples[i] * occlusionRange;
			viewSpaceSample = viewSpacePosition.xyz + viewSpaceSample;
			vec4 ndcSample = camera.projectTransform * vec4(viewSpaceSample, 1);
			ndcSample.xyz /= ndcSample.w;
			vec3 screenSpaceSample = ndcSample.xyz * 0.5 + 0.5; // from [-1, 1] to [0, 1]
			float depthDiff = screenSpaceDepthToViewSpaceDepth(texture(gBuffer.depth, screenSpaceSample.xy).r) - viewSpaceSample.z;
			bool occluded = depthDiff > 0 && depthDiff < occlusionRange;
			occlusion += occluded ? 1 : 0;
		}
		occlusion = occlusion / 64;
	}
	//fColor = vec4(vec3(1 - occlusion), 1);
}