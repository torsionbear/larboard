
struct PsInput {
    float4 position : SV_POSITION;
    float2 texCoord : PS_TEXCOORD;
    float3 viewDirection : PS_VIEW_DIRECTION;
};

struct PsOutput {
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 specular : SV_TARGET2;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer SsaoData : register(b3) {
    //int2 randomVectorTextureSize;
    int2 occlusionTextureSize;
    int sampleCount;
    float3 samples[95];
};
static int2 randomVectorTextureSize = int2(4, 4);

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D depthMap : register(t3);
Texture2D randomVectorMap : register(t4);
SamplerState staticSampler : register(s0);

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
    float ndcDepth = screenSpaceDepth;// *2 - 1;
    return -projectTransform[2][3] / (projectTransform[2][2] + ndcDepth);
}

float4 CalculateViewSpacePosition(float depth, float3 viewDirection) {
    float viewCoordZ = screenSpaceDepthToViewSpaceDepth(depth);
    return float4(viewDirection * -viewCoordZ, 1);
}

float main(PsInput input) : SV_TARGET
{
    float occlusionRange = 2.0;
    float screenSpaceDepth = depthMap.Sample(staticSampler, input.texCoord);

    float occlusion = 0;

    float4 viewSpacePosition = CalculateViewSpacePosition(screenSpaceDepth, input.viewDirection);

    float3 normal = normalMap.Sample(staticSampler, input.texCoord);
    float3 viewSpaceNormal = mul(viewTransform, normal).xyz;
    
    float4 randomVector = randomVectorMap.Sample(staticSampler, input.texCoord * float2(occlusionTextureSize) / float2(randomVectorTextureSize));
    float3 tangent = normalize(randomVector.xyz - viewSpaceNormal * dot(randomVector.xyz, viewSpaceNormal));
    float3 bitangent = cross(viewSpaceNormal, tangent);
    float3x3 tbn = transpose(float3x3(tangent, bitangent, viewSpaceNormal));
    
    for (int i = 0; i < sampleCount; ++i) {
        float3 viewSpaceSample = mul(tbn, samples[i]) * occlusionRange;
        viewSpaceSample = viewSpacePosition.xyz + viewSpaceSample;
        float4 ndcSample = mul(projectTransform, float4(viewSpaceSample, 1));
        ndcSample.xyz /= ndcSample.w;
        float2 sampleTextureCoord = ndcSample.xy * 0.5 + 0.5; // from [-1, 1] to [0, 1]
		sampleTextureCoord.y = 1.0 - sampleTextureCoord.y;
        float screenSpaceDepthAtSample = depthMap.Sample(staticSampler, sampleTextureCoord).r;
        float viewSpaceDepthAtSample = screenSpaceDepthToViewSpaceDepth(screenSpaceDepthAtSample);
        float depthDiff = viewSpaceDepthAtSample - viewSpaceSample.z;
        bool occluded = depthDiff > 0 && depthDiff < occlusionRange;
        occlusion += occluded ? 1 : 0;
    }
    occlusion = occlusion / sampleCount;
    return occlusion;
    
}