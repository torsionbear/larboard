
struct PsInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float4 normal : PS_NORMAL;
    float2 texCoord : PS_TEXCOORD;
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

PsInput main(float3 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD) {
    PsInput result;

    float4 newPosition = float4(position, 1);
    newPosition = mul(worldTransform, newPosition);
    result.worldPosition = newPosition;
    newPosition = mul(viewTransform, newPosition);
    newPosition = mul(projectTransform, newPosition);
    result.position = newPosition;
    result.normal = mul(normalTransform, float4(normal, 0));
    result.texCoord = float2(texCoord.x, 1.0 - texCoord.y); // reverse y coordinate because dx's texture coordinate origin is at top left
    return result;
}