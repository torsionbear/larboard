
struct PsInput {
    float4 position : SV_POSITION;
    float3 texCoord : PS_TEXCOORD;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

PsInput main( float3 position : POSITION )
{
    PsInput result;
    result.texCoord = position.xzy; // world coordinate: y front, x left, z up; dx cubemap coordinate: z front, x left, y up
    float4 newPosition = float4(position, 0);
    newPosition = mul(viewTransform, newPosition);
    newPosition = mul(projectTransform, float4(newPosition.xyz, 1));
    result.position = newPosition;
    return result;
}