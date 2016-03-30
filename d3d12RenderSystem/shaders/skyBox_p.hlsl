
struct PsInput {
    float4 position : SV_POSITION;
    float3 texCoord : PS_TEXCOORD;
};

struct PsOutput {
    float4 color : SV_TARGET;
    float depth : SV_DEPTH;
};

cbuffer TextureIndex : register(b6) {
    int diffuseMapIndex;
    int normalMapIndex;
    int specularMapIndex;
    int emissiveMapIndex;
    int textureIndex4;
    int textureIndex5;
    int textureIndex6;
};

TextureCube textures[10] : register(t1);
SamplerState staticSampler : register(s0);


PsOutput main(PsInput input)
{
    PsOutput result;
    result.color = textures[diffuseMapIndex].Sample(staticSampler, input.texCoord);
    result.depth = 1.0;
    return result;
}