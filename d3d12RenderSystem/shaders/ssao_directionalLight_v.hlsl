
struct PsInput {
    float4 position : SV_POSITION;
    noperspective float2 texCoord : PS_TEXCOORD;
    noperspective float3 viewDirection : PS_VIEW_DIRECTION;
    float4 directionalLightDirection : PS_DIRECTIONALLIGHT_DIRECTION;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer Transform : register(b1) {
    float4x4 worldTransform;
    float4x4 normalTransform;
    float4x4 _pad1;
    float4x4 _pad2;
};

PsInput main(float2 position : POSITION) {
    PsInput result;
    result.position = float4(position, 0, 1);
    result.texCoord = (position + 1) * 0.5;
    result.texCoord.y = 1.0 - result.texCoord.y;
    result.viewDirection = float3(position / float2(projectTransform[0][0], projectTransform[1][1]), -1);
    result.directionalLightDirection = mul(worldTransform, float4(0, 0, -1, 0));
    return result;
}