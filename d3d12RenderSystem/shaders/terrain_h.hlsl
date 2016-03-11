
struct HsInput {
    float4 position : SV_POSITION;
    float2 diffuseMapTexCoord : HS_TEXCOORD;
};

struct DsInput {
    float4 position : SV_POSITION;
    float2 diffuseMapTexCoord : HS_TEXCOORD;
};

struct TessLevel {
    float outer[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
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

float CalculateOuterTessLevel(float4 vertex0, float4 vertex1) {
    float edgeLength = length(vertex0 - vertex1);
    float distanceFactor = sightDistance / 60;
    float distance0 = length(2 * viewPosition - vertex0 - vertex1) / 2;
    return max(edgeLength * distanceFactor / distance0, 1.0);
}

TessLevel TessLevelCalculator(InputPatch<HsInput, 3> patch, uint patchId: SV_PrimitiveID) {
    TessLevel tessLevel;
    tessLevel.outer[0] = CalculateOuterTessLevel(patch[1].position, patch[2].position);
    tessLevel.outer[1] = CalculateOuterTessLevel(patch[2].position, patch[0].position);
    tessLevel.outer[2] = CalculateOuterTessLevel(patch[0].position, patch[1].position);
    tessLevel.inner = (tessLevel.outer[0] + tessLevel.outer[1] + tessLevel.outer[2]) / 3;
    return tessLevel;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TessLevelCalculator")]
DsInput main(InputPatch<HsInput, 3> patch, uint patchId : SV_PrimitiveID, uint pointId : SV_OutputControlPointID) {
    DsInput output;
    output.position = patch[pointId].position;
    output.diffuseMapTexCoord = patch[pointId].diffuseMapTexCoord;
    return output;
}