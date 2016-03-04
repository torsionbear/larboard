
struct PsInput {
    float4 position : SV_POSITION;
    float2 texCoord : PS_TEXCOORD;
    float3 viewDirection : PS_VIEW_DIRECTION;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

PsInput main(float2 position : POSITION) {
    PsInput result;
    result.position = float4(position, 0, 1);
    result.texCoord = (position + 1) * 0.5;
    result.texCoord.y = 1.0 - result.texCoord.y;
    result.viewDirection = float3(position / float2(projectTransform[0][0], projectTransform[1][1]), -1);
    return result;
}