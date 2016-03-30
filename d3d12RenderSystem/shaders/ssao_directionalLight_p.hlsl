
struct PsInput {
    float4 position : SV_POSITION;
    noperspective float2 texCoord : PS_TEXCOORD;
    noperspective float3 viewDirection : PS_VIEW_DIRECTION;
    float4 directionalLightDirection : PS_DIRECTIONALLIGHT_DIRECTION;
};

struct PsOutput {
    float4 color : SV_TARGET;
    //float depth : SV_DEPTH;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer DirectionalLight : register(b2) {
    float4 directionalLightColor;
};

cbuffer ShadowCastingLight : register(b5) {
    float4x4 lightViewTransform;
    float4x4 lightProjectTransform;
    float4x4 lightViewTransformInverse;
    float4 lightViewPosition;
    float3x4 _padShadowCastingLight;
};

cbuffer TextureIndex : register(b6) {
    int diffuseMapIndex;
    int normalMapIndex;
    int specularMapIndex;
    int emissiveMapIndex;
    int shadowMapIndex;
    int occlusionIndex;
    int DepthIndex;
};

Texture2D textures[10] : register(t1);
SamplerState staticSampler : register(s0);
SamplerState shadowMapSampler : register(s1);

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
    float ndcDepth = screenSpaceDepth;// *2 - 1;
    return -projectTransform[2][3] / (projectTransform[2][2] + ndcDepth);
}

float4 CalculateViewSpacePosition(float depth, float3 viewDirection) {
    float viewCoordZ = screenSpaceDepthToViewSpaceDepth(depth);
    return float4(viewDirection * -viewCoordZ, 1);
}

float DiffuseCoefficient(float3 normal, float3 lightDirection) {
    return max(dot(normal, -lightDirection), 0.0);
}

float SpecularCoefficient(float3 normal, float3 lightDirection, float3 viewDirection, float shininess) {
    float cosine = max(dot(viewDirection, reflect(lightDirection, normal)), 0);
    // multiple 128 to x3d shininess. see http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingmodel
    shininess *= 128;
    return shininess > 0 ? pow(cosine, shininess) : 1; // avoid pow(0, 0)
}

PsOutput main(PsInput input) {
    float depth = textures[DepthIndex].Sample(staticSampler, input.texCoord).r;
    float3 worldPosition = mul(viewTransformInverse, CalculateViewSpacePosition(depth, input.viewDirection)).xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition.xyz);

    float4 diffuseEmissive = textures[diffuseMapIndex].Sample(staticSampler, input.texCoord);
    float4 specularShininess = textures[specularMapIndex].Sample(staticSampler, input.texCoord);
    float3 normal = textures[normalMapIndex].Sample(staticSampler, input.texCoord).rgb;

    float3 diffuse = directionalLightColor.rgb * diffuseEmissive.rgb * DiffuseCoefficient(normal, input.directionalLightDirection.xyz);
    float3 specular = directionalLightColor.rgb * specularShininess.rgb * SpecularCoefficient(normal, input.directionalLightDirection.xyz, viewDirection, specularShininess.a);

    PsOutput ret;
    ret.color = float4(diffuse + specular, 1);

    return ret;
}