
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
    float3 normal = input.normal.xyz;
    float3 worldPosition = input.worldPosition.xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition);

    PsOutput ret;
    ret.diffuse = float4(diffuseColor, emissive.r);
    ret.normal = input.normal;
    ret.specular = float4(specularColor, shininess);
    return ret;
    
    
}