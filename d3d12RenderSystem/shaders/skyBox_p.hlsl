
struct PSInput {
    float4 position : SV_POSITION;
    float3 texCoord : PS_TEXCOORD;
};

TextureCube diffuseMap : register(t0);
SamplerState staticSampler : register(s0);


float4 main(PSInput input) : SV_TARGET
{
    return diffuseMap.Sample(staticSampler, input.texCoord);
}