

struct DsInput {
    float4 position : SV_POSITION;
    float2 diffuseMapTexCoord : HS_TEXCOORD;
};

struct TessLevel {
    float outer[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
};

struct PsInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float3 normal : PS_NORMAL;
    float3 diffuseMapTexCoord : PS_TEXCOORD;
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

Texture2D heightMap : register(t0);

SamplerState staticSampler : register(s0);

float Height(float2 position) {
    float2 heightMapTexCoord = (position / tileSize - heightMapOrigin) / heightMapSize;
    heightMapTexCoord.y = 1 - heightMapTexCoord.y;
    float height = heightMap.SampleLevel(staticSampler, heightMapTexCoord, 0).r;
    return height * 30.0 - 0.6;
}

[domain("tri")]
PsInput main(TessLevel tessLevel, float3 tessCoord : SV_DomainLocation, const OutputPatch<DsInput, 3> patch) {
    PsInput psInput;
    psInput.worldPosition = patch[0].position * tessCoord.x + patch[1].position * tessCoord.y + patch[2].position * tessCoord.z;

    float height = Height(psInput.worldPosition.xy);

    float2 diffuseMapTexCoord = patch[0].diffuseMapTexCoord * tessCoord.x + patch[1].diffuseMapTexCoord * tessCoord.y + patch[2].diffuseMapTexCoord * tessCoord.z;
    psInput.diffuseMapTexCoord = float3(diffuseMapTexCoord, (height + 10) / 15);

    psInput.worldPosition.z += height;
    psInput.position = mul(projectTransform, mul(viewTransform, psInput.worldPosition));

    // normal
    float step = tileSize / tessLevel.inner;
    float3 position_dx = psInput.worldPosition.xyz + float3(step, 0, 0);
    float3 position_dy = psInput.worldPosition.xyz + float3(0, step, 0);
    position_dx.z = Height(position_dx.xy);
    position_dy.z = Height(position_dy.xy);
    psInput.normal = normalize(cross(position_dx - psInput.worldPosition.xyz, position_dy - psInput.worldPosition.xyz));
    return psInput;
}