
struct PsInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float3 normal : PS_NORMAL;
    float3 diffuseMapTexCoord : PS_TEXCOORD;
};

struct PsOutput {
    float4 diffuse : SV_TARGET0; //SV_TARGET0;
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

Texture2DArray diffuseMap : register(t0);
Texture2D heightMap : register(t4);

SamplerState staticSampler : register(s0);

struct Material {
    float4 diffuseEmissive;
    float4 specularShininess;
};


PsOutput main(PsInput input)
{
    Material material;
    material.diffuseEmissive = float4(0.8, 0.8, 0.8, 0.0);
    material.specularShininess = float4(0.1, 0.1, 0.1, 0.01);

    PsOutput output;
    output.normal = float4(input.normal, 0);
    float3 fragColor0 = diffuseMap.Sample(staticSampler, input.diffuseMapTexCoord - float3(0, 0, 0.5)).rgb;
    float3 fragColor1 = diffuseMap.Sample(staticSampler, input.diffuseMapTexCoord + float3(0, 0, 0.5)).rgb;
    float interp = frac(input.diffuseMapTexCoord.z);
    output.diffuse = float4(lerp(fragColor0, fragColor1, interp), material.diffuseEmissive.a);
    output.specular = material.specularShininess;

    return output;
}