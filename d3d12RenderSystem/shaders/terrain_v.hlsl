
struct HsInput {
    float4 position : SV_POSITION;
    float2 diffuseMapTexCoord : HS_TEXCOORD;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer Terrain : register(b4) {
    int2 heightMapOrigin;
    int2 heightMapSize;
    int2 diffuseMapOrigin;
    int2 diffuseMapSize;
    float tileSize;
    float sightDistance;
};

HsInput main( float2 position : POSITION, float3 tileCoord : TILECOORD )
{
    HsInput output;
    output.diffuseMapTexCoord = ((tileCoord.xy + position) / tileSize - diffuseMapOrigin) / float2(diffuseMapSize);
    output.position = float4(tileCoord.xy + position, tileCoord.z, 1);

    return output;
}