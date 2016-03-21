
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

cbuffer TextureIndex : register(b6) {
    int diffuseTextureIndex;
    int normalTextureIndex;
    int specularTextureIndex;
    int emissiveTextureIndex;
    int shadowMapIndex;
    int randomVectorIndex;
    int DepthIndex;
};

Texture2D textures[10] : register(t1);
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
    float screenSpaceDepth = textures[DepthIndex].Sample(staticSampler, input.texCoord).r;

    float occlusion = 0;

    float4 viewSpacePosition = CalculateViewSpacePosition(screenSpaceDepth, input.viewDirection);

    float3 normal = textures[normalTextureIndex].Sample(staticSampler, input.texCoord).xyz;
    float3 viewSpaceNormal = mul(viewTransform, normal).xyz;
    
    float4 randomVector = textures[randomVectorIndex].Sample(staticSampler, input.texCoord * float2(occlusionTextureSize) / float2(randomVectorTextureSize));
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
        float screenSpaceDepthAtSample = textures[DepthIndex].Sample(staticSampler, sampleTextureCoord).r;
        float viewSpaceDepthAtSample = screenSpaceDepthToViewSpaceDepth(screenSpaceDepthAtSample);
        float depthDiff = viewSpaceDepthAtSample - viewSpaceSample.z;
        bool occluded = depthDiff > 0 && depthDiff < occlusionRange;
        occlusion += occluded ? 1 : 0;
    }
    occlusion = occlusion / sampleCount;
    //occlusion = occlusion < 0.5 ? 0.0 : (occlusion - 0.5) * 2;
    return occlusion;
    
}