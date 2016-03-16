
struct PSInput {
    float4 position : SV_POSITION;
    float3 texCoord : PS_TEXCOORD;
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


float4 main(PSInput input) : SV_TARGET
{
    return textures[diffuseMapIndex].Sample(staticSampler, input.texCoord);
}