
struct PSInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float4 normal : PS_NORMAL;
    float4 texCoord : PS_TEXCOORD;
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

PSInput main(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEXCOORD) {
    PSInput result;

    float4 newPosition = position;
    newPosition = mul(worldTransform, newPosition);
    result.worldPosition = newPosition;
    newPosition = mul(viewTransform, newPosition);
    newPosition = mul(projectTransform, newPosition);
    result.position = newPosition;
    result.worldPosition = position;
    result.normal = normal;
    result.texCoord = texCoord;
    return result;
}