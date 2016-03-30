
struct PsInput {
    float4 position : SV_POSITION;
    noperspective float2 texCoord : PS_TEXCOORD;
    noperspective float3 viewDirection : PS_VIEW_DIRECTION;
};

struct PsOutput {
    float4 color : SV_TARGET;
    //float depth : SV_DEPTH;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer AmbientLight : register(b2) {
    float4 ambientLightColor;
};

cbuffer SsaoData : register(b3) {
    //int2 randomVectorTextureSize;
    int2 occlusionTextureSize;
    int sampleCount;
    float3 samples[95];
};
static int2 randomVectorTextureSize = int2(4, 4);

cbuffer ShadowCastingLight : register(b5) {
    float4x4 lightViewTransform;
    float4x4 lightProjectTransform;
    float4x4 lightViewTransformInverse;
    float4 lightViewPosition;
    float3x4 _padShadowCastingLight;
};

cbuffer TextureIndex : register(b6) {
    int diffuseMapIndex;
    int normalMapIndex;
    int specularMapIndex;
    int emissiveMapIndex;
    int shadowMapIndex;
    int occlusionIndex;
    int DepthIndex;
};

Texture2D textures[10] : register(t1);
SamplerState staticSampler : register(s0);
SamplerState shadowMapSampler : register(s1);

float GetOcclusion(float2 texCoord) {
    // blur gBuffer.occlusion by randomVectorTextureSize
    float2 texelSize = 1.0 / occlusionTextureSize;
    float occlusion = 0;
    for (int i = 0; i < randomVectorTextureSize.x; ++i) {
        for (int j = 0; j < randomVectorTextureSize.y; ++j) {
            float2 offset = (float2(float(i), float(j)) - randomVectorTextureSize * 0.5) * texelSize;
            occlusion += textures[occlusionIndex].Sample(staticSampler, texCoord + offset).r;
        }
    }
    return occlusion / (randomVectorTextureSize.x * randomVectorTextureSize.y);
}

PsOutput main(PsInput input) {
    float occlusion = GetOcclusion(input.texCoord);
    float4 diffuseEmissive = textures[diffuseMapIndex].Sample(staticSampler, input.texCoord);

    float3 ambient = ambientLightColor.rgb * diffuseEmissive.rgb *(1 - occlusion) + diffuseEmissive.rgb * diffuseEmissive.a;

    PsOutput ret;
    ret.color = float4(ambient, 1);
    //ret.color = float4(occlusion, occlusion, occlusion, 1);
    return ret;
}