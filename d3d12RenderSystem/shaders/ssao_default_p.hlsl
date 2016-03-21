
struct PsInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float4 normal : PS_NORMAL;
    float2 texCoord : PS_TEXCOORD;
};

struct PsOutput {
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 specular : SV_TARGET2;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

cbuffer Material : register(b3) {
    float3 diffuse;
    bool hasDiffuseMap;
    float3 emissive;
    bool hasEmissiveMap;
    float3 specular;
    bool hasSpecularMap;
    float shininess;
    bool hasNormalMap;
    float transparency;
    float _pad1;
    float4x4 _pad2[3];
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

float3x3 CalculateTangentSpaceTbn(float3 n, float3 viewDirection, float2 texCoord) {
    float3 dp1 = ddx(viewDirection);
    float3 dp2 = ddy(viewDirection);
    float2 duv1 = ddx(texCoord);
    float2 duv2 = -ddy(texCoord);
    float3 dp2perp = cross(dp2, n);
    float3 dp1perp = cross(n, dp1);
    float3 t = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 b = dp2perp * duv1.y + dp1perp * duv2.y;
    float invmax = rsqrt(max(dot(t, t), dot(b, b)));
    return transpose(float3x3(t * invmax, b * invmax, n));
}

float3 CalculateNormal(float3 baseNormal, float3 viewDirection, float2 texCoord) {
    float3 tangentSpaceNormal = textures[normalMapIndex].Sample(staticSampler, texCoord).rgb;
    tangentSpaceNormal = tangentSpaceNormal * 255 / 127 - 128 / 127;
    float3x3 tbn = CalculateTangentSpaceTbn(baseNormal, -viewDirection, texCoord);
    return normalize(mul(tbn, tangentSpaceNormal));
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

float Attenuation(float4 attenuation, float distance) {
    float ret = 0;
    if (distance <= attenuation.a) {
        ret = 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * (distance * distance));
    }
    return ret;
}

PsOutput main(PsInput input)
{
    //return input.texCoord;
    //return diffuse.Sample(staticSampler, input.texCoord);
        
    float3 diffuseColor = hasDiffuseMap ? textures[diffuseMapIndex].Sample(staticSampler, input.texCoord).rgb : diffuse;
    float3 specularColor = hasSpecularMap ? textures[specularMapIndex].Sample(staticSampler, input.texCoord).rgb : specular;
    float3 emissiveColor = hasEmissiveMap ? textures[emissiveMapIndex].Sample(staticSampler, input.texCoord).rgb : emissive;

    float occlusion = 0;
    float3 worldPosition = input.worldPosition.xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition);
    float3 normal = hasNormalMap ? CalculateNormal(input.normal.xyz, viewDirection, input.texCoord) : input.normal.xyz;

    PsOutput ret;
    ret.diffuse = float4(diffuseColor, emissive.r);
    ret.normal = float4(normal, 0);
    ret.specular = float4(specularColor, shininess);
    return ret;
    
    
}